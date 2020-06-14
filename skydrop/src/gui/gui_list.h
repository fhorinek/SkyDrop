/*
 * gui_list.h
 *
 *  Created on: 5.5.2015
 *      Author: horinek
 */

#ifndef GUI_LIST_H_
#define GUI_LIST_H_

#include "../common.h"

typedef void gui_list_gen(uint8_t index, char * text, uint8_t * flags, char * sub_text);
typedef void gui_list_act(uint8_t index);

void gui_list_draw();
void gui_list_set(gui_list_gen * f_ptr, gui_list_act * f_act, uint8_t size, uint8_t back);
void gui_list_set_index(uint8_t task, uint8_t index);
uint8_t gui_list_get_index(uint8_t task);
void gui_list_moveup();
void gui_list_movedown();
void gui_list_action();
void gui_list_irqh(uint8_t type, uint8_t * buff);

#define GUI_LIST_T_MASK		0b00111111

#define GUI_LIST_ENTRY			0     // Normal entry
#define GUI_LIST_BACK			1     // The "back" entry
#define GUI_LIST_CHECK_ON		2     // An entry with a checkbox, which is currently ON
#define GUI_LIST_CHECK_OFF		3     // An entry with a checkbox, which is currently OFF
#define GUI_LIST_SUB_TEXT		4     // An entry containing a text under it.
#define GUI_LIST_TITLE			5     // Small text used as title in file manager
#define GUI_LIST_SUB_TEXT_SMALL	6

#define GUI_LIST_NOT_VISIBLE	0b10000000 //passed to the function just to inform that this line is not visible

#endif /* GUI_LIST_H_ */
