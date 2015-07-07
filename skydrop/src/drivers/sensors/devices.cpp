/*
 * devices.cc
 *
 *  Created on: 30.7.2014
 *      Author: horinek
 */

#include "devices.h"
#include "../../fc/fc.h"

I2c mems_i2c;
Lsm303d lsm303d;
MS5611 ms5611;
L3gd20 l3gd20;
SHT21 sht21;

bool mems_i2c_ok = false;

bool mems_i2c_init()
{
	//Enable I2c pull-ups
	I2C_POWER_ON;
	//Enable I2C peripheral
	MEMS_I2C_PWR_ON;

	//stabilize power
	_delay_ms(10);

	mems_i2c.InitMaster(MEMS_I2C, 800000ul, 100, 8);
	mems_i2c.StartTransmittion(0,0);
	_delay_ms(1);
	if (mems_i2c.Status() != i2c_idle)
		return false;

	mems_i2c_ok = true;
	return true;
}

bool mems_i2c_selftest()
{
	return mems_i2c_ok;
}

int32_t to_dec_3(int64_t c)
{
	if (c < (int64_t)0x800000)
		return c;
	return (int64_t)c - (int64_t)0x1000000;
}

int16_t to_dec_2(int32_t c)
{
	if (c < (int32_t)0xFF)
		return c;
	return (int32_t)c - (int32_t)0x100;
}

int8_t to_dec_1(int8_t c)
{
	if (c<32) return c;
	return c - 64;
}

ISR(AM_INT1_INT)
{
//	fc_mag_irq();
}

ISR(AM_INT2_INT)
{
//	fc_acc_irq();
}
