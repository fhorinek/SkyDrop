/*
 * accel.h
 *
 *  Created on: Nov 8, 2016
 *      Author: Fiala
 */

#ifndef FC_ACC_H_
#define FC_ACC_H_

#include "../common.h"


struct acc_calibration
{
	vector_float_t sens;
	vector_float_t bias;
};

struct acc_data_t
{
	acc_calibration calibration;
	float max_acc;
	float total_acc;
	uint8_t hold_time;
};

void accel_save_calibration(vector_float_t, vector_float_t);
void accel_load_calibration(vector_float_t * , vector_float_t * );
void accel_calc_init(void);
void accel_calc_vector(void);
void accel_calc_total(void);

#endif /* FC_ACC_H_ */
