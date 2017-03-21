/*
 * set_calib.h
 *
 *  Created on: Dec 5, 2016
 *      Author: fiala
 */

#ifndef GUI_SETTINGS_SET_CALIB_H_
#define GUI_SETTINGS_SET_CALIB_H_

#include "../gui.h"


void gui_set_calib_init();
void gui_set_calib_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_set_calib_action(uint8_t index);

#endif /* GUI_SETTINGS_SET_CALIB_H_ */
