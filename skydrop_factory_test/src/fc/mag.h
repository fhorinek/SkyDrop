/*
 * mag.h
 *
 *  Created on: Dec 7, 2016
 *      Author: fiala
 */

#ifndef FC_MAG_H_
#define FC_MAG_H_

#include "../common.h"


void mag_save_calibration(vector_float_t , vector_float_t );
void mag_load_calibration(volatile vector_float_t * sens_vf, volatile vector_float_t * bias_vf);
void mag_calc_init();
void mag_calc_vector();

#endif /* FC_MAG_H_ */
