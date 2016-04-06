#include "fc.h"

#include "../drivers/sensors/devices.h"
#include "../drivers/uart.h"
#include "../drivers/audio/audio.h"
#include "../drivers/audio/sequencer.h"


#include "kalman.h"
#include "vario.h"

#include "protocols/protocol.h"

#include "logger/logger.h"

#include "../gui/gui_dialog.h"

volatile flight_data_t fc;

Timer fc_meas_timer;

extern KalmanFilter * kalmanFilter;

MK_SEQ(gps_ready, ARR({750, 0, 750, 0, 750, 0}), ARR({250, 150, 250, 150, 250, 150}));

void fc_init()
{
	DEBUG(" *** Flight computer init ***\n");

	//start values
	active_page = config.gui.last_page;
	if (active_page >= config.gui.number_of_pages)
		active_page = 0;

	//reset flight status
	fc_reset();

	//temperature state machine
	fc.temp_step = 0;

	//init calculators
	vario_init();
	audio_init();
	logger_init();

	gps_init();
	if (config.connectivity.use_gps)
		gps_start();

	bt_init();
	if (config.connectivity.use_bt)
		bt_module_init();

	//VCC to baro, acc/mag gyro + i2c pull-ups
	mems_power_on();

	//init and test i2c
	//HW_REW_1504 have two mems enable pins, both have to be enabled!
	//HW_REW_1506 have standalone ldo for mems, hence only one pin is needed
	if (!mems_i2c_init())
	{
		DEBUG("ERROR I2C, Wrong board rev? (%02X)\n", hw_revision);
		led_set(0xFF, 0, 0);

		hw_revision = HW_REW_1504;
		eeprom_busy_wait();
		eeprom_update_byte(&config_ro.hw_revision, hw_revision);
		eeprom_busy_wait();

		mems_power_init();
		io_init();
		mems_power_on();
		assert(mems_i2c_init());
	}
	else
	{
		if (hw_revision == HW_REW_UNKNOWN)
		{
			hw_revision = HW_REW_1506;
			eeprom_busy_wait();
			eeprom_update_byte(&config_ro.hw_revision, hw_revision);
			eeprom_busy_wait();

			mems_power_init();
			io_init();
			mems_power_on();
			mems_i2c_init();
		}
	}


	if (!mems_i2c_init())
	{
		DEBUG("I2C error!\nUnable to init flight computer!\n");
		return;
	}

	//Barometer
	ms5611.Init(&mems_i2c, MS5611_ADDRESS_CSB_LO);

	//Magnetometer + Accelerometer
	lsm303d_settings lsm_cfg;

	lsm_cfg.enabled = true;
	lsm_cfg.accOdr = lsm_acc_1600Hz;
	lsm_cfg.accScale = lsm_acc_16g;

	lsm_cfg.magOdr = lsm_mag_100Hz;
	lsm_cfg.magScale = lsm_mag_4g;
	lsm_cfg.magHiRes = true;

	lsm_cfg.tempEnable = false;

	//Gyro
	l3gd20_settings l3g_cfg;
	l3g_cfg.enabled = true;
	l3g_cfg.bw = l3g_50Hz;
	l3g_cfg.odr = l3g_760Hz;
	l3g_cfg.scale = l3g_2000dps;

	//SHT21
	sht21_settings sht_cfg;
	sht_cfg.rh_enabled = true;
	sht_cfg.temp_enabled = true;

	//XXX: do self-test?
	lsm303d.Init(&mems_i2c, lsm_cfg);
	lsm303d.Start();

	l3gd20.Init(&mems_i2c, l3g_cfg);
	l3gd20.Start();

	sht21.Init(&mems_i2c, sht_cfg);

	//Measurement timer
	FC_MEAS_TIMER_PWR_ON;

	fc_meas_timer.Init(FC_MEAS_TIMER, timer_div1024);
	fc_meas_timer.SetInterruptPriority(MEDIUM);
	fc_meas_timer.EnableInterrupts(timer_overflow | timer_compareA | timer_compareB | timer_compareC);

	//tight timing!	      1 tick 0.032 ms
	//MS pressure conversion     9.040 ms
	//   temperature conversion  0.600 ms
	//MAG read 					 0.152 ms
	//ACC read					 1.600 ms
	//Gyro read					 1.000 ms

	fc_meas_timer.SetTop(313); // == 10ms
	fc_meas_timer.SetCompare(timer_A, 27); // == 0.78 ms
	fc_meas_timer.SetCompare(timer_B, 70); // == 2 ms
	fc_meas_timer.SetCompare(timer_C, 200); // == 6 ms

	ms5611.StartTemperature();
	lsm303d.StartReadMag(); //it takes 152us to transfer
	_delay_ms(1);

	fc_meas_timer.Start();

	DEBUG(" *** FC init done ***\n");
}

