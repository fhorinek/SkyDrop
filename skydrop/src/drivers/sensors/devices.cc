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
