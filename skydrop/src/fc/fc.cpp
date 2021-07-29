#include <fc/navigation.h>
#include "fc.h"

#include "../drivers/sensors/devices.h"
#include "../drivers/uart.h"
#include "../drivers/audio/audio.h"
#include "../drivers/audio/sequencer.h"

#include "kalman.h"
#include "vario.h"
#include "agl.h"
#include "airspace.h"
#include "compass.h"
#include "waypoint.h"
#include "alt_calibration.h"
#include "circling.h"


#include "protocols/protocol.h"

#include "logger/logger.h"

#include "../gui/gui_dialog.h"

//#include "debug_on.h"

volatile flight_computer_data_t fc;

Timer fc_meas_timer;

extern KalmanFilter kalmanFilter;

MK_SEQ(gps_ready, ARR({750, 0, 750, 0, 750, 0}), ARR({250, 150, 250, 150, 250, 150}));

#define FC_LOG_BATTERY_EVERY	(5 * 60 * 1000ul)
uint32_t fc_log_battery_next = 0;

void fc_init()
{
	DEBUG(" *** Flight computer init ***\n");

	//start values
	active_page = config.gui.last_page;
	if (active_page >= config.gui.number_of_pages)
		active_page = 0;

	gui_page_set_mode(PAGE_MODE_PREPARE);

	//reset flight status
	fc_reset();

	//home valid if loaded from SD
	if (config.home.flags & HOME_LOADED)
		fc.flight.home_valid = true;
	else
		fc.flight.home_valid = false;

	// We do not have a valid waypoint
	fc.task.active = false;

	// Todo: read/write into eeprom
	fc.odometer = 0;

	//total time
	ee_read_dword(&config_ro.total_flight_time, fc.flight.total_time);
	if (fc.flight.total_time == 0xFFFFFFFF)
		fc.flight.total_time = 0;

	//using fake data
	#ifdef FAKE_ENABLE
		return;
	#endif

	//temperature state machine
	fc.temp.step = 0;

	//init DMA
	DMA_PWR_ON;

	//init calculators
	audio_init();
	logger_init();
	protocol_init();
	wind_init();
	agl_init();
	airspace_init();
	gyro_init();
	imu_init();
	vario_init();
	compass_init();
	navigation_init();

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

		hw_revision = HW_REW_1504;
		
		ee_update_byte(&config_ro.hw_revision, hw_revision);
		

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
			
			ee_update_byte(&config_ro.hw_revision, hw_revision);
			

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

	lsm_cfg.accOdr = lsm_acc_1600Hz;
	lsm_cfg.accScale = lsm_acc_8g;

	lsm_cfg.magOdr = lsm_mag_100Hz;
	lsm_cfg.magScale = lsm_mag_4g;
	lsm_cfg.magHiRes = true;

	lsm_cfg.tempEnable = false;

	//Acceleration calculation init
	acc_calc_init();
	//Magnetic field calculation init
	mag_calc_init();

	//Gyro
	l3gd20_settings l3g_cfg;
	l3g_cfg.bw = l3g_100Hz;
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

	if (fc.flight.state == FLIGHT_HIKE)
		fc_end_hike();

	if (fc.flight.state == FLIGHT_FLIGHT)
		fc_landing();

	
	//store altimeter settings
	ee_update_float(&config_ee.altitude.QNH1, config.altitude.QNH1);
	ee_update_float(&config_ee.altitude.QNH2, config.altitude.QNH2);

	if (config.connectivity.use_bt)
		bt_stop();

	if (config.connectivity.use_gps)
		gps_stop();

	for (uint8_t i=0; i < NUMBER_OF_ALTIMETERS; i++)
	{
		ee_update_word((uint16_t *)&config_ee.altitude.altimeter[i].delta, config.altitude.altimeter[i].delta);
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

	ms5611.ReadPressure();

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

	int16_t x, y, z;

	lsm303d.ReadMag(&x, &y, &z);
	ms5611.ReadTemperature();

	ms5611.StartPressure();
	lsm303d.StartReadAccStream(16); //it take 1600us to transfer

	if (hw_revision == HW_REW_1504)
	{
		fc.mag.raw.x = y;
		fc.mag.raw.y = -x;
		fc.mag.raw.z = z;
	}

	if (hw_revision == HW_REW_1506)
	{
		fc.mag.raw.x = -y;
		fc.mag.raw.y = x;
		fc.mag.raw.z = -z;
	}

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

	int16_t x, y, z;

	lsm303d.ReadAccStreamAvg(&x, &y, &z, 16);
	l3gd20.StartReadGyroStream(7); //it take 1000us to transfer

	if (hw_revision == HW_REW_1504)
	{
		fc.acc.raw.x = -y;
		fc.acc.raw.y = x;
		fc.acc.raw.z = -z;
	}

	if (hw_revision == HW_REW_1506)
	{
		fc.acc.raw.x = y;
		fc.acc.raw.y = -x;
		fc.acc.raw.z = -z;
	}

	acc_calc_vector(); //calculate actual acceleration as vector
	acc_calc_total();	//calculate actual total acceleration from vector data
	acc_widget_filter();  //filter total acceleration for widget

	mag_calc_vector();


	BT_ALLOW_TX
}

//Final fc meas period
// * Load l3gd20 gyroscope data from buffer (free the i2c bus)
// * Handle slow sht21 conversions
ISR(FC_MEAS_TIMER_CMPC)
{
	BT_SUPRESS_TX

	int16_t x, y, z;

	l3gd20.ReadGyroStreamAvg(&x, &y, &z, 7); //it take 1000us to transfer

	if (hw_revision == HW_REW_1504)
	{
		fc.gyro.raw.x = -x;
		fc.gyro.raw.y = -y;
		fc.gyro.raw.z = z;
	}

	if (hw_revision == HW_REW_1506)
	{
		fc.gyro.raw.x = -y;
		fc.gyro.raw.y = x;
		fc.gyro.raw.z = z;
	}

	if (fc.temp.cnt >= FC_TEMP_PERIOD)
	{
		fc.temp.cnt = 0;

		switch (fc.temp.step)
		{
			case(0):
				sht21.StartHumidity();
			break;
			case(1):
				sht21.Read();
			break;
			case(2):
				sht21.CompensateHumidity();
				fc.temp.humid = sht21.humidity;
			break;
			case(3):
				sht21.StartTemperature();
			break;
			case(4):
				sht21.Read();
			break;
			case(5):
				sht21.CompensateTemperature();
				fc.temp.temp = sht21.temperature;
			break;
		}
		fc.temp.step = (fc.temp.step + 1) % 6;
	}
	else
	{
		fc.temp.cnt++;
	}

//	DEBUG("$;%d;%d;%d", fc.acc_data.x, fc.acc_data.y, fc.acc_data.z);
//	DEBUG(";%d;%d;%d", fc.mag_data.x, fc.mag_data.y, fc.mag_data.z);
//	DEBUG(";%d;%d;%d", fc.gyro_data.x, fc.gyro_data.y, fc.gyro_data.z);
//	DEBUG(";%0.0f\n", ms5611.pressure);

	gyro_calc_vector();
	imu_step();

	BT_ALLOW_TX
}

void fc_hike()
{
	fc_reset();

	fc.flight.state = FLIGHT_HIKE;
	fc.flight.timer = task_get_ms_tick();
	fc.flight.autostart_odo = fc.odometer;

	gui_showmessage_P(PSTR("Hike start"));
}

void fc_save_stats()
{
	logger_comment(PSTR(" SKYDROP-START-s: %lu "), time_get_local() - (fc.flight.timer / 1000));
	logger_comment(PSTR(" SKYDROP-DURATION-ms: %lu "), fc.flight.timer);
	logger_comment(PSTR(" SKYDROP-ALT-MAX-m: %d "), fc.flight.stats.max_alt);
	logger_comment(PSTR(" SKYDROP-ALT-MIN-m: %d "), fc.flight.stats.min_alt);
	logger_comment(PSTR(" SKYDROP-CLIMB-MAX-cm: %d "), fc.flight.stats.max_climb);
	logger_comment(PSTR(" SKYDROP-SINK-MAX-cm: %d "), fc.flight.stats.max_sink);
	logger_comment(PSTR(" SKYDROP-ODO-m: %lu "), fc.odometer - fc.flight.autostart_odo);
}

void fc_end_hike()
{
	fc.flight.state = FLIGHT_WAIT;
	gui_showmessage_P(PSTR("Hike end"));
	fc.flight.timer = task_get_ms_tick() - fc.flight.timer;

	//prevent auto start
	fc.flight.autostart_timer = task_get_ms_tick();
	fc.flight.autostart_altitude = fc.altitude1;

	fc_save_stats();

	logger_stop();
}

void fc_takeoff()
{
	if (!fc.vario.valid)
		return;

	if (fc.flight.state == FLIGHT_HIKE)
		fc_end_hike();

	gui_showmessage_P(PSTR("Take off"));

	gui_page_set_mode(PAGE_MODE_NORMAL);

	fc.flight.state = FLIGHT_FLIGHT;
	fc.flight.timer = task_get_ms_tick();

	//reset timer and altitude for autoland
	fc.flight.autostart_altitude = fc.altitude1;
	fc.flight.autostart_timer = task_get_ms_tick();
	fc.flight.autostart_odo = fc.odometer;

	//set start position
	if (fc.gps_data.valid)
	{
		if (config.home.flags & HOME_TAKEOFF)
		{
			fc.flight.home_valid = true;
			config.home.lat = fc.gps_data.latitude;
			config.home.lon = fc.gps_data.longtitude;
		}
	}

	//reset battery info timer
	fc_log_battery_next = 0;

	//zero altimeters at take off
	for (uint8_t i = 0; i < NUMBER_OF_ALTIMETERS; i++)
	{
		if (config.altitude.altimeter[i].flags & ALT_AUTO_ZERO)
			fc_zero_alt(i + 1);
	}
}

uint8_t fc_landing_old_gui_task;

void fc_landing_cb(uint8_t ret)
{
	gui_switch_task(fc_landing_old_gui_task);
}

void fc_landing()
{
	DEBUG("Landing\n");

	gui_page_set_mode(PAGE_MODE_LANDED);

	gui_dialog_set_P(PSTR("Landing"), PSTR(""), GUI_STYLE_STATS, fc_landing_cb);
	fc_landing_old_gui_task = gui_task;
	gui_switch_task(GUI_DIALOG);

	audio_off();

	fc.flight.state = FLIGHT_LAND;
	fc.flight.autostart_timer = task_get_ms_tick();
	fc.flight.timer = task_get_ms_tick() - fc.flight.timer;

	fc.flight.total_time += fc.flight.timer / 1000;

	
	ee_update_dword(&config_ro.total_flight_time, fc.flight.total_time);

	fc_save_stats();

	logger_stop();
}

void fc_reset()
{
	//autostart timer reset
	fc.flight.autostart_timer = task_get_ms_tick();
	fc.flight.state = FLIGHT_WAIT;

	//statistic
	fc.flight.stats.max_alt = -32678;
	fc.flight.stats.min_alt = 32677;
	fc.flight.stats.max_climb = 0;
	fc.flight.stats.max_sink = 0;

	fc.altitude_alarm_status = 0b00000000;

	circling_reset();
}

void fc_sync_gps_time()
{
	DEBUG("fc_sync_gps_time\n");

	//time is already synced
	if (time_get_local() == (fc.gps_data.utc_time + config.system.time_zone * 1800ul))
		return;

	//Do not update time during flight, except when the time is not valid
	if (fc.flight.state == FLIGHT_FLIGHT && time_is_set())
		return;

	DEBUG("Syncing time\n");
	DEBUG(" local    %lu\n", time_get_local());
	DEBUG(" gps + tz %lu\n", fc.gps_data.utc_time + config.system.time_zone * 1800ul);

	time_set_utc(fc.gps_data.utc_time);

//	gui_showmessage_P(PSTR("GPS Time set"));

	time_set_flags();
}


void fc_step()
{
	//using fake data
	#ifdef FAKE_ENABLE
		return;
	#endif

	agl_step(); //it is before gps_step, so new gps fix will be processed in next loop

	airspace_step();   // must be after agl_step, because it needs data from there.

	alt_calibration_step();

	gps_step();

	bt_step();

	protocol_step();

	logger_step();

	wind_step();

	compass_step();

	navigation_step();

	circling_step();

	//logger always enabled
	if (config.autostart.flags & AUTOSTART_ALWAYS_ENABLED)
	{
		if (fc.vario.valid && fc.flight.state == FLIGHT_WAIT)
		{
			fc_takeoff();
		}
	}
	else
	{
		//auto start
		// baro valid, waiting to take off or landed, and enabled auto start
		if (fc.vario.valid && (fc.flight.state == FLIGHT_WAIT || fc.flight.state == FLIGHT_LAND))
		{
			//gps speed enabled and gps avalible
			bool speed_hi = false;
			if (config.autostart.gps_speed > 0 && fc.gps_data.valid)
				speed_hi = fc.gps_data.ground_speed * FC_KNOTS_TO_KPH > config.autostart.gps_speed;

			if ((abs(fc.altitude1 - fc.flight.autostart_altitude) > config.autostart.start_sensititvity  && config.autostart.start_sensititvity > 0) && speed_hi)
			{
				fc_takeoff();
			}
			else
			{
				uint32_t t = task_get_ms_tick();

				if(t < fc.flight.autostart_timer)
				{
					assert(0);
					DEBUG("old %lu\n", fc.flight.autostart_timer);
					DEBUG("act %lu\n", t);
				}

				//reset wait timer
				if (t - fc.flight.autostart_timer > (uint32_t)config.autostart.timeout * 1000ul)
				{
					fc.flight.autostart_timer = t;
					fc.flight.autostart_altitude = fc.altitude1;
				}
			}
		}

		//auto land
		// flying and auto land enabled
		if (fc.flight.state == FLIGHT_FLIGHT && config.autostart.land_sensititvity > 0)
		{
			//gps speed enabled and gps avalible
			bool speed_low = true;
			if (config.autostart.gps_speed > 0 && fc.gps_data.valid)
				speed_low = fc.gps_data.ground_speed * FC_KNOTS_TO_KPH < config.autostart.gps_speed;

			if ((abs(fc.altitude1 - fc.flight.autostart_altitude) < config.autostart.land_sensititvity) && speed_low)
			{
				uint32_t tick = task_get_ms_tick();

				if (tick < fc.flight.autostart_timer)
				{
					assert(0);
					DEBUG("TT %lu\n", tick);
					DEBUG("AT %lu\n", fc.flight.autostart_timer);
				}
				else
				if (tick - fc.flight.autostart_timer > (uint32_t)config.autostart.timeout * 1000ul)
				{
					//reduce timeout from flight time
					fc.flight.timer += (uint32_t)config.autostart.timeout * 1000ul;

					gui_reset_timeout();
					fc_landing();
				}
			}
			else
			{
				fc.flight.autostart_altitude = fc.altitude1;
				fc.flight.autostart_timer = task_get_ms_tick();
			}
		}
	}

	//altitude alarm
	if (config.altitude.alarm_enabled && fc.flight.state == FLIGHT_FLIGHT)
	{
		if (fc_alarm_running(fc_active_alarm()))
		{
			gui_switch_task(GUI_ALARM);
		}
	}

	//gps time sync
	if ((config.system.time_flags & TIME_SYNC) && fc.gps_data.fix_cnt == GPS_FIX_TIME_SYNC)
	{
		if (config.gui.menu_audio_flags & CFG_AUDIO_MENU_GPS)
			seq_start(&gps_ready, config.gui.alert_volume);

		fc_sync_gps_time();
		fc.gps_data.fix_cnt++;
	}

	//glide ratio
	//when you have GPS, baro and speed is higher than 2km/h and you are sinking <= -0.01
	if (fc.gps_data.valid && fc.vario.valid && fc.gps_data.ground_speed > FC_GLIDE_MIN_KNOTS && fc.vario.avg <= FC_GLIDE_MIN_SINK)
	{
		float spd_m = fc.gps_data.ground_speed * FC_KNOTS_TO_MPS;
		fc.glide_ratio = spd_m / abs(fc.vario.avg);

		fc.glide_ratio_valid = true;
	}
	else
	{
		fc.glide_ratio_valid = false;
	}

	//flight logger
	if (config.logger.enabled)
	{
		if (fc.flight.state == FLIGHT_FLIGHT && !logger_active() && time_is_set() && !logger_error())
		{
			logger_start();
		}

		if (fc.flight.state == FLIGHT_HIKE && !logger_active() && time_is_set() && !logger_error())
		{
			logger_start();
		}
	}

	//flight statistic
	if (fc.flight.state == FLIGHT_FLIGHT || fc.flight.state == FLIGHT_HIKE)
	{
		int16_t t_vario = fc.vario.avg * 100;

		if (fc.altitude1 > fc.flight.stats.max_alt)
			fc.flight.stats.max_alt = fc.altitude1;
		if (fc.altitude1 < fc.flight.stats.min_alt)
			fc.flight.stats.min_alt = fc.altitude1;

		if (t_vario > fc.flight.stats.max_climb)
			fc.flight.stats.max_climb = t_vario;
		if (t_vario < fc.flight.stats.max_sink)
			fc.flight.stats.max_sink = t_vario;
	}

	//flight modes
    if (fc.flight.state == FLIGHT_FLIGHT)
    {
        if (fc.vario.avg < config.gui.page_acro_thold)
        {
            gui_page_set_mode(PAGE_MODE_ACRO);
        }
        else
        {
        	//if total heading is > 360 : normal -> circling
        	//if total heading is < 340 : circling -> normal
        	//			AND
        	//circl only if avg heading > config thold
            if (((abs(fc.flight.total_heading_change) > 360 && !fc.flight.circling) ||
              	 (abs(fc.flight.total_heading_change) > 340 &&  fc.flight.circling)) &&
                 (abs(fc.flight.avg_heading_change) > PAGE_AUTOSET_CIRCLING_THOLD))
            {
                if (!fc.flight.circling)
                {
                    gui_page_set_mode(PAGE_MODE_CIRCLING);
					fc.flight.circling = true;
					fc.flight.circling_start = task_get_ms_tick();
					fc.flight.circling_start_altitude = fc.altitude1;
                }

                fc.flight.circling_stop = 0;
                fc.flight.circling_time = (task_get_ms_tick() - fc.flight.circling_start) / 1000;
                fc.flight.circling_gain = fc.altitude1 - fc.flight.circling_start_altitude;
            }
            else
            {
            	if (fc.flight.circling)
            	{
                	if (fc.flight.circling_stop == 0)
                		fc.flight.circling_stop = task_get_ms_tick();

                	if (task_get_ms_tick() - fc.flight.circling_stop > (config.gui.page_circling_timeout * 1000))
                	{
    					fc.flight.circling = false;
    					fc.flight.total_heading_change = 0;
    					gui_page_set_mode(PAGE_MODE_NORMAL);
                	}
            	}
            }
        }
    }
}

/**
 * Find out, if and which altitude alarm is active.
 *
 * @return 0 if no alarm is active, 1 for alarm_1, 2 for alarm_2, 3 for alarm_h1
 */
uint8_t fc_active_alarm()
{
	//clear suppress flags if altitude is above the alarms
	if (fc.altitude1 > config.altitude.alarm_1 + config.altitude.alarm_reset)
		fc.altitude_alarm_status &= ~FC_ALT_ALARM1_SUPPRESS;

	if (fc.altitude1 > config.altitude.alarm_2 + config.altitude.alarm_reset)
		fc.altitude_alarm_status &= ~FC_ALT_ALARM2_SUPPRESS;

	if (fc.altitude1 < config.altitude.alarm_h1 - config.altitude.alarm_reset)
		fc.altitude_alarm_status &= ~FC_ALT_ALARM_H1_SUPPRESS;

	if (fc.altitude1 < config.altitude.alarm_2)
		return 2;

	if (fc.altitude1 < config.altitude.alarm_1)
		return 1;

	if (fc.altitude1 > config.altitude.alarm_h1)
		return 3;

	return 0;
}

/**
 * Find out, if the given alarm is running (not suppressed).
 *
 * @return alarm, if it is not suppressed, otherwise 0
 */
uint8_t fc_alarm_running(uint8_t alarm)
{
	switch (alarm)
	{
		case (1):
			if (!(fc.altitude_alarm_status & FC_ALT_ALARM1_SUPPRESS))
				return alarm;
		break;
		case (2):
			if (!(fc.altitude_alarm_status & FC_ALT_ALARM2_SUPPRESS))
				return alarm;
		break;
		case (3):
			if (!(fc.altitude_alarm_status & FC_ALT_ALARM_H1_SUPPRESS))
				return alarm;
		break;
	}

	return 0;
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
	kalmanFilter.Reset(val);
	fc.altitude1 = val;

    if (fc.flight.state == FLIGHT_WAIT || fc.flight.state == FLIGHT_LAND)
    	fc.flight.autostart_altitude = val;
}

void fc_log_battery()
{
	if (fc_log_battery_next > task_get_ms_tick())
		return;

	fc_log_battery_next = task_get_ms_tick() + FC_LOG_BATTERY_EVERY;

	if (battery_per == BATTERY_CHARGING)
		logger_comment(PSTR("bat: chrg"));
	else if (battery_per == BATTERY_FULL)
		logger_comment(PSTR("bat: full"));
	else
		logger_comment(PSTR("bat: %u%% (%u)"),  battery_per, battery_adc_raw);

	logger_comment(PSTR("temp: %d %d%%"), fc.temp.temp / 10, fc.temp.humid / 100);
}
