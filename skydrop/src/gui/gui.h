/*
 * gui.h
 *
 *  Created on: 22.1.2015
 *      Author: horinek
 */

#ifndef GUI_H_
#define GUI_H_

#include "../common.h"
#include "../drivers/n5110_disp.h"
#include "../drivers/led.h"

extern n5110display disp;
extern FILE * lcd_out;

void gui_init();
void gui_stop();
void gui_loop();
void gui_irqh(uint8_t type, uint8_t * buff);

void gui_trigger_backlight();
void gui_showmessage(const char * msg);

void gui_draw_box();
#define GUI_BOX_TOP			3
#define GUI_BOX_LEFT		3
#define GUI_BOX_RIGHT		(n5110_width - GUI_BOX_TOP - 1)
#define GUI_BOX_BOTTOM		(n5110_height - GUI_BOX_LEFT - 1)

void gui_raligh_text(char * text, uint8_t x, uint8_t y);

#define GUI_NONE	0xFF
#define GUI_PAGES	0

#define GUI_NUMBER_OF_TASKS		1

#endif /* GUI_H_ */
