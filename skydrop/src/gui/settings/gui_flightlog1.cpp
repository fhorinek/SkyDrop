/*
 * gui_flightlog1.cpp
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
	//config.system.debug_log = DEBUG_MAGIC_ON;

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
 	DEBUG("\n\n\nCalling FLIGHTLOG2 with %s\n", flightlog_dir);
 	gui_switch_task(GUI_FLIGHTLOG2);
}

void gui_flightlog1_item(uint8_t idx, char * text, uint8_t * flags, char * sub_text)
{

	if (true) {
		logger_fileno(idx + 1, text, true);
		strcpy(text, text + 6);         // strlen("/logs/") = 6
		text[10] = 0;                   // strlen("2017/02/17") = 10
		*flags |= GUI_LIST_FOLDER;

		DEBUG("gui_flightlog_item(%d)=%s\n", idx, text);

	} else {
		logger_fileno(idx + 1, text, false);

		strcpy(sub_text, text + 17);    // strlen("/logs/2017/02/17/") = 17

		// Remove preceeding LOG_DIR:
		strcpy(text, text + 6);         // strlen("/logs/") = 6
		text[10] = 0;                   // strlen("2017/02/17") = 10
		DEBUG("gui_flightlog_item(%d)=%s,%s\n", idx, text, sub_text);

		*flags |= GUI_LIST_SUB_TEXT;
	}
}

