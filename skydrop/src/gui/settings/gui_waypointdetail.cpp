/*
 * gui_waypoint_detail.cpp
 *
 *  Created on: Jun 23, 2018
 *      Author: bubeck
 */

#include <gui/settings/gui_filemanager.h>
#include "gui_waypointdetail.h"

#include "common.h"
#include "../gui_list.h"
#include "../gui_dialog.h"
#include "../../fc/waypoint.h"
#include "../../fc/conf.h"
#include "../../fc/logger/logger.h"

void gui_waypointdetail_init()
{
	char tmp[44];

	sprintf_P(tmp, PSTR("%s/%s"), gui_filemanager_path, gui_filemanager_name);
	set_waypoint_file(tmp);

	gui_list_set(gui_waypointdetail_item, gui_waypointdetail_action, 5, GUI_FILEMANAGER);
}

void gui_waypointdetail_stop() {}

void gui_waypointdetail_loop()
{
	gui_list_draw();
}

void gui_waypointdetail_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_waypointdetail_action(uint8_t index)
{
	if (index == 0)
	{
		gui_showmessage_P(PSTR("Waypoint\nselected."));
		gui_switch_task(GUI_PAGES);

#if 0
		memcpy((void *)&config.home, (void *)&home_new, sizeof(cfg_home));

		config.home.flags = HOME_LOADED_FROM_SD;
		eeprom_busy_wait();
		eeprom_update_block((void *)&config.home, &config_ee.home, sizeof(config.home));
#endif
	}
}

void gui_waypointdetail_item(uint8_t idx, char * text, uint8_t * flags, char * sub_text)
{
	switch (idx)
	{
		case 0:
			strcpy_P(text, PSTR("Use this Waypoint"));
		break;

		case 1:
			strcpy_P(text, PSTR("Name:"));
			strcpy(sub_text, (const char *)fc.flight.next_waypoint.name);
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case 2:
			strcpy_P(text, PSTR("Lat:"));
			sprintf_P(sub_text, PSTR("%2.6f"), fc.flight.next_waypoint.lat / 10000000.0);
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case 3:
			strcpy_P(text, PSTR("Lon:"));
			sprintf_P(sub_text, PSTR("%2.6f"), fc.flight.next_waypoint.lon / 10000000.0);
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case 4:
			strcpy_P(text, PSTR("Radius:"));
			sprintf_P(sub_text, PSTR("%dm"), fc.flight.next_waypoint.radius_m);
			*flags |= GUI_LIST_SUB_TEXT;
		break;

	}

}

