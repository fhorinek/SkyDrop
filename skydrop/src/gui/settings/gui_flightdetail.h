/*
 * gui_flightdetail.h
 *
 *  Created on: 22.02.2017
 *      Author: tilmann@bubecks.de
 */


#ifndef GUI_FLIGHTDETAIL_H_
#define GUI_FLIGHTDETAIL_H_

#include "../gui.h"

#define gui_flightdetail_stop gui_dummy
#define gui_flightdetail_loop gui_list_draw
#define gui_flightdetail_irqh gui_list_irqh
#define gui_flightdetail_action gui_dummy

void gui_flightdetail_init();
void gui_flightdetail_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);

void gui_flightdetail_parse_logfile(const char *filename);

#endif /* GUI_FLIGHTDETAIL_H_ */
