/*
 * gui_flightlog3.h
 *
 *  Created on: 22.02.2017
 *      Author: tilmann@bubecks.de
 */


#ifndef gui_flightlog3_H_
#define gui_flightlog3_H_

#include "../gui.h"

void gui_flightlog3_init();
void gui_flightlog3_stop();
void gui_flightlog3_loop();
void gui_flightlog3_irqh(uint8_t type, uint8_t * buff);
void gui_flightlog3_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_flightlog3_action(uint8_t index);

#endif /* gui_flightlog3_H_ */
