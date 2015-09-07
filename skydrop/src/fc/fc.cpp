#include "fc.h"

#include "../drivers/sensors/devices.h"
#include "../drivers/uart.h"
#include "../drivers/audio/audio.h"

#include "kalman.h"
#include "vario.h"

volatile flight_data_t fc;

Timer fc_meas_timer;

extern KalmanFilter * kalmanFilter;

void fc_init()
{
	DEBUG(" *** Flight computer init ***\n");

	//start values
	active_page = config.gui.last_page;
	if (active_page >= config.gui.number_of_pages)
		active_page = 0;

	fc.epoch_flight_timer = time_get_actual();
	fc.autostart_state = AUTOSTART_WAIT;

	fc.temp_step = 0;


	//init calculators
	vario_init();
	audio_init();

	gps_init();
	if (config.system.use_gps)
		gps_start();

	bt_init();
//	if (fc.use_flage & ENABLE_BT)
//		bt_module_init();

	//VCC to baro, acc/mag gyro
	MEMS_POWER_ON;

	GpioSetDirection(IO0, OUTPUT);
	GpioWrite(IO0, HIGH);

	//init and test i2c
	if (!mems_i2c_init())
	{
		DEBUG("ERROR I2C\n");
		led_set(0xFF, 0, 0);
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

	fc_meas_timer.Init(FC_MEAS_TIMER, timer_div256); //125 == 1ms
	fc_meas_timer.SetInterruptPriority(MEDIUM);
	fc_meas_timer.EnableInterrupts(timer_overflow | timer_compareA | timer_compareB | timer_compareC);
	fc_meas_timer.SetTop(125 * 10); // == 10ms
	fc_meas_timer.SetCompare(timer_A, 100); // == 0.64ms
	fc_meas_timer.SetCompare(timer_B, 430); // == 2.7ms
	fc_meas_timer.SetCompare(timer_C, 555); // == 3.7ms
	fc_meas_timer.Start();

	DEBUG(" *** FC init done ***\n");

}

void fc_deinit()
{
	eeprom_busy_wait();
	//store altimeter info
	eeprom_update_float(&config_ee.altitude.QNH1, config.altitude.QNH1);
	eeprom_update_float(&config_ee.altitude.QNH2, config.altitude.QNH2);


	for (uint8_t i=0; i<NUMBER_OF_ALTIMETERS; i++)
	{
		eeprom_update_word((uint16_t *)&config_ee.altitude.altimeter[i].delta, config.altitude.altimeter[i].delta);
	}

	MEMS_POWER_OFF;
	I2C_POWER_OFF;
}

void fc_pause()
{
	fc_meas_timer.Stop();
}

void fc_continue()
{
	fc_meas_timer.Start();
}


ISR(FC_MEAS_TIMER_OVF)
{
	IO1_HIGH
	ms5611.ReadPressure();
	ms5611.StartTemperature();
	lsm303d.StartReadMag(); //it takes 152us to transfer

	ms5611.CompensatePressure();
	IO1_LOW

}


ISR(FC_MEAS_TIMER_CMPA)
{
	IO1_HIGH
	lsm303d.ReadMag(&fc.mag_data.x, &fc.mag_data.y, &fc.mag_data.z);
	ms5611.ReadTemperature();
	ms5611.StartPressure();
	lsm303d.StartReadAccStream(16); //it take 1600us to transfer

	vario_calc(ms5611.pressure);

	//audio loop
	audio_step();

	if (fc.baro_valid)
	{
		//auto start
		if (fc.autostart_state == AUTOSTART_WAIT)
		{
			if (abs(fc.altitude1 - fc.start_altitude) > config.autostart.sensititvity)
			{
				fc_takeoff();
			}
			else
			{
				//reset timer
				if (time_get_actual() - fc.epoch_flight_timer > FC_AUTOSTART_RESET)
				{
					fc.epoch_flight_timer = time_get_actual();
					fc.start_altitude = fc.altitude1;
				}
			}
		}
	}

	ms5611.CompensateTemperature();

	IO1_LOW
}

ISR(FC_MEAS_TIMER_CMPB)
{
	IO1_HIGH
	lsm303d.ReadAccStreamAvg(&fc.acc_data.x, &fc.acc_data.y, &fc.acc_data.z, 16);
	l3gd20.StartReadGyroStream(7); //it take 1000us to transfer
	IO1_LOW
}

ISR(FC_MEAS_TIMER_CMPC)
{
	IO1_HIGH
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

	IO1_LOW
}

void fc_takeoff()
{
	gui_showmessage_P(PSTR("Take off"));

	fc.autostart_state = AUTOSTART_FLIGHT;
	fc.epoch_flight_timer = time_get_actual();

	//zero altimeters at take off
	for (uint8_t i = 0; i < NUMBER_OF_ALTIMETERS; i++)
	{
		if (config.altitude.altimeter[i].flags & ALT_AUTO_ZERO)
			fc_zero_alt(i + 1);
	}
}

void fc_landing()
{
	gui_showmessage_P(PSTR("Landing"));

	fc.autostart_state = AUTOSTART_LAND;
	fc.epoch_flight_timer = time_get_actual() - fc.epoch_flight_timer;
}

void fc_sync_gps_time()
{
	uint32_t diff = 0;

	//do not change flight time during update
	if (fc.autostart_state == AUTOSTART_FLIGHT)
		diff = time_get_actual() - fc.epoch_flight_timer;

	time_set_actual(fc.gps_data.utc_time + (config.system.time_zone * 3600ul) / 2);

	//do not change flight time during update
	if (fc.autostart_state == AUTOSTART_FLIGHT)
		fc.epoch_flight_timer = time_get_actual() - diff;

	gui_showmessage_P(PSTR("GPS Time set"));
}

void fc_step()
{
	gps_step();
	bt_step();

	//gps time sync
	if ((config.system.time_flags & TIME_SYNC) && fc.gps_data.fix_cnt == GPS_FIX_TIME_SYNC)
	{
		fc_sync_gps_time();
	}

	//glide ratio
	//when you hav GPS, baro and speed is higher than 2km/h and you are sinking
	if (fc.gps_data.valid && fc.baro_valid && fc.gps_data.groud_speed > FC_GLIDE_MIN_KNOTS && fc.avg_vario < 0.0)
	{
		float spd_m = fc.gps_data.groud_speed * FC_KNOTS_TO_MPS;
		fc.glide_ratio = spd_m / abs(fc.avg_vario);

		fc.glide_ratio_valid = true;
	}
	else
	{
		fc.glide_ratio_valid = false;
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
			{
				config.altitude.altimeter[index].delta = -fc.altitude1;
			}
			else
				config.altitude.altimeter[index].delta = -fc.altitudes[a_index];

		}
}

void fc_manual_alt0_change(float val)
{
    kalmanFilter->setXAbs(val);
    fc.start_altitude = val;
}
