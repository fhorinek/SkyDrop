/*
 * gyro.c
 *
 *  Created on: Mar 14, 2017
 *      Author: fiala
 */

#include "gyro.h"
#include "fc.h"
#include "../common.h"

#define GYRO_SENS 16.384 // 2^16 / 4000 deg per sec

void gyro_save_calibration(volatile vector_float_t * bias_vf)
{
	vector_i16_t bias;

	bias.x = int16_t(bias_vf->x * 1000.0 );
	bias.y = int16_t(bias_vf->y * 1000.0 );
	bias.z = int16_t(bias_vf->z * 1000.0 );

	eeprom_busy_wait();
	eeprom_update_block(&bias, &config_ro.gyro_bias, sizeof(config_ro.gyro_bias));

	//DEBUG("written float: bias %.2f %.2f %.2f\n", bias_vf->x, bias_vf->y, bias_vf->z);
	//DEBUG("written int: bias %d %d %d\n", bias.x, bias.y, bias.z);

}

void gyro_load_calibration(volatile vector_float_t * bias_vf)
{
	vector_i16_t bias;

	eeprom_busy_wait();
	eeprom_read_block(&bias, &config_ro.gyro_bias, sizeof(config_ro.gyro_bias));

	bias_vf->x = (float(bias.x) / 1000.0);
	bias_vf->y = (float(bias.y) / 1000.0);
	bias_vf->z = (float(bias.z) / 1000.0);

	//DEBUG("loaded float: bias %0.2f %0.2f %0.2f\n", bias_vf->x, bias_vf->y, bias_vf->z);
	//DEBUG("loaded int: bias %d %d %d\n", bias.x, bias.y, bias.z);
}

void gyro_init()
{
	//initialize variables

	// ...

	//load config from eeprom
	gyro_load_calibration( &fc.gyro.bias);
}


void gyro_calc_vector()
{
	//calc gyro data using bias and sens
	fc.gyro.vector.x = ( float(fc.gyro.raw.x) - fc.gyro.bias.x ) / GYRO_SENS * 0.01 ;
	fc.gyro.vector.y = ( float(fc.gyro.raw.y) - fc.gyro.bias.y ) / GYRO_SENS * 0.01 ;
	fc.gyro.vector.z = ( float(fc.gyro.raw.z) - fc.gyro.bias.z ) / GYRO_SENS * 0.01 ;

}


