/*
 * settings.h
 *
 *  Created on: 5.5.2015
 *      Author: horinek
 */

#ifndef set_gps_detail_H_
#define set_gps_detail_H_

#include "../gui.h"

#define gui_set_gps_detail_stop gui_dummy

void gui_set_gps_detail_init();
void gui_set_gps_detail_loop();
void gui_set_gps_detail_irqh(uint8_t type, uint8_t * buff);

#endif /* SETTINGS_H_ */
