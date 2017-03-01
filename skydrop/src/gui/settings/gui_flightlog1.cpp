/*
 * gui_flightlog1.cpp
 *
 * Let the user choose a log directory. If he selects one, then store that
 * name in flightlog_dir and call gui_flightlog2.
 *
 *  Created on: 22.02.2017
 *      Author: tilmann@bubecks.de
 */

#include "gui_flightlog1.h"
#include "../gui_list.h"
#include "../gui_dialog.h"
#include "../../fc/conf.h"
#include "../../fc/logger/logger.h"

/**
 * The directory, that was selected in flightlog1 stage. It will be used by flightlog2
 * for file selection.
 */
char flightlog_dir[30];

void gui_flightlog1_init()
{
	gui_list_set(gui_flightlog1_item, gui_flightlog1_action, logger_count(true), GUI_SETTINGS);
}

void gui_flightlog1_stop() {}

void gui_flightlog1_loop()
{
	gui_list_draw();
}

void gui_flightlog1_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_flightlog1_action(uint8_t index)
{
 	logger_fileno(index + 1, flightlog_dir, true);
 	gui_switch_task(GUI_FLIGHTLOG2);
}

void gui_flightlog1_item(uint8_t idx, char * text, uint8_t * flags, char * sub_text)
{
	logger_fileno(idx + 1, text, true);
	strcpy(text, index(text, '/') + 1);         // strlen("/logs/") = 6
	//text[10] = 0;                   // strlen("2017/02/17") = 10
	*flags |= GUI_LIST_FOLDER;
}

