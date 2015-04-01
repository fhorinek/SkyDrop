/*
 * widgets.h
 *
 *  Created on: 2.3.2015
 *      Author: horinek
 */

#ifndef WIDGETS_H_
#define WIDGETS_H_

#define WIDGES_PER_PAGE		9
#define NUMBER_OF_PAGES		5

#include "../gui.h"
#include "../../fc/fc.h"
#include "layouts/layouts.h"


struct widget
{
	void (* draw)(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
	const char * label;
	void (* menu_draw)();
	void (* menu_handler)(uint8_t type, uint8_t * buff);
};

#define register_widget(name, draw, label, menu, menu_draw) \
	const char name ## _label[] PROGMEM = label;\
	widget name = { \
		draw, \
		name ## _label, \
		menu, \
		menu_draw, \
	}; \


#define WIDGET_OFF			0xFF
#define WIDGET_DUMMY		0
#include "dummy.h"

#define WIDGET_VARIO		1
#define WIDGET_AVG_VARIO	2
#define WIDGET_VARIO_BAR	3
#include "vario.h"

#define WIDGET_ALT1			4
#define WIDGET_ALT2			5
#include "altitude.h"

#define WIDGET_ACCX			6
#include "acc.h"


void widgets_init();
void widgets_draw(uint8_t page);

uint8_t widget_label(const char * label, uint8_t x, uint8_t y);
void widget_value_int(char * value, uint8_t x, uint8_t y, uint8_t w, uint8_t h);

uint8_t layout_get_number_of_widgets(uint8_t type);
void layout_get_widget_rect(uint8_t type, uint8_t widget, uint8_t * x, uint8_t * y, uint8_t * w, uint8_t * h);

uint8_t widget_get_type(uint8_t page, uint8_t widget);

#endif /* WIDGETS_H_ */
