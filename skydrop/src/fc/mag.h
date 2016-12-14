/*
 * mag.h
 *
 *  Created on: Dec 7, 2016
 *      Author: fiala
 */

#ifndef FC_MAG_H_
#define FC_MAG_H_

#include "../common.h"

struct mag_calibration_t
{
	vector_float_t bias;
	vector_float_t sens;
};

struct mag_calc_data_t
{
	mag_calibration_t calibration;
};

void mag_save_calibration(vector_float_t , vector_float_t );
void mag_load_calibration(vector_float_t * , vector_float_t * );
void mag_calc_init();
void mag_calc();

#endif /* FC_MAG_H_ */
