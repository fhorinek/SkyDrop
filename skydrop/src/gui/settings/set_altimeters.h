/*
 * settings.h
 *
 *  Created on: 5.5.2015
 *      Author: horinek
 */

#ifndef SET_ALTIMETERS_H_
#define SET_ALTIMETERS_H_

#include "../gui.h"

void gui_set_altimeters_init();
void gui_set_altimeters_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_set_altimeters_action(uint8_t index);

#endif /* SET_ALTIMETERS_H_ */
