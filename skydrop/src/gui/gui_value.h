/*
 * settings.h
 *
 *  Created on: 5.5.2015
 *      Author: horinek
 */

#ifndef GUI_VALUE_H_
#define GUI_VALUE_H_

#include "gui.h"

#define GUI_VAL_NUMBER			0
#define GUI_VAL_SYSTEM_TIME		1
#define GUI_VAL_SYSTEM_DATE		2
#define GUI_VAL_CONTRAST		3
#define GUI_VAL_BRIGTHNES		4
#define GUI_VAL_VARIO_TEST		5
#define GUI_VAL_VOLUME			6
#define GUI_VAL_NUMBER_DISABLE	7
#define GUI_VAL_TIME			8


typedef void float_cb(float val);

void gui_value_conf_P(const char * label, uint8_t type, const char * format, float start, float min, float max, float step, float_cb * cb, float mul = 1.0);

#define gui_value_init gui_dummy
#define gui_value_stop gui_dummy

void gui_value_loop();
void gui_value_irqh(uint8_t type, uint8_t * buff);


#endif /* SETTINGS_H_ */
