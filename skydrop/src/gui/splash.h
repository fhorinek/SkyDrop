/*
 * splash.h
 *
 *  Created on: 12.5.2015
 *      Author: horinek
 */

#ifndef SPLASH_H_
#define SPLASH_H_

#include "gui.h"

void gui_splash_init();
void gui_splash_stop();
void gui_splash_loop();
void gui_splash_irqh(uint8_t type, uint8_t * buff);

void gui_splash_set_mode(uint8_t mode);
void gui_splash_set_message(char * str);

#define SPLASH_BOOT	0
#define SPLASH_ON	1
#define SPLASH_OFF	2

#endif /* SPLASH_H_ */
