#include "gui_waypoint_editor.h"

#include "../gui_list.h"
#include "../gui_value.h"
#include "../gui_filemanager.h"

#include "gui_waypoint_list.h"

uint8_t gui_waypoint_editor_wpt_index;
task_waypoint_t gui_waypoint_editor_wpt;

void gui_waypoint_editor_init()
{
	gui_list_set(gui_waypoint_editor_item, gui_waypoint_editor_action, 5, GUI_TASK_EDITOR);
	waypoint_task_get_wpt(gui_waypoint_editor_wpt_index, &gui_waypoint_editor_wpt);
}

void gui_waypoint_editor_radius_cb(float val)
{
	gui_switch_task(GUI_TASK_EDITOR);

	gui_waypoint_editor_wpt.radius_m = val;
	waypoint_task_modify_wpt(gui_waypoint_editor_wpt_index, &gui_waypoint_editor_wpt);

	waypoint_task_calc_distance();
}

void gui_waypoint_editor_action(uint8_t index)
{
	switch(index)
	{
		case(0):
			gui_waypoint_list_return = GUI_WAYPOINT_EDITOR;
			gui_switch_task(GUI_WAYPOINT_LIST);
		break;

		case(1):
			if (gui_waypoint_editor_wpt_index == 0)
				break;

			gui_value_conf_P(PSTR("Radius"), GUI_VAL_NUMBER, PSTR("%0.0f m"), gui_waypoint_editor_wpt.radius_m, 50, 30000, 50, gui_waypoint_editor_radius_cb);
			gui_switch_task(GUI_SET_VAL);
			break;

		case(2):
			waypoint_task_switch_wpt(gui_waypoint_editor_wpt_index, gui_waypoint_editor_wpt_index - 1);
			gui_switch_task(GUI_TASK_EDITOR);
			waypoint_task_calc_distance();
			break;

		case(3):
			waypoint_task_switch_wpt(gui_waypoint_editor_wpt_index, gui_waypoint_editor_wpt_index + 1);
			gui_switch_task(GUI_TASK_EDITOR);
			waypoint_task_calc_distance();
			break;

		case(4):
			waypoint_task_remove_wpt(gui_waypoint_editor_wpt_index);
			gui_switch_task(GUI_TASK_EDITOR);
			waypoint_task_calc_distance();
			break;
	}
}

void gui_waypoint_editor_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch(index)
	{
		case(0):
			sprintf_P(text, PSTR("Waypoint #%u"), gui_waypoint_editor_wpt_index + 1);
			sprintf_P(sub_text, PSTR("%s"), gui_waypoint_editor_wpt.wpt.name);
			*flags = GUI_LIST_SUB_TEXT;
			break;

		case(1):
			strcpy_P(text, PSTR("Radius"));
			*flags =  GUI_LIST_SUB_TEXT;

			if (gui_waypoint_editor_wpt_index == 0)
				strcpy_P(sub_text, PSTR("Take-off"));
			else
				sprintf_P(sub_text, PSTR("%um"), gui_waypoint_editor_wpt.radius_m);
			break;
		case(2):
			strcpy_P(text, PSTR("Move up"));
			break;

		case(3):
			strcpy_P(text, PSTR("Move down"));
			break;

		case(4):
			strcpy_P(text, PSTR("Remove"));
			break;
	}
}
