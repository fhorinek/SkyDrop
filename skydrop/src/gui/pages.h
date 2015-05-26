/*
 * pages.h
 *
 *  Created on: 11.3.2015
 *      Author: horinek
 */

#ifndef PAGES_H_
#define PAGES_H_

#include "gui.h"

void gui_pages_init();
void gui_pages_stop();
void gui_pages_loop();
void gui_pages_irqh(uint8_t type, uint8_t * buff);

bool gui_enter_widget_menu();
void gui_exit_widget_menu();

extern uint8_t active_page;

#endif /* PAGES_H_ */
