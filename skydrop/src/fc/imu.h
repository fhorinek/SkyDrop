/*
 * imu.h
 *
 *  Created on: Jan 31, 2017
 *      Author: fiala
 */

#ifndef FC_IMU_H_
#define FC_IMU_H_

#include "../common.h"

struct imu_gyro_t
{
	uint16_t count;
	vector_float_t sum;
	vector_float_t bias;

};

struct imu_quaternion_t
{
	float q0;
	float q1;
	float q2;
	float q3;

};

struct imu_data_t
{
	float quat[4];
};


void imu_init();
void imu_step();

#endif /* FC_IMU_H_ */
