/*
 * widgets.h
 *
 *  Created on: 2.3.2015
 *      Author: horinek
 */

#ifndef WIDGETS_H_
#define WIDGETS_H_

#define MAX_WIDGES_PER_PAGE		9
#define MAX_NUMBER_OF_PAGES		5

#include "../gui.h"
#include "../../fc/fc.h"
#include "../pages.h"



#include "layouts/layouts.h"

struct widget
{
	const char * label;
	void (* draw)(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags);
	void (* menu_loop)(uint8_t flags);
	void (* menu_irqh)(uint8_t type, uint8_t * buff, uint8_t flags);
	uint8_t flags;
};

#define register_widget3(name, label, draw, loop, irqh, flags) \
	const char name ## _label[] PROGMEM = label;\
	widget name = { \
		name ## _label, \
		draw, \
		loop, \
		irqh, \
		flags, \
	}; \

#define register_widget2(name, label, draw, loop, irqh) \
	const char name ## _label[] PROGMEM = label;\
	widget name = { \
		name ## _label, \
		draw, \
		loop, \
		irqh, \
		0, \
	}; \

#define register_widget1(name, label, draw) \
	const char name ## _label[] PROGMEM = label;\
	widget name = { \
		name ## _label, \
		draw, \
		NULL, \
		NULL, \
		0, \
	}; \


extern uint8_t widget_menu_state;
extern uint8_t widget_menu_param1;
extern float widget_menu_fvalue1;

#define WIDGET_OFF				0xFF
#define WIDGET_EMPTY			0
#define WIDGET_DEBUG_ETC		1
#define WIDGET_DEBUG_IMU		2
#include "dummy.h"

#define WIDGET_VARIO			3
#define WIDGET_AVG_VARIO		4
#define WIDGET_VARIO_BAR		5
#include "vario.h"

#define WIDGET_ALT1				6
#define WIDGET_ALT2				7
#define WIDGET_ALT3				8
#define WIDGET_ALT4				9
#define WIDGET_ALT5				10
#include "altitude.h"

#define WIDGET_ACCX				11
#include "acc.h"

#define WIDGET_TIME				12
#define WIDGET_FTIME			13
#include "time.h"

#define WIDGET_TEMPERATURE		14
#include "temperature.h"

#define WIDGET_GHEADING			15
#define WIDGET_GROUND_SPD		16
#define WIDGET_POSITION			17
#define WIDGET_GHEADING_ARROW	18
#include "gps.h"

#define WIDGET_BATTERY			19
#include "battery.h"

#define WIDGET_GLIDE_RATIO		20
#include "flight.h"

#define WIDGET_CTRL_AUDIO		21
#define WIDGET_CTRL_WLIFT		22
#include "ctrl.h"

#define NUMBER_OF_WIDGETS		23


void widgets_draw(uint8_t page);

uint8_t widget_label_P(const char * label, uint8_t x, uint8_t y);
uint8_t widget_label(char * label, uint8_t x, uint8_t y);

void widget_value_int(char * value, uint8_t x, uint8_t y, uint8_t w, uint8_t h);
void widget_value_int_sub(char * value, char * sub, uint8_t x, uint8_t y, uint8_t w, uint8_t h);

void widget_value_txt(char * value, uint8_t x, uint8_t y, uint8_t w, uint8_t h);
void widget_value_txt2(char * value1, char * value2, uint8_t x, uint8_t y, uint8_t w, uint8_t h);

uint8_t layout_get_number_of_widgets(uint8_t type);
void layout_get_widget_rect(uint8_t type, uint8_t widget, uint8_t * x, uint8_t * y, uint8_t * w, uint8_t * h);

uint8_t widget_get_type(uint8_t page, uint8_t widget);

extern widget widget_array[NUMBER_OF_WIDGETS];

#endif /* WIDGETS_H_ */
