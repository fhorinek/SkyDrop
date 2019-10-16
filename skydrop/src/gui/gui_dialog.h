/*
 * gui_dialog.h
 *
 *  Created on: 30.6.2015
 *      Author: horinek
 */

#ifndef GUI_DIALOG_H_
#define GUI_DIALOG_H_

#include "gui.h"

#define GUI_STYLE_OK			0
#define GUI_STYLE_OKCANCEL		1
#define GUI_STYLE_YESNO			2
#define GUI_STYLE_STATS			3
#define GUI_STYLE_TIMESET		4
#define GUI_STYLE_FORMAT		5
#define GUI_STYLE_AIRSPACE		6

#define GUI_STYLE_NO_TITLE		0b10000000
#define GUI_STYLE_MASK			0b00111111

#define GUI_DIALOG_OK			1
#define GUI_DIALOG_CANCEL		2
#define GUI_DIALOG_YES			1
#define GUI_DIALOG_NO			2
#define GUI_DIALOG_SET			1
#define GUI_DIALOG_WAIT			2
#define GUI_DIALOG_FORMAT		2
#define GUI_DIALOG_NOW			1
#define GUI_DIALOG_FOREVER		2


typedef void uint8_cb(uint8_t val);

void gui_dialog_set_P(const char * title, const char * msg, uint8_t style, uint8_cb * cb);
void gui_dialog_set(char * title, char * msg, uint8_t style, uint8_cb * cb);

#define gui_dialog_init gui_dummy
#define gui_dialog_stop gui_dummy

void gui_dialog_loop();
void gui_dialog_irqh(uint8_t type, uint8_t * buff);

#endif /* GUI_DIALOG_H_ */
