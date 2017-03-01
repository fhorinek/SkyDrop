/*
 * gui_flightlog2.cpp
 *
 * Let the user choose a file in the previously chosen directory.
 * The filename is stored in flightlog_file and then gui_flightlog3 is called.
 *
 *  Created on: 22.02.2017
 *      Author: tilmann@bubecks.de
 */

#include "gui_flightlog2.h"
#include "gui_flightlog1.h"
#include "../gui_list.h"
#include "../gui_dialog.h"
#include "../../fc/conf.h"
#include "../../fc/logger/logger.h"

/**
 * The file, that was selected in flightlog2 stage. It will be used by flightlog3.
 */
char flightlog_file[30];

void gui_flightlog2_init()
{
	gui_list_set(gui_flightlog2_item, gui_flightlog2_action, logger_count(flightlog_dir, false), GUI_FLIGHTLOG1);
}

void gui_flightlog2_stop() {}

void gui_flightlog2_loop()
{
	gui_list_draw();
}

void gui_flightlog2_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_flightlog2_action(uint8_t index)
{
	logger_fileno(flightlog_dir, index + 1, flightlog_file, false);
 	gui_switch_task(GUI_FLIGHTLOG3);
}

void gui_flightlog2_item(uint8_t idx, char * text, uint8_t * flags, char * sub_text)
{
	logger_fileno(flightlog_dir, idx + 1, text, false);

	strcpy(text, rindex(text, '/') + 1);   // copy only basename

	*flags |= GUI_LIST_FOLDER;
}

