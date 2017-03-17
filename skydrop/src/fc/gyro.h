/*
 * gyro.h
 *
 *  Created on: Mar 14, 2017
 *      Author: fiala
 */

#ifndef FC_GYRO_H_
#define FC_GYRO_H_

#include "../common.h"


void gyro_save_calibration(volatile vector_float_t * );
void gyro_load_calibration(volatile vector_float_t * );
void gyro_init();
void gyro_calc_vector();



#endif /* FC_GYRO_H_ */
