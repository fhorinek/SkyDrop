/*
 * settings.h
 *
 *  Created on: 5.5.2015
 *      Author: horinek
 */

#ifndef SET_DISPLAY_H_
#define SET_DISPLAY_H_

#include "../gui.h"

#define gui_set_display_stop gui_dummy
#define gui_set_display_loop gui_list_draw
#define gui_set_display_irqh gui_list_irqh

void gui_set_display_init();
void gui_set_display_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_set_display_action(uint8_t index);

#endif /* SETTINGS_H_ */
