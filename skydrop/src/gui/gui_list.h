/*
 * gui_list.h
 *
 *  Created on: 5.5.2015
 *      Author: horinek
 */

#ifndef GUI_LIST_H_
#define GUI_LIST_H_

#include "../common.h"

typedef void gui_list_gen(uint8_t index, char * text, uint8_t * disabled);

void gui_list_draw();
void gui_list_set(gui_list_gen * f_ptr, uint8_t size);
void gui_list_moveup();
void gui_list_movedown();


#endif /* GUI_LIST_H_ */
