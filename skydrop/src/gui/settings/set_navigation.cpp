/*
 * gui_home.cpp
 *
 *  Created on: 11.05.2017
 *      Author: tilmann@bubecks.de
 */

#include "set_navigation.h"

#include "../gui_filemanager.h"
#include "../gui_list.h"

#include "gui_waypoint_list.h"

#include "../../fc/conf.h"
#include "../../fc/waypoint.h"

#include "gui_task_editor.h"

void gui_home_init()
{
	gui_list_set(gui_home_item, gui_home_action, 5, GUI_SETTINGS);
}

void load_task()
{
	gui_filemanager_set_dir((char*) "/TASKS");
	gui_filemanager_level = 0;
	gui_filemanager_set_tasks(GUI_TASK_EDITOR, GUI_NAVIGATION);
	gui_switch_task(GUI_FILEMANAGER);
	gui_task_editor_mode = GUI_TE_OPEN;
}

void gui_home_action(uint8_t index)
{
	switch (index)
	{

	case (0):
		gui_switch_task(GUI_TASK_EDITOR);
		gui_task_editor_mode = GUI_TE_NEW;
		break;

	case (1):
		if (fc.task.active)
		{
			fc.task.active = false;
		}
		else
		{
			load_task();
		}
		break;

	case (2):
		if (fc.task.active)
		{
			gui_switch_task(GUI_TASK_EDITOR);
			gui_task_editor_mode = GUI_TE_EDIT;
		}
		else
		{
			load_task();
			gui_task_editor_mode = GUI_TE_OPEN_EDIT;
		}
		break;

	case (3):
		if (!(config.home.flags & HOME_TAKEOFF))
		{
			gui_switch_task(GUI_WAYPOINT_LIST);
			gui_waypoint_list_return = GUI_NAVIGATION;
		}
		break;

	case (4):
		config.home.flags ^= HOME_TAKEOFF;
		if (config.home.flags & HOME_TAKEOFF)
		{
			fc.flight.home_valid = false;

			config.home.flags = HOME_TAKEOFF;
			config.home.name[0] = 0;
		}
		
		ee_update_block((void*) &config.home, &config_ee.home, sizeof(config.home));

		break;
	}
}

void gui_home_item(uint8_t index, char *text, uint8_t *flags, char *sub_text)
{
	switch (index)
	{
	case 0:
		strcpy_P(text, PSTR("Create Task"));
		break;

	case 1:
		if (fc.task.active)
			strcpy_P(text, PSTR("Close Task"));
		else
			strcpy_P(text, PSTR("Load Task"));
		break;

	case 2:
		strcpy_P(text, PSTR("Edit Task"));
		*flags =  GUI_LIST_SUB_TEXT;
		if (fc.task.active)
			strcpy(sub_text, (char *)fc.task.name);
		else
			strcpy_P(sub_text, PSTR("<Load>"));

		break;

	case 3:
		strcpy_P(text, PSTR("Home position"));
		*flags =  GUI_LIST_SUB_TEXT;
		if (config.home.flags & HOME_TAKEOFF)
			strcpy_P(sub_text, PSTR("<Take off>"));
		else if(config.home.flags  & HOME_LOADED)
			strcpy(sub_text, (char *)config.home.name);
		else
			strcpy_P(sub_text, PSTR("<Load>"));
		break;

	case 4:
		strcpy_P(text, PSTR("Use take off"));
		if (config.home.flags & HOME_TAKEOFF)
			*flags =  GUI_LIST_CHECK_ON;
		else
			*flags =  GUI_LIST_CHECK_OFF;
		break;

	}

}

