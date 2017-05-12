/*
 * gui_home.cpp
 *
 *  Created on: 11.05.2017
 *      Author: tilmann@bubecks.de
 */

#include "gui_home.h"

#include "gui_flightlog.h"

#include "common.h"
#include "../gui_list.h"
#include "../gui_dialog.h"
#include "../../fc/conf.h"
#include "../../fc/logger/logger.h"

void gui_home_init()
{
	gui_list_set(gui_home_item, gui_home_action, 2, GUI_SETTINGS);
}

void gui_home_stop() {}

void gui_home_loop()
{
	gui_list_draw();
}

void gui_home_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_home_action(uint8_t index) {

	switch(index)
	{
	case(0):
		fc.flight.home_set_on_autostart = !fc.flight.home_set_on_autostart;
		if ( fc.flight.home_set_on_autostart ) {
			fc.flight.home_valid = false;
			fc.flight.home.name[0] = 0;
			fc.flight.home.freq[0] = 0;
			fc.flight.home.rwy[0] = 0;
			fc.flight.home.traffic_pattern[0] = 0;
			fc.flight.home.info[0] = 0;
		}
	break;

	case(1):
		if ( !fc.flight.home_set_on_autostart ) {
			gui_flightlog_set_dir("/HOMES");
			gui_flightlog_level = 0;
			gui_flightlog_set_file_task(GUI_HOMEDETAIL);
			gui_switch_task(GUI_FLIGHTLOG);
		}
		break;
	}
}

void gui_home_item(uint8_t idx, char * text, uint8_t * flags, char * sub_text)
{
	switch (idx)
	{
		case 0:
			strcpy_P(text, PSTR("on Takeoff"));
			if (fc.flight.home_set_on_autostart)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;
		break;

		case 1:
			strcpy_P(text, PSTR("Set home from SD"));
			strcpy(sub_text, (const char *)fc.flight.home.name);
			*flags |= GUI_LIST_SUB_TEXT;
			if (fc.flight.home_set_on_autostart)
				*flags |= GUI_LIST_DISABLED;
		break;
	}

}

