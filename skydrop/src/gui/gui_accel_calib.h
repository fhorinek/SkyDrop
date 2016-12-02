/*
 * gui_accel_calib.h
 *
 *  Created on: Nov 21, 2016
 *      Author: Fiala
 */

#ifndef GUI_GUI_ACCEL_CALIB_H_
#define GUI_GUI_ACCEL_CALIB_H_

#include "gui.h"
#include "../common.h"

extern acc_data_t acc_data;

struct accel_calib_calc_t
{
	int16_t old_value;
	uint8_t counter;
	int32_t avg_value;
	vector_i16_t pos_val;
	vector_i16_t neg_val;
};
struct accel_valid_t
{
	bool Xpos;
	bool Ypos;
	bool Zpos;
	bool Xneg;
	bool Yneg;
	bool Zneg;
};
struct accel_calib_t
{
	accel_calib_calc_t calc;
	accel_valid_t valid;
	bool new_calib;
	bool is_sampling;
	accel_valid_t sampling;

	vector_float_t bias;
	vector_float_t sens;
	vector_i32_t filtered_raw;
	float filtered_total_new;
	float filtered_total_old;
};


//void gui_accelerometer_calib(void);
void gui_accelerometer_calib_init(void);
void gui_accelerometer_calib_stop(void);
void gui_accelerometer_calib_loop(void);
void gui_accelerometer_calib_irqh(uint8_t type, uint8_t * buff);

#endif /* GUI_GUI_ACCEL_CALIB_H_ */
