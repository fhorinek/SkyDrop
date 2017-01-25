/*
 * accel.h
 *
 *  Created on: Nov 8, 2016
 *      Author: Fiala
 */

#ifndef FC_ACC_H_
#define FC_ACC_H_

#include "../common.h"


void acc_widget_filter(void);
void acc_save_calibration(vector_float_t, vector_float_t);
void acc_load_calibration(volatile vector_float_t * sens_vf, volatile vector_float_t * bias_vf);
void acc_calc_init(void);
void acc_calc_vector(void);
void acc_calc_total(void);

#endif /* FC_ACC_H_ */
