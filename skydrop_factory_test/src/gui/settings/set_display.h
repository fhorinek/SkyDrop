/*
 * settings.h
 *
 *  Created on: 5.5.2015
 *      Author: horinek
 */

#ifndef SET_DISPLAY_H_
#define SET_DISPLAY_H_

#include "../gui.h"

void gui_set_display_init();
void gui_set_display_stop();
void gui_set_display_loop();
void gui_set_display_irqh(uint8_t type, uint8_t * buff);
void gui_set_display_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_set_display_action(uint8_t index);

#endif /* SETTINGS_H_ */
