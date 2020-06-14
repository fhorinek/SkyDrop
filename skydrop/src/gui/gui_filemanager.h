/*
 * gui_filemanager.h
 *
 *  Created on: 22.02.2017
 *      Author: tilmann@bubecks.de
 */

#ifndef gui_filemanager_H_
#define gui_filemanager_H_

#include "gui.h"

void gui_filemanager_init();
void gui_filemanager_stop();

#define gui_filemanager_loop gui_list_draw
#define gui_filemanager_irqh gui_list_irqh

void gui_filemanager_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_filemanager_action(uint8_t index);

void gui_filemanager_set_dir(char * path);

/**
 * The the file manager, which task should be switched to if the user selects a file.
 * This page typically shows details about the file.
 *
 * @param action the next action to switch for file.
 * @param ret the return task
 */
void gui_filemanager_set_tasks(uint8_t action, uint8_t ret);

/**
 * The directory level.
 */
extern uint8_t gui_filemanager_level;

/**
 * This holds the directory of the selected file.
 */
extern char gui_filemanager_path[32];

/**
 * This holds the file name of the selected file.
 */
extern char gui_filemanager_name[13];

#endif /* gui_filemanager_H_ */
