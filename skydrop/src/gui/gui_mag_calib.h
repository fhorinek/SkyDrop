/*
 * gui_mag_calib.h
 *
 *  Created on: Dec 5, 2016
 *      Author: fiala
 */

#ifndef GUI_GUI_MAG_CALIB_H_
#define GUI_GUI_MAG_CALIB_H_

#include "gui.h"
#include "../common.h"

void gui_mag_calib_init(void);
void gui_mag_calib_stop(void);
void gui_mag_calib_loop(void);
void gui_mag_calib_irqh(uint8_t type, uint8_t * buff);



#endif /* GUI_GUI_MAG_CALIB_H_ */
