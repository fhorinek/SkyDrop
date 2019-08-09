/*
 * settings.h
 *
 *  Created on: 5.5.2015
 *      Author: horinek
 */

#ifndef SET_SYSTEM_H_
#define SET_SYSTEM_H_

#include "../gui.h"

void gui_set_system_init();
void gui_set_system_stop();
void gui_set_system_loop();
void gui_set_system_irqh(uint8_t type, uint8_t * buff);
void gui_set_system_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_set_system_action(uint8_t index);

#endif /* SET_SYSTEM_H_ */
