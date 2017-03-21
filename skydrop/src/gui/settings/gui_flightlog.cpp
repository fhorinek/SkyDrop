/*
 * gui_flightlog.cpp
 *
 *
 *  Created on: 22.02.2017
 *      Author: tilmann@bubecks.de
 */

#include "gui_flightlog.h"
#include "gui_flightdetail.h"

#include "../gui_list.h"

#include "../../fc/logger/logger.h"
#include "../../drivers/storage/storage.h"


char gui_flightlog_path[32];
uint8_t gui_flightlog_level = 0;

void gui_flightlog_set_dir(char * path)
{
	gui_list_set_index(GUI_FLIGHTLOG, 0);

	if (path == NULL)
	{
		sprintf_P(gui_flightlog_path, PSTR("/%S"), LOG_DIR_P);
		gui_flightlog_level = 0;
	}
	else
	{
		strcpy(gui_flightlog_path, path);
	}
}

void gui_flightlog_init()
{
	storage_dir_open(gui_flightlog_path);

	gui_list_set(gui_flightlog_item, gui_flightlog_action, storage_get_files() + 1, GUI_SETTINGS);
}

void gui_flightlog_stop()
{
	storage_dir_close();

	if (gui_new_task == GUI_SETTINGS)
	{
		if (gui_flightlog_level > 0)
		{
			char * p;

			p = rindex(gui_flightlog_path, '/');
			if (p != NULL)
				*p = 0;

			gui_flightlog_level--;

			gui_switch_task(GUI_FLIGHTLOG);
		}
	}
}

void gui_flightlog_action(uint8_t index)
{
	char tmp[44];
	char name[13];
	uint8_t flags = 0;

	if (index == 0)
	{
		gui_switch_task(GUI_SETTINGS);
		return;
	}

	storage_dir_rewind();

	uint8_t i = 0;
	do
	{
		storage_dir_list(name, &flags);
		i++;
	}
	while (i <= index - 1);

	sprintf_P(tmp, PSTR("%s/%s"), gui_flightlog_path, name);

	if (flags & STORAGE_IS_DIR)
	{
		gui_flightlog_set_dir(tmp);

		//going in to dir
		storage_dir_close();
		gui_flightlog_init();
		gui_flightlog_level++;
	}
	else
	{
		gui_flightdetail_parse_logfile(tmp);
		gui_switch_task(GUI_FLIGHTDETAIL);
	}
}

void gui_flightlog_item(uint8_t ind, char * text, uint8_t * flags, char * sub_text)
{
	if (ind == 0)
	{
		*flags |= GUI_LIST_TITLE;

		if (gui_flightlog_level == 0)
		{
			strcpy_P(text, PSTR("Logs"));
		}
		else
		{
			char * p = index(gui_flightlog_path + 1, '/');
			strcpy(text, p);
		}

		storage_dir_rewind();
		return;
	}

	uint8_t f = 0;

	storage_dir_list(text, &f);

	if (f & STORAGE_IS_DIR)
		*flags |= GUI_LIST_FOLDER;
}

