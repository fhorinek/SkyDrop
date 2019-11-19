/*
 * gui_waypoint_detail.h
 *
 *  Created on: Jun 23, 2018
 *      Author: tilmann@bubecks.de
 */

#ifndef GUI_waypoint_list_H_
#define GUI_waypoint_list_H_

#include "../gui.h"

#define gui_waypoint_list_stop gui_dummy
#define gui_waypoint_list_loop gui_list_draw
#define gui_waypoint_list_irqh gui_list_irqh

void gui_waypoint_list_init();
void gui_waypoint_list_action(uint8_t index);
void gui_waypoint_list_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);


#define GUI_WL_NORMAL		0
#define GUI_WL_OPEN			1

extern uint8_t gui_waypoint_list_mode;
extern uint8_t gui_waypoint_list_return;

#endif /* GUI_waypoint_list_H_ */
