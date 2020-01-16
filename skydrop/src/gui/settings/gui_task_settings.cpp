#include "gui_task_settings.h"
#include "../../fc/waypoint.h"

#include "../gui_list.h"
#include "../gui_text.h"
#include "../gui_filemanager.h"

#include "gui_waypoint_editor.h"
#include "gui_waypoint_list.h"

void gui_task_settings_init()
{
	gui_list_set(gui_task_settings_item, gui_task_settings_action, 5, GUI_TASK_EDITOR);
}

void gui_task_settings_action(uint8_t index)
{
	switch(index)
	{
		case(0):
			fc.task.head.flags ^= CFG_TASK_FLAGS_START_EXIT;
			waypoint_task_modify_head((task_header_t*)&fc.task.head);
			break;

		case(1):
//			fc.task.head.flags ^= CFG_TASK_FLAGS_GOAL_LINE;
//			waypoint_task_modify_head((task_header_t*)&fc.task.head);
			break;

		case(2):
//			strcpy_P(text, PSTR("Start"));
			break;


		case(3):
//			strcpy_P(text, PSTR("Deadline"));
			break;

		case(4):
			fc.task.head.flags ^= CFG_TASK_FLAGS_FAI_SPHERE;
			waypoint_task_modify_head((task_header_t*)&fc.task.head);

			waypoint_task_calc();
			break;
	}
}

void gui_task_settings_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch(index)
	{
		case(0):
			strcpy_P(text, PSTR("SSS Crossing"));
			if (fc.task.head.flags & CFG_TASK_FLAGS_START_EXIT)
				strcpy_P(sub_text, PSTR("Exit"));
			else
				strcpy_P(sub_text, PSTR("Enter"));
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
			if (fc.task.head.start_hour == !CFG_TASK_HOUR_DISABLED)
				sprintf_P(sub_text, PSTR("%02u:%02u"), fc.task.head.start_hour, fc.task.head.start_min);
			else
				strcpy_P(sub_text, PSTR("--:--"));
			break;


		case(3):
			strcpy_P(text, PSTR("Deadline"));
			*flags =  GUI_LIST_SUB_TEXT;
			if (fc.task.head.deadline_hour == !CFG_TASK_HOUR_DISABLED)
				sprintf_P(sub_text, PSTR("%02u:%02u"), fc.task.head.deadline_hour, fc.task.head.deadline_min);
			else
				strcpy_P(sub_text, PSTR("--:--"));
			break;

		case(4):
			strcpy_P(text, PSTR("Earth model"));
			*flags =  GUI_LIST_SUB_TEXT;
			if (fc.task.head.flags & CFG_TASK_FLAGS_FAI_SPHERE)
				strcpy_P(sub_text, PSTR("FAI Sphere"));
			else
				strcpy_P(sub_text, PSTR("WGS-84"));
			break;
	}
}
