/*
 * gui_gyro_calib.h
 *
 *  Created on: Mar 13, 2017
 *      Author: fiala
 */

#ifndef GUI_SETTINGS_GUI_GYRO_CALIB_H_
#define GUI_SETTINGS_GUI_GYRO_CALIB_H_

#include "../gui.h"
#include "../../common.h"

#define gui_gyro_calib_stop gui_dummy

void gui_gyro_calib_init(void);
void gui_gyro_calib_loop(void);
void gui_gyro_calib_irqh(uint8_t type, uint8_t * buff);



#endif /* GUI_SETTINGS_GUI_GYRO_CALIB_H_ */
