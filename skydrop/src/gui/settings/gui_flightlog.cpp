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
char gui_flightlog_name[13];

uint8_t gui_flightlog_level = 0;
uint8_t file_task;

void gui_flightlog_set_dir(char * path)
{
	gui_list_set_index(GUI_FLIGHTLOG, 0);
	strcpy(gui_flightlog_path, path);
}

/**
 * The the file manager, which task should be switched to if the user selects a file.
 * This page typically shows details about the file.
 *
 * @param task the next task to switch for file.
 */
void gui_flightlog_set_file_task(uint8_t task) {
	file_task = task;
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

void gui_flightlog_loop()
{
	gui_list_draw();
}

void gui_flightlog_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_flightlog_action(uint8_t index)
{
	char tmp[44];
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
		storage_dir_list(gui_flightlog_name, &flags);
		i++;
	}
	while (i <= index - 1);

	sprintf_P(tmp, PSTR("%s/%s"), gui_flightlog_path, gui_flightlog_name);

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
		gui_switch_task(file_task);
	}
}

void gui_flightlog_item(uint8_t ind, char * text, uint8_t * flags, char * sub_text)
{
	if (ind == 0)
	{
		*flags |= GUI_LIST_TITLE;

		if (gui_flightlog_level == 0)
		{
			strcpy(text, gui_flightlog_path + 1);
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

