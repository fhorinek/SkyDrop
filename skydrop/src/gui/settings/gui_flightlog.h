/*
 * gui_flightlog.h
 *
 *  Created on: 22.02.2017
 *      Author: tilmann@bubecks.de
 */

#ifndef gui_flightlog_H_
#define gui_flightlog_H_

#include "../gui.h"

void gui_flightlog_init();
void gui_flightlog_stop();
void gui_flightlog_loop();
void gui_flightlog_irqh(uint8_t type, uint8_t * buff);
void gui_flightlog_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_flightlog_action(uint8_t index);

void gui_flightlog_set_dir(char * path);

/**
 * The the file manager, which task should be switched to if the user selects a file.
 * This page typically shows details about the file.
 *
 * @param task the next task to switch for file.
 */
void gui_flightlog_set_file_task(uint8_t task);

/**
 * The directory level.
 */
extern uint8_t gui_flightlog_level;

/**
 * This holds the directory of the selected file.
 */
extern char gui_flightlog_path[32];

/**
 * This holds the file name of the selected file.
 */
extern char gui_flightlog_name[13];

#endif /* gui_flightlog_H_ */
