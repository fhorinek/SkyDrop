/*
 * mag.cpp
 *
 *  Created on: Dec 7, 2016
 *      Author: fiala
 */

#include "mag.h"
#include "fc.h"

mag_calc_data_t mag_calc_data;


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

void mag_load_calibration(vector_float_t * sens_vf, vector_float_t * bias_vf)
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
	mag_load_calibration( &mag_calc_data.calibration.sens, &mag_calc_data.calibration.bias );
}


void mag_calc()
{
	//calc magnetic data using bias and sens
	fc.mag_data.x = float(fc.mag_data_raw.x - mag_calc_data.calibration.bias.x) / float(mag_calc_data.calibration.sens.x) ;
	fc.mag_data.y = float(fc.mag_data_raw.y - mag_calc_data.calibration.bias.y) / float(mag_calc_data.calibration.sens.y) ;
	fc.mag_data.z = float(fc.mag_data_raw.z - mag_calc_data.calibration.bias.z) / float(mag_calc_data.calibration.sens.z) ;

}
