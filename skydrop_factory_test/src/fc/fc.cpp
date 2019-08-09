#include "fc.h"

#include "../drivers/sensors/devices.h"
#include "../drivers/uart.h"
#include "../drivers/audio/audio.h"
#include "../drivers/audio/sequencer.h"

#include "vario.h"
#include "compass.h"

#include "protocols/protocol.h"

#include "../gui/gui_dialog.h"

//#include "debug_on.h"

volatile flight_computer_data_t fc;

Timer fc_meas_timer;

MK_SEQ(gps_ready, ARR({750, 0, 750, 0, 750, 0}), ARR({250, 150, 250, 150, 250, 150}));

#define FC_LOG_BATTERY_EVERY	(5 * 60 * 1000ul)
uint32_t fc_log_battery_next = 0;

void fc_init()
{
	//temperature state machine
	fc.temp.step = 0;

	//init DMA
	DMA_PWR_ON;

	//init calculators

	audio_init();
	protocol_init();
	gyro_init();
	imu_init();
	vario_init();
	compass_init();

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
	l3g_cfg.enabled = true;
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

	if (config.connectivity.use_bt)
		bt_stop();

	if (config.connectivity.use_gps)
		gps_stop();

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

void fc_step()
{
	gps_step();

	bt_step();

	protocol_step();

	compass_step();
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
