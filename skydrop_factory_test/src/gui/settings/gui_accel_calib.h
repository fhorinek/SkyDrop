/*
 * gui_accel_calib.h
 *
 *  Created on: Nov 21, 2016
 *      Author: Fiala
 */

#ifndef GUI_GUI_ACCEL_CALIB_H_
#define GUI_GUI_ACCEL_CALIB_H_

#include "../gui.h"
#include "../../common.h"

void gui_accelerometer_calib_init(void);
void gui_accelerometer_calib_stop(void);
void gui_accelerometer_calib_loop(void);
void gui_accelerometer_calib_irqh(uint8_t type, uint8_t * buff);

#endif /* GUI_GUI_ACCEL_CALIB_H_ */
