/*
 * update.h
 *
 *  Created on: 26.8.2015
 *      Author: horinek
 */

#ifndef UPDATE_H_
#define UPDATE_H_

#include "gui.h"

#define gui_update_stop gui_dummy
#define gui_update_irqh gui_dummy

void gui_update_init();
void gui_update_loop();
void gui_update_cb(uint8_t ret);

#endif /* UPDATE_H_ */
