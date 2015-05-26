/*
 * settings.h
 *
 *  Created on: 5.5.2015
 *      Author: horinek
 */

#ifndef GUI_VALUE_H_
#define GUI_VALUE_H_

#include "gui.h"

#define GUI_VAL_NUMBER	0
#define GUI_VAL_VOLUME	0

typedef void void_cb(float val);

void gui_value_conf(char * label, uint8_t type, char * format, float start, float min, float max, float step, void_cb * cb);

void gui_value_init();
void gui_value_stop();
void gui_value_loop();
void gui_value_irqh(uint8_t type, uint8_t * buff);


#endif /* SETTINGS_H_ */
