/*
 * gui_text.h
 *
 *  Created on: 21.1.2016
 *      Author: horinek
 */

#ifndef GUI_TEXT_H_
#define GUI_TEXT_H_

#include "gui.h"
#include "gui_dialog.h"

#define GUI_TEXT_OK		1
#define GUI_TEXT_CANCEL	0

typedef void uint8_t_cb(uint8_t val, char * buff);

#define gui_text_init gui_dummy
#define gui_text_stop gui_dummy

void gui_text_conf(char * text, uint8_t max_len, uint8_t_cb * cb);
void gui_text_loop();
void gui_text_irqh(uint8_t type, uint8_t * buff);

#endif /* GUI_TEXT_H_ */
