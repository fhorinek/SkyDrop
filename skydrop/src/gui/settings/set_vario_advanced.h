/*
 * set_vario_advanced.h
 *
 *  Created on: 4.9.2015
 *      Author: horinek
 */

#ifndef SET_VARIO_ADVANCED_H_
#define SET_VARIO_ADVANCED_H_

#include "../gui.h"

#define gui_set_vario_advanced_stop gui_dummy
#define gui_set_vario_advanced_loop gui_list_draw
#define gui_set_vario_advanced_irqh gui_list_irqh

void gui_set_vario_advanced_init();
void gui_set_vario_advanced_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_set_vario_advanced_action(uint8_t index);

#endif /* SET_VARIO_ADVANCED_H_ */
