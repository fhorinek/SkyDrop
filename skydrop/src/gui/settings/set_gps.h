/*
 * settings.h
 *
 *  Created on: 5.5.2015
 *      Author: horinek
 */

#ifndef SET_GPS_H_
#define SET_GPS_H_

#include "../gui.h"

#define gui_set_gps_loop gui_list_draw
#define gui_set_gps_irqh gui_list_irqh

void gui_set_gps_init();
void gui_set_gps_stop();
void gui_set_gps_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_set_gps_action(uint8_t index);

#endif /* SETTINGS_H_ */
