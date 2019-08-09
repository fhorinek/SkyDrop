/*
 * devices.h
 *
 *  Created on: 30.7.2014
 *      Author: horinek
 */

#ifndef DEVICES_H_
#define DEVICES_H_

#include "../../skydrop.h"


#include "lsm303d.h"
#include "ms5611.h"
#include "l3gd20.h"
#include "sht21.h"
#include "gps_l80.h"

extern I2c mems_i2c;
extern Lsm303d lsm303d;
extern MS5611 ms5611;
extern L3gd20 l3gd20;
extern SHT21 sht21;



int32_t to_dec_3(int64_t c);
int16_t to_dec_2(int32_t c);
int8_t to_dec_1(int8_t c);

bool mems_i2c_init();
bool mems_i2c_selftest();


#endif /* DEVICES_H_ */
