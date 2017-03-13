/*
 * gui_flightdetail.h
 *
 *  Created on: 22.02.2017
 *      Author: tilmann@bubecks.de
 */


#ifndef GUI_FLIGHTDETAIL_H_
#define GUI_FLIGHTDETAIL_H_

#include "../gui.h"

void gui_flightdetail_init();
void gui_flightdetail_stop();
void gui_flightdetail_loop();
void gui_flightdetail_irqh(uint8_t type, uint8_t * buff);
void gui_flightdetail_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_flightdetail_action(uint8_t index);

void gui_flightdetail_parse_logfile(const char *filename);

#endif /* GUI_FLIGHTDETAIL_H_ */
