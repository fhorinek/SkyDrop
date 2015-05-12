#include "fc.h"

#include "../drivers/sensors/devices.h"
#include "../drivers/uart.h"

#include "vario.h"

volatile flight_data_t flight_data;

Timer fc_meas_timer;

void fc_init()
{
	DEBUG(" *** Flight computer init ***\n");

	//load configuration
	DEBUG("Loading data form EEPROM\n");
	eeprom_busy_wait();

	flight_data.QNH1 = eeprom_read_float(&config.altitude.QNH1);
	DEBUG("QNH1 %0.1f\n", flight_data.QNH1);

	flight_data.QNH2 = eeprom_read_float(&config.altitude.QNH2);
	DEBUG("QNH2 %0.1f\n", flight_data.QNH2);

	flight_data.digital_vario_dampening = eeprom_read_float(&config.vario.digital_vario_dampening);
	DEBUG("digital_vario_dampening %0.2f\n", flight_data.digital_vario_dampening);

	flight_data.avg_vario_dampening = eeprom_read_float(&config.vario.avg_vario_dampening);
	DEBUG("avg_vario_dampening %0.2f\n", flight_data.avg_vario_dampening);


	for (uint8_t i=0; i<NUMBER_OF_ALTIMETERS; i++)
	{
		DEBUG("altimeter[%d]\n", i);
		flight_data.altimeter[i].altitude = 0;

		flight_data.altimeter[i].flags = eeprom_read_byte(&config.altitude.altimeter[i].flags);
		DEBUG(" flags %02X\n", flight_data.altimeter[i].flags);

		eeprom_read_block((int16_t *) &flight_data.altimeter[i].delta, &config.altitude.altimeter[i].delta, sizeof(int16_t));
		DEBUG(" delta %d\n", flight_data.altimeter[i].delta);
	}

	eeprom_read_block((void *)&flight_data.mag_bias, &config.calibration.mag_bias, sizeof(vector_i16_t));
	DEBUG("mag_bias\n");
	DEBUG(" x %d\n", flight_data.mag_bias.x);
	DEBUG(" y %d\n", flight_data.mag_bias.y);
	DEBUG(" z %d\n", flight_data.mag_bias.z);

	eeprom_read_block((void *)&flight_data.mag_sensitivity, &config.calibration.mag_sensitivity, sizeof(vector_i16_t));
	DEBUG("mag_sensitivity\n");
	DEBUG(" x %d\n", flight_data.mag_sensitivity.x);
	DEBUG(" y %d\n", flight_data.mag_sensitivity.y);
	DEBUG(" z %d\n", flight_data.mag_sensitivity.z);

	eeprom_read_block((void *)&flight_data.acc_bias, &config.calibration.acc_bias, sizeof(vector_i16_t));
	DEBUG("acc_bias\n");
	DEBUG(" x %d\n", flight_data.acc_bias.x);
	DEBUG(" y %d\n", flight_data.acc_bias.y);
	DEBUG(" z %d\n", flight_data.acc_bias.z);

	eeprom_read_block((void *)&flight_data.acc_sensitivity, &config.calibration.acc_sensitivity, sizeof(vector_i16_t));
	DEBUG("acc_sensitivity\n");
	DEBUG(" x %d\n", flight_data.acc_sensitivity.x);
	DEBUG(" y %d\n", flight_data.acc_sensitivity.y);
	DEBUG(" z %d\n", flight_data.acc_sensitivity.z);


	DEBUG("\n");
	//init calculators
	vario_init();

	//gps_init();

	//VCC to baro, acc/mag gyro
	MEMS_POWER_ON;

	GpioSetDirection(IO0, OUTPUT);
	GpioWrite(IO0, HIGH);

	//Enable I2c pull-ups
	I2C_POWER_ON;
	//Enable I2C peripheral
	MEMS_I2C_PWR_ON;

	//stabilize power
	_delay_ms(1);

	mems_i2c.InitMaster(MEMS_I2C, 800000ul, 100, 8);
	mems_i2c.Scan();

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

	//XXX: do self-test?
	lsm303d.Init(&mems_i2c, lsm_cfg);
	lsm303d.Start();

	l3gd20.Init(&mems_i2c, l3g_cfg);
	l3gd20.Start();

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

ISR(FC_MEAS_TIMER_OVF)
{
	IO0_HIGH
	ms5611.ReadPressure();
	ms5611.StartTemperature();
	lsm303d.StartReadMag(); //it takes 152us to transfer

	ms5611.CompensatePressure();
	IO0_LOW

}

ISR(FC_MEAS_TIMER_CMPA)
{
	IO0_HIGH
	lsm303d.ReadMag(&flight_data.mag_data.x, &flight_data.mag_data.y, &flight_data.mag_data.z);
	ms5611.ReadTemperature();
	ms5611.StartPressure();
	lsm303d.StartReadAccStream(16); //it take 1600us to transfer

	vario_calc(ms5611.pressure);
	ms5611.CompensateTemperature();

	IO0_LOW
}

ISR(FC_MEAS_TIMER_CMPB)
{
	IO0_HIGH
	lsm303d.ReadAccStreamAvg(&flight_data.acc_data.x, &flight_data.acc_data.y, &flight_data.acc_data.z, 16);
	l3gd20.StartReadGyroStream(7); //it take 1000us to transfer
	IO0_LOW
}

ISR(FC_MEAS_TIMER_CMPC)
{
	IO0_HIGH
	l3gd20.ReadGyroStreamAvg(&flight_data.gyro_data.x, &flight_data.gyro_data.y, &flight_data.gyro_data.z, 7); //it take 1000us to transfer

	IO0_LOW
}


void fc_step()
{
	//gps_step();

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
