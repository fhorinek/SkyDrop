/*
 * gui_compass_calib.h
 *
 *  Created on: Mar 29, 2017
 *      Author: fiala
 */

#ifndef GUI_SETTINGS_GUI_COMPASS_CALIB_H_
#define GUI_SETTINGS_GUI_COMPASS_CALIB_H_

#include "../gui.h"
#include "../../common.h"

void gui_compass_calib_init(void);
void gui_compass_calib_stop(void);
void gui_compass_calib_loop(void);
void gui_compass_calib_irqh(uint8_t type, uint8_t * buff);

#endif /* GUI_SETTINGS_GUI_COMPASS_CALIB_H_ */