void fc_deinit()
{
	fc_meas_timer.Stop();

	if (fc.flight_state == FLIGHT_FLIGHT)
		fc_landing();

	eeprom_busy_wait();
	//store altimeter info
	eeprom_update_float(&config_ee.altitude.QNH1, config.altitude.QNH1);
	eeprom_update_float(&config_ee.altitude.QNH2, config.altitude.QNH2);

	if (config.connectivity.use_bt)
		bt_stop();

	if (config.connectivity.use_gps)
		gps_stop();

	for (uint8_t i=0; i<NUMBER_OF_ALTIMETERS; i++)
	{
		eeprom_update_word((uint16_t *)&config_ee.altitude.altimeter[i].delta, config.altitude.altimeter[i].delta);
	}

	mems_power_off();
}

void fc_pause()
{
	fc_meas_timer.Stop();
}

void fc_continue()
{
	fc_meas_timer.Start();
}

uint8_t calib_cnt;
uint16_t calib_rtc_cnt;

//First fc meas period
// * Read pressure from ms5611
// * Start temperature conversion ms5611
// * Init lsm303d i2c readout for magnetometer (block the i2c bus)
// * Compensate pressure from ms5611
ISR(FC_MEAS_TIMER_OVF)
{
	BT_SUPRESS_TX

	io_write(1, HIGH);
	ms5611.ReadPressure();
	io_write(1, LOW);

	ms5611.StartTemperature();
	lsm303d.StartReadMag(); //it takes 152us to transfer

	calib_cnt++;
	if (calib_cnt == 10)
	{
		calib_cnt = 0;

		uint16_t rtc = RtcGetValue();
		if (rtc > calib_rtc_cnt)
		{
			uint16_t delta = rtc - calib_rtc_cnt;
			uint8_t cala = DFLLRC32M.CALA & 0b01111111;

			if (delta < 3274)
			{
				if (cala > 0)
					cala--;

				DFLLRC32M.CALA = cala & 0b01111111;
			}
			else if (delta > 3281)
			{
				if (cala < 127)
					cala++;
				DFLLRC32M.CALA = cala & 0b01111111;
			}
		}
		calib_rtc_cnt = rtc;
	}


	BT_ALLOW_TX
}

//Second fc meas period
// * Load lsm303d magnetometer data from buffer (free the i2c bus)
// * Read temperature form ms5611
// * Start pressure conversion ms5611
// * Init lsm303d i2c readout for accelerometer (block the i2c bus)
// * Calculate time sensitive values
// * Setup the Buzzer
// * Compensate temperature
ISR(FC_MEAS_TIMER_CMPA)
{
	BT_SUPRESS_TX

	lsm303d.ReadMag(&fc.mag_data.x, &fc.mag_data.y, &fc.mag_data.z);
	ms5611.ReadTemperature();

	ms5611.StartPressure();
	lsm303d.StartReadAccStream(16); //it take 1600us to transfer

	ms5611.CompensateTemperature();
	ms5611.CompensatePressure();

	//vario loop
	vario_calc(ms5611.pressure);
	//audio loop
	audio_step();

	BT_ALLOW_TX
}

//Third fc meas period
// * Load lsm303d accelerometer data from buffer (free the i2c bus)
// * Init l3gd20 i2c readout for gyroscope (block the i2c bus)
ISR(FC_MEAS_TIMER_CMPB)
{
	BT_SUPRESS_TX

	lsm303d.ReadAccStreamAvg(&fc.acc_data.x, &fc.acc_data.y, &fc.acc_data.z, 16);
	l3gd20.StartReadGyroStream(7); //it take 1000us to transfer

	BT_ALLOW_TX
}

