/*
 * set_autoset.h
 *
 *  Created on: 25.5.2015
 *      Author: horinek
 */

#ifndef SET_AUTOSET_H_
#define SET_AUTOSET_H_

#include "../gui.h"

#define gui_set_autoset_stop gui_dummy
#define gui_set_autoset_loop gui_list_draw
#define gui_set_autoset_irqh gui_list_irqh

void gui_set_autoset_init();
void gui_set_autoset_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_set_autoset_action(uint8_t index);

#endif /* SET_autoset_H_ */
