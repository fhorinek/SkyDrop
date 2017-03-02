/*
 * gui_flightlog1.h
 *
 *  Created on: 22.02.2017
 *      Author: tilmann@bubecks.de
 */

#ifndef gui_flightlog1_H_
#define gui_flightlog1_H_

#include "../gui.h"

void gui_flightlog1_init();
void gui_flightlog1_stop();
void gui_flightlog1_loop();
void gui_flightlog1_irqh(uint8_t type, uint8_t * buff);
void gui_flightlog1_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_flightlog1_action(uint8_t index);

/**
 * The directory, that was selected in flightlog1 stage. It will be used by flightlog2
 * for file selection.
 */
extern char flightlog_dir[30];

#endif /* gui_flightlog1_H_ */