//Final fc meas period
// * Load l3gd20 gyroscope data from buffer (free the i2c bus)
// * Handle slow sht21 conversions
ISR(FC_MEAS_TIMER_CMPC)
{
	BT_SUPRESS_TX

	l3gd20.ReadGyroStreamAvg(&fc.gyro_data.x, &fc.gyro_data.y, &fc.gyro_data.z, 7); //it take 1000us to transfer

	if (fc.temp_next < task_get_ms_tick())
	{
		switch (fc.temp_step)
		{
			case(0):
				sht21.StartHumidity();
			break;
			case(1):
				sht21.Read();
			break;
			case(2):
				sht21.CompensateHumidity();
				fc.humidity = sht21.humidity;
			break;
			case(3):
				sht21.StartTemperature();
			break;
			case(4):
				sht21.Read();
			break;
			case(5):
				sht21.CompensateTemperature();
				fc.temperature = sht21.temperature;
			break;
		}
		fc.temp_next = task_get_ms_tick() + FC_TEMP_PERIOD;
		fc.temp_step = (fc.temp_step + 1) % 6;
	}

//	DEBUG("$;%d;%d;%d", fc.acc_data.x, fc.acc_data.y, fc.acc_data.z);
//	DEBUG(";%d;%d;%d", fc.mag_data.x, fc.mag_data.y, fc.mag_data.z);
//	DEBUG(";%d;%d;%d", fc.gyro_data.x, fc.gyro_data.y, fc.gyro_data.z);
//	DEBUG(";%0.0f\n", ms5611.pressure);

	BT_ALLOW_TX
}

void fc_takeoff()
{
	if (!fc.baro_valid)
		return;

	gui_showmessage_P(PSTR("Take off"));

	fc.flight_state = FLIGHT_FLIGHT;
	fc.flight_timer = time_get_actual();

	//reset timer and altitude for autoland
	fc.autostart_altitude = fc.altitude1;
	fc.autostart_timer = time_get_actual();

	//zero altimeters at take off
	for (uint8_t i = 0; i < NUMBER_OF_ALTIMETERS; i++)
	{
		if (config.altitude.altimeter[i].flags & ALT_AUTO_ZERO)
			fc_zero_alt(i + 1);
	}

	logger_start();
}

uint8_t fc_landing_old_gui_task;

void fc_landing_cb(uint8_t ret)
{
	gui_switch_task(fc_landing_old_gui_task);
}

void fc_landing()
{
	gui_dialog_set_P(PSTR("Landing"), PSTR(""), GUI_STYLE_STATS, fc_landing_cb);
	fc_landing_old_gui_task = gui_task;
	gui_switch_task(GUI_DIALOG);

	fc.flight_state = FLIGHT_LAND;
	fc.flight_timer = time_get_actual() - fc.flight_timer;

	audio_off();

	logger_stop();
}

void fc_reset()
{
	//autostart timer
	fc.autostart_timer = time_get_actual();
	fc.flight_state = FLIGHT_WAIT;

	//statistic
	fc.stats.max_alt = -32678;
	fc.stats.min_alt = 32677;
	fc.stats.max_climb = 0;
	fc.stats.max_sink = 0;

}

void fc_sync_gps_time()
{
	uint32_t diff = 0;

	if (time_get_actual() == (fc.gps_data.utc_time + (config.system.time_zone * 3600ul) / 2))
		return;

	//do not change flight time during update
	if (fc.flight_state == FLIGHT_FLIGHT)
		diff = time_get_actual() - fc.flight_timer;

	time_set_actual(fc.gps_data.utc_time + (config.system.time_zone * 3600ul) / 2);

	//do not change flight time during update
	if (fc.flight_state == FLIGHT_FLIGHT)
		fc.flight_timer = time_get_actual() - diff;

	//restart autostart/autoland timer
	fc.autostart_timer = time_get_actual();

	gui_showmessage_P(PSTR("GPS Time set"));
}

