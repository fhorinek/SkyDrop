/*
 * gui_airspaces.h
 *
 *  Created on: 8. 2. 2018
 *      Author: horinek
 */

#ifndef GUI_GUI_airspaces_H_
#define GUI_GUI_airspaces_H_

#include "../common.h"

void gui_airspace_init();

#define gui_airspace_stop gui_dummy

void gui_airspace_loop();
void gui_airspace_irqh(uint8_t type, uint8_t * buff);

#endif /* GUI_GUI_airspaces_H_ */
