/*
 * set_layout.h
 *
 *  Created on: 25.5.2015
 *      Author: horinek
 */

#ifndef SET_LAYOUT_H_
#define SET_LAYOUT_H_

#include "../gui.h"

#define gui_set_layout_stop gui_dummy

void gui_set_layout_init();
void gui_set_layout_loop();
void gui_set_layout_irqh(uint8_t type, uint8_t * buff);
void gui_set_layout_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_set_layout_action(uint8_t index);

#endif /* SET_layout_H_ */