void fc_step()
{
	gps_step();

	bt_step();

	protocol_step();

	logger_step();

	//auto start
	// baro valid, waiting to take off, and enabled auto start
	if (fc.baro_valid && (fc.flight_state == FLIGHT_WAIT || fc.flight_state == FLIGHT_LAND) && config.autostart.start_sensititvity > 0)
	{
		if (abs(fc.altitude1 - fc.autostart_altitude) > config.autostart.start_sensititvity)
		{
			fc_takeoff();
		}
		else
		{
			//reset wait timer
			if (time_get_actual() - fc.autostart_timer > config.autostart.timeout)
			{
				fc.autostart_timer = time_get_actual();
				fc.autostart_altitude = fc.altitude1;
			}
		}
	}

	//auto land
	// flying and auto land enabled
	if (fc.flight_state == FLIGHT_FLIGHT && config.autostart.land_sensititvity > 0)
	{
		if (abs(fc.altitude1 - fc.autostart_altitude) < config.autostart.land_sensititvity)
		{
			if (time_get_actual() - fc.autostart_timer > config.autostart.timeout)
			{
				//reduce timeout from flight time
				fc.flight_timer += config.autostart.timeout;

				gui_reset_timeout();
				fc_landing();
			}
		}
		else
		{
			fc.autostart_altitude = fc.altitude1;
			fc.autostart_timer = time_get_actual();
		}

	}


	//gps time sync
	if ((config.system.time_flags & TIME_SYNC) && fc.gps_data.fix_cnt == GPS_FIX_TIME_SYNC)
	{
		if (config.gui.menu_audio_flags & CFG_AUDIO_MENU_GPS)
			seq_start(&gps_ready, config.gui.menu_volume);
		fc_sync_gps_time();
		fc.gps_data.fix_cnt++;
	}

	//glide ratio
	//when you have GPS, baro and speed is higher than 2km/h and you are sinking <= -0.01
	if (fc.gps_data.valid && fc.baro_valid && fc.gps_data.groud_speed > FC_GLIDE_MIN_KNOTS && fc.avg_vario <= FC_GLIDE_MIN_SINK)
	{
		float spd_m = fc.gps_data.groud_speed * FC_KNOTS_TO_MPS;
		fc.glide_ratio = spd_m / abs(fc.avg_vario);

		fc.glide_ratio_valid = true;
	}
	else
	{
		fc.glide_ratio_valid = false;
	}

	//flight statistic
	if (fc.flight_state == FLIGHT_FLIGHT)
	{
		int16_t t_vario = fc.avg_vario * 100;

		if (fc.altitude1 > fc.stats.max_alt)
			fc.stats.max_alt = fc.altitude1;
		if (fc.altitude1 < fc.stats.min_alt)
			fc.stats.min_alt = fc.altitude1;

		if (t_vario > fc.stats.max_climb)
			fc.stats.max_climb = t_vario;
		if (t_vario < fc.stats.max_sink)
			fc.stats.max_sink = t_vario;
	}
}

float fc_alt_to_qnh(float alt, float pressure)
{
	return pressure / pow(1.0 - (alt / 44330.0), 5.255);
}

float fc_press_to_alt(float pressure, float qnh)
{
	return 44330.0 * (1 - pow((pressure / qnh), 0.190295));
}

float fc_alt_to_press(float alt, float qnh)
{
	return qnh * pow(1.0 - (alt / 44330.0), 5.255);
}

void fc_zero_alt(uint8_t index)
{
	index -= 1;

	if (config.altitude.altimeter[index].flags & ALT_DIFF)
		{
			uint8_t a_index = config.altitude.altimeter[index].flags & 0x0F;

			if (a_index == 0)
				config.altitude.altimeter[index].delta = -fc.altitude1;
			else
				config.altitude.altimeter[index].delta = -fc.altitudes[a_index];

		}
}

void fc_manual_alt0_change(float val)
{
    kalmanFilter->setXAbs(val);
    if (fc.flight_state == FLIGHT_WAIT)
    {
    	fc.autostart_altitude = val;
    }
}

void fc_log_battery()
{
	char text[32];

	sprintf_P(text, PSTR("bat: %u%% (%u)"), battery_per, battery_adc_raw);

	logger_comment(text);
}
