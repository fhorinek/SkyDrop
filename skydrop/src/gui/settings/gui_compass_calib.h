/*
 * gui_compass_calib.h
 *
 *  Created on: Mar 30, 2017
 *      Author: fiala
 */

#ifndef GUI_SETTINGS_GUI_SET_COMPASS_H_
#define GUI_SETTINGS_GUI_SET_COMPASS_H_

#include "../gui.h"

void gui_compass_calib_init();

#define gui_compass_calib_stop gui_dummy
#define gui_compass_calib_loop gui_list_draw
#define gui_compass_calib_irqh gui_list_irqh

void gui_compass_calib_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_compass_calib_action(uint8_t index);



#endif /* GUI_SETTINGS_GUI_SET_COMPASS_H_ */
