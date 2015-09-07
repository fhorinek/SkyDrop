/*
 * set_weaklift.h
 *
 *  Created on: 4.9.2015
 *      Author: horinek
 */

#ifndef SET_WEAKLIFT_H_
#define SET_WEAKLIFT_H_

#include "../gui.h"

void gui_set_weaklift_init();
void gui_set_weaklift_stop();
void gui_set_weaklift_loop();
void gui_set_weaklift_irqh(uint8_t type, uint8_t * buff);
void gui_set_weaklift_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_set_weaklift_action(uint8_t index);

#endif /* SET_WEAKLIFT_H_ */
