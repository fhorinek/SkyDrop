#include "gui_task_editor.h"
#include "../../fc/waypoint.h"

#include "../gui_list.h"
#include "../gui_text.h"
#include "../gui_filemanager.h"

#include "gui_waypoint_editor.h"
#include "gui_waypoint_list.h"


uint8_t gui_task_editor_mode = GUI_TE_EDIT;

void gui_task_editor_refresh()
{
	gui_list_set(gui_task_editor_item, gui_task_editor_action,7 + fc.task.waypoint_count, GUI_NAVIGATION);
}

void gui_task_editor_init()
{
	if (gui_task_editor_mode == GUI_TE_NEW)
	{
		waypoint_task_open((char *)"");
	}

	if (gui_task_editor_mode == GUI_TE_OPEN)
	{
		waypoint_task_open(gui_filemanager_name);
		gui_switch_task(GUI_NAVIGATION);
	}

	if (gui_task_editor_mode == GUI_TE_OPEN_EDIT)
	{
		waypoint_task_open(gui_filemanager_name);
	}

	if (gui_task_editor_mode == GUI_TE_EDIT)
	{
		waypoint_task_open((char *)fc.task.name);
	}

	gui_task_editor_mode = GUI_TE_EDIT;
	gui_task_editor_refresh();
}

void gui_task_editor_name_cb(uint8_t ret, char * buff)
{
	if (ret == GUI_TEXT_OK && strcmp(buff, (char *)fc.task.name) != 0)
	{
		char line[32];

		const char * task_root = PSTR("/TASKS");
		sprintf_P(line, PSTR("%S/%s"), task_root, buff);
		if (storage_file_exist(line))
		{
			sprintf_P(line, PSTR("Task name\n%s\nexists!"), buff);
			gui_showmessage(line);
		}
		else
		{
			char line2[32];
			sprintf_P(line, PSTR("%S/%s"), task_root, (char *)fc.task.name);
			sprintf_P(line2, PSTR("%S/%s"), task_root, buff);

			f_rename(line, line2);
			strcpy((char *)fc.task.name, buff);
		}
	}

	gui_task_editor_mode = GUI_TE_EDIT;
	gui_switch_task(GUI_TASK_EDITOR);
}

void gui_task_editor_delete_cb(uint8_t ret)
{
	if (ret == GUI_DIALOG_FORMAT)
	{
		waypoint_task_remove();
		gui_switch_task(GUI_NAVIGATION);
	}
	else
	{
		gui_switch_task(GUI_TASK_EDITOR);
	}
}

void gui_task_editor_action(uint8_t index)
{
	if (index == 0)
	{
		gui_text_conf((char *)fc.task.name, sizeof(fc.task.name) - 1, gui_task_editor_name_cb);
		gui_switch_task(GUI_TEXT);
	}

	if (index == 1)
	{
		gui_switch_task(GUI_TASK_SETTINGS);
	}

	if (index > 2 && (index < fc.task.waypoint_count + 3))
	{
		uint8_t windex = index - 3;

		gui_waypoint_editor_wpt_index = windex;
		gui_switch_task(GUI_WAYPOINT_EDITOR);
	}

	if (index == fc.task.waypoint_count + 3 || index == fc.task.waypoint_count + 4)
	{
		if (fc.task.waypoint_count < 3)
			return;

		const char * ctype = (fc.task.head.flags & CFG_TASK_FLAGS_FAI_SPHERE) ? PSTR("FAI") : PSTR("WGS");
		char text[64];

		uint8_t last_step = 0xFF;
		while (fc.task.opti_step < 6)
		{
			if (last_step != fc.task.opti_step)
			{
				last_step = fc.task.opti_step;


				sprintf_P(text, PSTR("Distance (%S)\nOptimal:%d%%\nCenter:%0.1fkm"), ctype, fc.task.opti_step * 20, fc.task.head.center_dist_m / 1000.0);

				gui_showmessage(text);
				gui_forcemessage();
				gui_force_loop();
			}

			waypoint_task_optimise_step();
			ewdt_reset();
		}

		sprintf_P(text, PSTR("Distance (%S)\nOptimal:%0.1fkm\nCenter:%0.1fkm"), ctype, fc.task.head.opti_dist_m / 1000.0, fc.task.head.center_dist_m / 1000.0);

		gui_showmessage(text);
		gui_forcemessage();
	}

	if (index == fc.task.waypoint_count + 5)
	{
		gui_switch_task(GUI_WAYPOINT_LIST);
		gui_waypoint_list_return = GUI_TASK_EDITOR;
	}

	if (index == fc.task.waypoint_count + 6)
	{
		gui_dialog_set_P(PSTR("Warning"), PSTR("\nDelete this task?"), GUI_STYLE_FORMAT, gui_task_editor_delete_cb);
		gui_switch_task(GUI_DIALOG);
	}
}


