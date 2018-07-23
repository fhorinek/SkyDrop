/*
 * gui_filemanager.cpp
 *
 *
 *  Created on: 22.02.2017
 *      Author: tilmann@bubecks.de
 */

#include <gui/settings/gui_filemanager.h>
#include "gui_flightdetail.h"

#include "../gui_list.h"

#include "../../fc/logger/logger.h"
#include "../../drivers/storage/storage.h"


char gui_filemanager_path[32];
char gui_filemanager_name[13];
int16_t direntry_count;

uint8_t gui_filemanager_level = 0;
uint8_t filemanager_action;
uint8_t filemanager_return = GUI_SETTINGS;

void gui_filemanager_set_dir(char * path)
{
	gui_list_set_index(GUI_FILEMANAGER, 0);
	strcpy(gui_filemanager_path, path);
}

/**
 * The file manager, which task should be switched to if the user selects a file.
 * This page typically shows details about the file.
 *
 * @param task the next action to switch for file.
 * @param ret the return task
 */
void gui_filemanager_set_tasks(uint8_t action, uint8_t ret)
{
	filemanager_action = action;
	filemanager_return = ret;
}

void gui_filemanager_init()
{
	storage_dir_open(gui_filemanager_path);

	direntry_count = storage_dir_get_count();
	gui_list_set(gui_filemanager_item, gui_filemanager_action, direntry_count + 1, filemanager_return);
}

void gui_filemanager_stop()
{
	storage_dir_close();

	if (gui_new_task == filemanager_return)
	{
		if (gui_filemanager_level > 0)
		{
			char * p;

			p = rindex(gui_filemanager_path, '/');
			if (p != NULL)
				*p = 0;

			gui_filemanager_level--;

			gui_switch_task(GUI_FILEMANAGER);
		}
	}
}

void gui_filemanager_loop()
{
	gui_list_draw();
}

void gui_filemanager_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_filemanager_action(uint8_t index)
{
	char tmp[44];
	uint8_t flags = 0;

	if (index == 0)
	{
		gui_switch_task(filemanager_return);
		return;
	}

	storage_dir_rewind();
	storage_dir_list_n(gui_filemanager_name, &flags, direntry_count - index + 1);   // Reverse the order

	sprintf_P(tmp, PSTR("%s/%s"), gui_filemanager_path, gui_filemanager_name);

	if (flags & STORAGE_IS_DIR)
	{
		gui_filemanager_set_dir(tmp);

		//going in to dir
		storage_dir_close();
		gui_filemanager_init();
		gui_filemanager_level++;
	}
	else
	{
		gui_switch_task(filemanager_action);
	}
}

void gui_filemanager_item(uint8_t ind, char * text, uint8_t * flags, char * sub_text)
{
	if (ind == 0)
	{
		*flags |= GUI_LIST_TITLE;

		if (gui_filemanager_level == 0)
		{
			// Shows the name of the top most dir, e.g. "home" or "logs"
			strcpy(text, gui_filemanager_path + 1);
		}
		else
		{
			// Shows the directory names under the top most, excluding the top most
			char * p = index(gui_filemanager_path + 1, '/');
			strcpy(text, p);
		}

		return;
	}

	uint8_t f = 0;

	storage_dir_rewind();
	storage_dir_list_n(text, &f, direntry_count - ind + 1);   // Reverse the order

	if (f & STORAGE_IS_DIR)
		*flags |= GUI_LIST_FOLDER;
}

