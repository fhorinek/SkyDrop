/*
 * settings.h
 *
 *  Created on: 5.5.2015
 *      Author: horinek
 */

#ifndef SET_BLUETOOTH_H_
#define SET_BLUETOOTH_H_

#include "../gui.h"

void gui_set_bluetooth_init();
void gui_set_bluetooth_stop();
void gui_set_bluetooth_loop();
void gui_set_bluetooth_irqh(uint8_t type, uint8_t * buff);
void gui_set_bluetooth_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_set_bluetooth_action(uint8_t index);

#endif /* SET_BLUETOOTH_H_ */
