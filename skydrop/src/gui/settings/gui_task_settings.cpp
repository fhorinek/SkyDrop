#include "gui_task_settings.h"
#include "../../fc/waypoint.h"

#include "../gui_list.h"
//##include "../gui_text.h"
//#include "../gui_filemanager.h"
#include "../gui_value.h"

#include "gui_waypoint_editor.h"
#include "gui_waypoint_list.h"

void gui_task_settings_init()
{
	gui_list_set(gui_task_settings_item, gui_task_settings_action, 6, GUI_TASK_EDITOR);
}

void gui_task_settings_start_cb(float val)
{
	fc.task.head.start_hour = (0xFF00 & (uint16_t)val) >> 8;
	fc.task.head.start_min = (0x00FF & (uint16_t)val) >> 0;

	waypoint_task_modify_head((task_header_t*)&fc.task.head);
	gui_switch_task(GUI_TASK_SETTINGS);
}

void gui_task_settings_deadline_cb(float val)
{
	fc.task.head.deadline_hour = (0xFF00 & (uint16_t)val) >> 8;
	fc.task.head.deadline_min = (0x00FF & (uint16_t)val) >> 0;

	waypoint_task_modify_head((task_header_t*)&fc.task.head);
	gui_switch_task(GUI_TASK_SETTINGS);
}


void gui_task_settings_action(uint8_t index)
{
	switch(index)
	{
		case(0):
			fc.task.head.flags ^= CFG_TASK_FLAGS_START_ENTER;
			waypoint_task_modify_head((task_header_t*)&fc.task.head);
			break;

		case(1):
			fc.task.head.flags ^= CFG_TASK_FLAGS_GOAL_LINE;
			waypoint_task_modify_head((task_header_t*)&fc.task.head);
			break;

		case(2):
			if (fc.task.head.start_hour & CFG_TASK_HOUR_DISABLED)
			{
				fc.task.head.start_hour &= ~CFG_TASK_HOUR_DISABLED;
				gui_value_conf_P(PSTR("Start time"), GUI_VAL_TIME, PSTR(""), (fc.task.head.start_hour << 8) | fc.task.head.start_min, 0, 0, 1, gui_task_settings_start_cb);
				gui_switch_task(GUI_SET_VAL);
			}
			else
			{
				fc.task.head.start_hour |= CFG_TASK_HOUR_DISABLED;
				waypoint_task_modify_head((task_header_t*)&fc.task.head);
			}
			break;


		case(3):
			if (fc.task.head.deadline_hour & CFG_TASK_HOUR_DISABLED)
			{
				fc.task.head.deadline_hour &= ~CFG_TASK_HOUR_DISABLED;
				gui_value_conf_P(PSTR("Deadline"), GUI_VAL_TIME, PSTR(""), (fc.task.head.deadline_hour << 8) | fc.task.head.deadline_min, 0, 0, 1, gui_task_settings_deadline_cb);
				gui_switch_task(GUI_SET_VAL);
			}
			else
			{
				fc.task.head.deadline_hour |= CFG_TASK_HOUR_DISABLED;
				waypoint_task_modify_head((task_header_t*)&fc.task.head);
			}
			break;

		case(4):
			fc.task.head.flags ^= CFG_TASK_FLAGS_FAI_SPHERE;
			waypoint_task_modify_head((task_header_t*)&fc.task.head);

			waypoint_task_calc_distance();
			break;

		case(5):
			fc.task.head.flags ^= CFG_TASK_FLAGS_OPTIMIZE;
			waypoint_task_modify_head((task_header_t*)&fc.task.head);

			waypoint_task_optimise_reset();
			break;
	}
}

void gui_task_settings_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch(index)
	{
		case(0):
			strcpy_P(text, PSTR("SSS Crossing"));
			if (fc.task.head.flags & CFG_TASK_FLAGS_START_ENTER)
				strcpy_P(sub_text, PSTR("Enter"));
			else
				strcpy_P(sub_text, PSTR("Exit"));
			*flags = GUI_LIST_SUB_TEXT;
			break;

		case(1):
			strcpy_P(text, PSTR("Goal"));
			if (fc.task.head.flags & CFG_TASK_FLAGS_GOAL_LINE)
				strcpy_P(sub_text, PSTR("Line"));
			else
				strcpy_P(sub_text, PSTR("Cylinder"));
			*flags = GUI_LIST_SUB_TEXT;
			break;

		case(2):
			strcpy_P(text, PSTR("Start"));
			*flags =  GUI_LIST_SUB_TEXT;
			if (fc.task.head.start_hour & CFG_TASK_HOUR_DISABLED)
				strcpy_P(sub_text, PSTR("--:--"));
			else
				sprintf_P(sub_text, PSTR("%02u:%02u"), fc.task.head.start_hour, fc.task.head.start_min);
			break;


		case(3):
			strcpy_P(text, PSTR("Deadline"));
			*flags =  GUI_LIST_SUB_TEXT;
			if (fc.task.head.deadline_hour & CFG_TASK_HOUR_DISABLED)
				strcpy_P(sub_text, PSTR("--:--"));
			else
				sprintf_P(sub_text, PSTR("%02u:%02u"), fc.task.head.deadline_hour, fc.task.head.deadline_min);
			break;

		case(4):
			strcpy_P(text, PSTR("Earth model"));
			*flags =  GUI_LIST_SUB_TEXT;
			if (fc.task.head.flags & CFG_TASK_FLAGS_FAI_SPHERE)
				strcpy_P(sub_text, PSTR("FAI Sphere"));
			else
				strcpy_P(sub_text, PSTR("WGS-84"));
			break;

		case(5):
			strcpy_P(text, PSTR("Optimise"));
			if (fc.task.head.flags & CFG_TASK_FLAGS_OPTIMIZE)
				*flags =  GUI_LIST_CHECK_ON;
			else
				*flags =  GUI_LIST_CHECK_OFF;
			break;
	}
}
