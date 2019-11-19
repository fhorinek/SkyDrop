/*
 * gui_waypoint_detail.h
 *
 *  Created on: Jun 23, 2018
 *      Author: tilmann@bubecks.de
 */

#ifndef GUI_TASK_EDITOR_H_
#define GUI_TASK_EDITOR_H_

#include "../gui.h"

#define gui_task_editor_stop gui_dummy
#define gui_task_editor_loop gui_list_draw
#define gui_task_editor_irqh gui_list_irqh

void gui_task_editor_init();

void gui_task_editor_action(uint8_t index);
void gui_task_editor_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);


#define GUI_TE_NEW			0
#define GUI_TE_OPEN			1
#define GUI_TE_EDIT			2
#define GUI_TE_OPEN_EDIT	3

extern uint8_t gui_task_editor_mode;

#endif /* GUI_TASK_EDITOR_H_ */
