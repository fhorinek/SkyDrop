/*
 * gui_waypoint_detail.h
 *
 *  Created on: Jun 23, 2018
 *      Author: tilmann@bubecks.de
 */

#ifndef GUI_waypoint_editor_H_
#define GUI_waypoint_editor_H_

#include "../gui.h"

#define gui_waypoint_editor_stop gui_dummy
#define gui_waypoint_editor_loop gui_list_draw
#define gui_waypoint_editor_irqh gui_list_irqh

void gui_waypoint_editor_init();
void gui_waypoint_editor_action(uint8_t index);
void gui_waypoint_editor_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);

#include "../../fc/waypoint.h"

extern uint8_t gui_waypoint_editor_wpt_index;
extern task_waypoint_t gui_waypoint_editor_wpt;


#endif /* GUI_waypoint_editor_H_ */
