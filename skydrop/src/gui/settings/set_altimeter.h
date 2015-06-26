/*
 * settings.h
 *
 *  Created on: 5.5.2015
 *      Author: horinek
 */

#ifndef SET_ALTIMETER_H_
#define SET_ALTIMETER_H_

#include "../gui.h"

void gui_set_altimeter_init();
void gui_set_altimeter_stop();
void gui_set_altimeter_loop();
void gui_set_altimeter_irqh(uint8_t type, uint8_t * buff);
void gui_set_altimeter_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_set_altimeter_action(uint8_t index);

void gui_set_altimeter_index(uint8_t index);

#endif /* SET_ALTIMETER_H_ */
