/*
 * gui_waypoint_detail.h
 *
 *  Created on: Jun 23, 2018
 *      Author: tilmann@bubecks.de
 */

#ifndef GUI_SETTINGS_GUI_WAYPOINTDETAIL_H_
#define GUI_SETTINGS_GUI_WAYPOINTDETAIL_H_

#include "../gui.h"
#include "../../fc/fc.h"

void gui_waypointdetail_init();
void gui_waypointdetail_stop();
void gui_waypointdetail_loop();
void gui_waypointdetail_irqh(uint8_t type, uint8_t * buff);
void gui_waypointdetail_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_waypointdetail_action(uint8_t index);

void read_waypointfile(const char *filename, cfg_home * home);

#endif /* GUI_SETTINGS_GUI_WAYPOINTDETAIL_H_ */
