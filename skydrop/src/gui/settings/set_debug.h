/*
 * set_debug.h
 *
 *  Created on: 5.5.2015
 *      Author: horinek
 */

#ifndef set_debug_H_
#define set_debug_H_

#include "../gui.h"

#define gui_set_debug_stop gui_dummy
#define gui_set_debug_loop gui_list_draw
#define gui_set_debug_irqh gui_list_irqh


#ifndef DISABLE_DEBUG
void gui_set_debug_init();
void gui_set_debug_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_set_debug_action(uint8_t index);
#else
#define gui_set_debug_init gui_dummy
#endif

#endif /* set_debug_H_ */
