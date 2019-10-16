/*
 * splash.h
 *
 *  Created on: 12.5.2015
 *      Author: horinek
 */

#ifndef SPLASH_H_
#define SPLASH_H_

#include "gui.h"

#define gui_splash_stop gui_dummy
#define gui_splash_irqh gui_dummy

void gui_splash_init();
void gui_splash_loop();

void gui_splash_set_mode(uint8_t mode);

#define SPLASH_ON	1
#define SPLASH_OFF	2

#endif /* SPLASH_H_ */
