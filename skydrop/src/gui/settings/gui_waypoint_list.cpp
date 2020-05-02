#include "gui_waypoint_list.h"
#include "../../fc/waypoint.h"

#include "../gui_list.h"
#include "../gui_text.h"
#include "../gui_filemanager.h"

#include "gui_waypoint_editor.h"

uint8_t gui_waypoint_list_mode = GUI_WL_NORMAL;
uint8_t gui_waypoint_list_return;

void gui_waypoint_list_browse()
{
	gui_filemanager_set_dir((char*) "/WPT");
	gui_filemanager_level = 0;
	gui_filemanager_set_tasks(GUI_WAYPOINT_LIST, gui_waypoint_list_return);
	gui_switch_task(GUI_FILEMANAGER);
	gui_waypoint_list_mode = GUI_WL_OPEN;
}

void gui_waypoint_list_init()
{
	if (gui_waypoint_list_mode == GUI_WL_OPEN)
	{
		waypoint_list_open(gui_filemanager_name);

		if (waypoint_list_count == 0)
		{
			gui_showmessage_P(PSTR("No waypoints\nfound!"));
			waypoint_list_close();
		}
	}

	if (waypoint_list_name[0])
	{
		gui_list_set(gui_waypoint_list_item, gui_waypoint_list_action, waypoint_list_count + 1, GUI_TASK_EDITOR);
		gui_waypoint_list_mode = GUI_WL_NORMAL;
	}
	else
	{
		gui_waypoint_list_browse();
	}
}

void gui_waypoint_list_action(uint8_t index)
{
	if (index == 0)
	{
		gui_waypoint_list_browse();
	}
	else
	{
		waypoint_cache_t wpt;
		waypoint_list_get_wpt(index - 1, &wpt);

		//add new waypoint to the task
		if (gui_waypoint_list_return == GUI_TASK_EDITOR)
		{
			//if there is no wp add first as Take-off
			if (fc.task.waypoint_count == 0)
				waypoint_task_add_wpt(&wpt);

			waypoint_task_add_wpt(&wpt);

			waypoint_task_calc_distance();
		}

		//change active waypoint
		if (gui_waypoint_list_return == GUI_WAYPOINT_EDITOR)
		{
			memcpy(&gui_waypoint_editor_wpt.wpt, &wpt, sizeof(wpt));
			waypoint_task_modify_wpt(gui_waypoint_editor_wpt_index, &gui_waypoint_editor_wpt);
			waypoint_task_calc_distance();
		}

		//set new home position
		if (gui_waypoint_list_return == GUI_NAVIGATION)
		{
			memcpy((char *)config.home.name, wpt.name, sizeof(wpt.name));
			config.home.lat = wpt.latitude;
			config.home.lon = wpt.longtitude;
			config.home.flags = HOME_LOADED;
			fc.flight.home_valid = true;

			
			ee_update_block((void*) &config.home, &config_ee.home, sizeof(config.home));
		}

		gui_switch_task(gui_waypoint_list_return);
	}
}

void gui_waypoint_list_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	if (index == 0)
	{
		strcpy(text, waypoint_list_name);
		*flags =  GUI_LIST_TITLE;
	}
	else
	{
		if (*flags & GUI_LIST_NOT_VISIBLE)
			return;

		waypoint_cache_t wpt;

		waypoint_list_get_wpt(index - 1, &wpt);
		strcpy(text, wpt.name);
	}

}
