/*
 * layouts.h
 *
 *  Created on: 5.5.2015
 *      Author: horinek
 */

#ifndef GUI_LAYOUTS_H_
#define GUI_LAYOUTS_H_

#include "../gui.h"


#define gui_layouts_stop gui_dummy
#define gui_layouts_loop gui_list_draw
#define gui_layouts_irqh gui_list_irqh

void gui_layouts_init();
void gui_layouts_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_layouts_action(uint8_t index);

#endif /* layouts_H_ */
