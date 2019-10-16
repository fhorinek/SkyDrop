/*
 * settings.h
 *
 *  Created on: 5.5.2015
 *      Author: horinek
 */

#ifndef SET_VARIO_H_
#define SET_VARIO_H_

#include "../gui.h"

#define gui_set_vario_stop gui_dummy
#define gui_set_vario_loop gui_list_draw
#define gui_set_vario_irqh gui_list_irqh

void gui_set_vario_init();
void gui_set_vario_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_set_vario_action(uint8_t index);

#endif /* SETTINGS_H_ */