void gui_task_editor_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	if (index == 0)
	{
		strcpy_P(text, PSTR("Task name"));
		*flags =  GUI_LIST_SUB_TEXT;
		strcpy(sub_text, (char *)fc.task.name);
	}

	if (index == 1)
	{
		strcpy_P(text, PSTR("Settings"));
	}

	if (index == 2)
	{
		strcpy_P(text, PSTR("Waypoints"));
		*flags =  GUI_LIST_TITLE;
	}

	if (index > 2 && (index < fc.task.waypoint_count + 3))
	{
		task_waypoint_t twpt;
		uint8_t windex = index - 3;

		*flags |= GUI_LIST_SUB_TEXT_SMALL;
		if (*flags & GUI_LIST_NOT_VISIBLE)
			return;

		waypoint_task_get_wpt(windex, &twpt);

		sprintf_P(text, PSTR("#%u:%s"), windex + 1, twpt.wpt.name);
		if (windex == 0)
		{
			strcpy_P(sub_text, PSTR("Take-off"));
		}
		else
		{
			if (windex == 1)
				sprintf_P(sub_text, PSTR("SSS D:%0.2fkm R:%0.1fkm"), twpt.opti_dist_m / 1000.0, twpt.radius_m / 1000.0);
			else if (windex == fc.task.waypoint_count - 2 || (windex == 2 && fc.task.waypoint_count == 3))
				sprintf_P(sub_text, PSTR("ESS D:%0.2fkm R:%0.1fkm"), twpt.opti_dist_m / 1000.0, twpt.radius_m / 1000.0);
			else if (windex == fc.task.waypoint_count - 1)
			{
				//if goal is line, distance is longer by radius
				if (fc.task.head.flags & CFG_TASK_FLAGS_GOAL_LINE)
					sprintf_P(sub_text, PSTR("Goal D:%0.2fkm L:%0.1fkm"), twpt.opti_dist_m / 1000.0, twpt.radius_m / 500.0);
				else
					sprintf_P(sub_text, PSTR("Goal D:%0.2fkm R:%0.1fkm"), (twpt.opti_dist_m - twpt.radius_m ) / 1000.0, twpt.radius_m / 1000.0);
			}
			else
				sprintf_P(sub_text, PSTR("D:%0.2fkm R:%0.1fkm"), twpt.opti_dist_m / 1000.0, twpt.radius_m / 1000.0);
		}
	}

	if (index == fc.task.waypoint_count + 3)
	{
		sprintf_P(text, PSTR("Center:%0.2fkm"), fc.task.head.center_dist_m / 1000.0);
	}

	if (index == fc.task.waypoint_count + 4)
	{
		if (fc.task.head.opti_dist_m == 0 && fc.task.head.center_dist_m > 0 )
			sprintf_P(text, PSTR("Short: %u%%"), fc.task.opti_step * 20);
		else
			sprintf_P(text, PSTR("Short:%0.2fkm"), fc.task.head.opti_dist_m / 1000.0);
	}

	if (index == fc.task.waypoint_count + 5)
	{
		strcpy_P(text, PSTR("Add waypoint"));
	}

	if (index == fc.task.waypoint_count + 6)
	{
		strcpy_P(text, PSTR("Delete task"));
	}
}
