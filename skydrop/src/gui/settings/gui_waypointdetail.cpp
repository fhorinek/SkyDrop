/*
 * gui_waypoint_detail.cpp
 *
 *  Created on: Jun 23, 2018
 *      Author: bubeck
 */

#include "gui_waypointdetail.h"

#include "gui_filemanager.h"
#include "../../fc/waypoint.h"


void gui_waypointdetail_init()
{
	char tmp[44];

	sprintf_P(tmp, PSTR("%s/%s"), gui_filemanager_path, gui_filemanager_name);
	set_waypoint_file(tmp);
	gui_switch_task(GUI_PAGES);
}

void gui_waypointdetail_stop() {}

void gui_waypointdetail_loop() {}

void gui_waypointdetail_irqh(uint8_t type, uint8_t * buff) {}

