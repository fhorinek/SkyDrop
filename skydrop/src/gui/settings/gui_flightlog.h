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

#endif /* gui_flightlog_H_ */
