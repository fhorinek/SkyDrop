/*
 * set_debug.h
 *
 *  Created on: 5.5.2015
 *      Author: horinek
 */

#ifndef set_debug_H_
#define set_debug_H_

#include "../gui.h"

void gui_set_debug_init();
void gui_set_debug_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_set_debug_action(uint8_t index);

#endif /* set_debug_H_ */
