/*
 * mag.cpp
 *
 *  Created on: Dec 7, 2016
 *      Author: fiala
 */

#include "mag.h"
#include "fc.h"

void mag_save_calibration(vector_float_t sens_vf, vector_float_t bias_vf)
{
	vector_i16_t bias;
	vector_i16_t sens;

	sens.x = int16_t(sens_vf.x * 2);
	sens.y = int16_t(sens_vf.y * 2);
	sens.z = int16_t(sens_vf.z * 2);
	bias.x = int16_t(bias_vf.x * 2);
	bias.y = int16_t(bias_vf.y * 2);
	bias.z = int16_t(bias_vf.z * 2);

	eeprom_busy_wait();
	eeprom_update_block(&bias, &config_ro.calibration.mag_bias, sizeof(config_ro.calibration.mag_bias));
	eeprom_update_block(&sens, &config_ro.calibration.mag_sensitivity, sizeof(config_ro.calibration.mag_sensitivity));

	//DEBUG("written float: bias %f %f %f sens %f %f %f\n", bias_vf.x, bias_vf.y, bias_vf.z, sens_vf.x, sens_vf.y, sens_vf.z);
	//DEBUG("written int: bias %d %d %d sens %d %d %d\n", bias.x, bias.y, bias.z, sens.x, sens.y, sens.z);
}

void mag_load_calibration(volatile vector_float_t * sens_vf, volatile vector_float_t * bias_vf)
{
	vector_i16_t bias;
	vector_i16_t sens;

	eeprom_busy_wait();
	eeprom_read_block(&bias, &config_ro.calibration.mag_bias, sizeof(config_ro.calibration.mag_bias));
	eeprom_read_block(&sens, &config_ro.calibration.mag_sensitivity, sizeof(config_ro.calibration.mag_sensitivity));

	sens_vf->x = (float(sens.x) / 2);
	sens_vf->y = (float(sens.y) / 2);
	sens_vf->z = (float(sens.z) / 2);
	bias_vf->x = (float(bias.x) / 2);
	bias_vf->y = (float(bias.y) / 2);
	bias_vf->z = (float(bias.z) / 2);

	//DEBUG("loaded float: bias %f %f %f sens %f %f %f\n", bias_vf->x, bias_vf->y, bias_vf->z, sens_vf->x, sens_vf->y, sens_vf->z);
	//DEBUG("loaded int: bias %d %d %d sens %d %d %d\n", bias.x, bias.y, bias.z, sens.x, sens.y, sens.z);
}


void mag_calc_init()
{
	//initialize variables
	// ...
	//load config from eeprom
	mag_load_calibration(&fc.mag.sens, &fc.mag.bias);
}


void mag_calc_vector()
{
	//calc magnetic data using bias and sens
	fc.mag.vector.x = float(fc.mag.raw.x - fc.mag.bias.x) / float(fc.mag.sens.x) ;
	fc.mag.vector.y = float(fc.mag.raw.y - fc.mag.bias.y) / float(fc.mag.sens.y) ;
	fc.mag.vector.z = float(fc.mag.raw.z - fc.mag.bias.z) / float(fc.mag.sens.z) ;
}
