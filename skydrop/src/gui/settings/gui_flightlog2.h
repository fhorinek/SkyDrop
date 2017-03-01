/*
 * gui_flightlog2.h
 *
 *  Created on: 22.02.2017
 *      Author: tilmann@bubecks.de
 */


#ifndef gui_flightlog2_H_
#define gui_flightlog2_H_

#include "../gui.h"

/**
 * The file, that was selected in flightlog2 stage. It will be used by flightlog3.
 */
extern char flightlog_file[30];

void gui_flightlog2_init();
void gui_flightlog2_stop();
void gui_flightlog2_loop();
void gui_flightlog2_irqh(uint8_t type, uint8_t * buff);
void gui_flightlog2_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_flightlog2_action(uint8_t index);

#endif /* gui_flightlog2_H_ */
