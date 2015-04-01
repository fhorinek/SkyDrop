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
	eeprom_busy_wait();
	flight_data.QNH1 = eeprom_read_float(&config.altitude.QNH1);
	flight_data.QNH2 = eeprom_read_float(&config.altitude.QNH2);

	flight_data.digital_vario_dampening = eeprom_read_float(&config.vario.digital_vario_dampening);
	flight_data.avg_vario_dampening = eeprom_read_float(&config.vario.avg_vario_dampening);

	//init calculators
	vario_init();
	//gps_init();

	//VCC to baro and acc/mag
	MEMS_POWER_ON;
	//Enable I2c pull-ups
	I2C_POWER_ON;
	//Enable I2C peripheral
	MEMS_I2C_PWR_ON;

	//stabilize power
	_delay_ms(1);

	mems_i2c.InitMaster(MEMS_I2C, 800000ul);
	mems_i2c.Scan();

	//Barometer
	ms5611.Init(&mems_i2c, MS5611_ADDRESS_CSB_LO);

	//Magnetometer + Accelerometer
	lsm303d_settings lsm_cfg;

	lsm_cfg.enabled = true;
	lsm_cfg.accOdr = lsm_acc_1600Hz;
	lsm_cfg.accScale = lsm_acc_4g;

	lsm_cfg.magOdr = lsm_mag_100Hz;
	lsm_cfg.magScale = lsm_mag_4g;
	lsm_cfg.magHiRes = true;

	lsm_cfg.tempEnable = false;

	//XXX: do self-test?
	lsm303d.Init(&mems_i2c, lsm_cfg);
	lsm303d.Start();

	//Measurement timer
	FC_MEAS_TIMER_PWR_ON;

	fc_meas_timer.Init(FC_MEAS_TIMER, timer_div256); //125 == 1ms
	fc_meas_timer.SetInterruptPriority(MEDIUM);
	fc_meas_timer.EnableInterrupts(timer_overflow | timer_compareA);
	fc_meas_timer.SetTop(125 * 10); // == 10ms
	fc_meas_timer.SetCompare(timer_A, 100); // == 0.64ms
	fc_meas_timer.Start();

	DEBUG(" *** FC init done ***\n");
}

ISR(FC_MEAS_TIMER_OVF)
{
//	GPIO2_HIGH
	ms5611.ReadPressure();
	ms5611.StartTemperature();

	ms5611.CompensatePressure();
	vario_calc(ms5611.pressure);
//	GPIO2_LOW
}

ISR(FC_MEAS_TIMER_CMPA)
{
//	GPIO2_HIGH
	ms5611.ReadTemperature();
	ms5611.StartPressure();
	ms5611.CompensateTemperature();

	lsm303d.ReadMag(&flight_data.mag_data.x, &flight_data.mag_data.y, &flight_data.mag_data.z);
	lsm303d.ReadAccStreamAvg(&flight_data.acc_data.x, &flight_data.acc_data.y, &flight_data.acc_data.z, 16);
//	GPIO2_LOW
}


void fc_step()
{
	//gps_step();

}
