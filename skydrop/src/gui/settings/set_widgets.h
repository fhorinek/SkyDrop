/*
 * set_widgets.h
 *
 *  Created on: 25.5.2015
 *      Author: horinek
 */

#ifndef SET_WIDGETS_H_
#define SET_WIDGETS_H_

#include "../gui.h"

#define gui_set_widgets_stop gui_dummy

void gui_set_widgets_init();
void gui_set_widgets_loop();
void gui_set_widgets_irqh(uint8_t type, uint8_t * buff);
void gui_set_widgets_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_set_widgets_action(uint8_t index);

#endif /* SET_WIDGETS_H_ */
