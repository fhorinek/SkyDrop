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

#define gui_pages_stop gui_dummy

void gui_pages_loop();
void gui_pages_irqh(uint8_t type, uint8_t * buff);

bool gui_enter_widget_menu();
void gui_exit_widget_menu();
void gui_page_power_off();
void page_switch(bool right);
void page_set(uint8_t new_page);

void gui_page_set_mode(uint8_t mode);

extern uint8_t active_page;

#endif /* PAGES_H_ */
