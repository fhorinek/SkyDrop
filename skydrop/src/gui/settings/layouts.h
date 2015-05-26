/*
 * layouts.h
 *
 *  Created on: 5.5.2015
 *      Author: horinek
 */

#ifndef GUI_LAYOUTS_H_
#define GUI_LAYOUTS_H_

#include "../gui.h"

void gui_layouts_init();
void gui_layouts_stop();
void gui_layouts_loop();
void gui_layouts_irqh(uint8_t type, uint8_t * buff);
void gui_layouts_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_layouts_action(uint8_t index);

#endif /* layouts_H_ */
