/*
 * widgets.h
 *
 *  Created on: 2.3.2015
 *      Author: horinek
 */

#ifndef WIDGETS_H_
#define WIDGETS_H_

#define MAX_WIDGES_PER_PAGE		9
#define MAX_NUMBER_OF_PAGES		8

#include "../gui.h"
#include "../../fc/fc.h"
#include "../pages.h"

#include "layouts/layouts.h"

struct widget
{
	const char * label;
	void (* draw)(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
	void (* menu_loop)();
	void (* menu_irqh)(uint8_t type, uint8_t * buff);
};

#define register_widget2(name, label, draw, loop, irqh) \
	const char name ## _label[] PROGMEM = label;\
	widget name = { \
		name ## _label, \
		draw, \
		loop, \
		irqh, \
	}; \

#define register_widget1(name, label, draw) \
	const char name ## _label[] PROGMEM = label;\
	widget name = { \
		name ## _label, \
		draw, \
		NULL, \
		NULL, \
	}; \


extern uint8_t widget_menu_state;
extern uint8_t widget_menu_param1;
extern float widget_menu_fvalue1;

#define WIDGET_OFF				0xFF
#define WIDGET_EMPTY			0
#include "dummy.h"

#define WIDGET_VARIO			1
#define WIDGET_AVG_VARIO		2
#define WIDGET_VARIO_BAR		3
#define WIDGET_VARIO_HISTORY	4
#include "vario.h"

#define WIDGET_ALT1				5
#define WIDGET_ALT2				6
#define WIDGET_ALT3				7
#define WIDGET_ALT4				8
#define WIDGET_ALT5				9
#include "altitude.h"

#define WIDGET_ACC_TOT			10
#include "acc.h"

#define WIDGET_TIME				11
#define WIDGET_DATE				12
#define WIDGET_FTIME			13
#define WIDGET_HIKE				14
#include "time.h"

#define WIDGET_TEMPERATURE		15
#include "temperature.h"

#define WIDGET_GHEADING			16
#define WIDGET_GROUND_SPD		17
#define WIDGET_POSITION			18
#define WIDGET_GHEADING_ARROW	19
#define WIDGET_GHEADING_POINTS	20
#include "gps.h"

#define WIDGET_BATTERY			21
#include "battery.h"

#define WIDGET_GLIDE_RATIO		22
#include "flight.h"

#define WIDGET_CTRL_AUDIO		23
#define WIDGET_CTRL_WLIFT		24
#include "ctrl.h"

#define WIDGET_WIND_SPD			25
#define WIDGET_WIND_DIR			26
#define WIDGET_WIND_DIR_ARROW	27
#define WIDGET_WIND_DIR_POINTS	28
#include "wind.h"

#define WIDGET_AGL_HEIGHT		29
#define WIDGET_AGL_LEVEL		30
#include "agl.h"

#define WIDGET_ODO_METER		31
#define WIDGET_ODO_BACK			32
#define WIDGET_ODO_DISTANCE		33
#define WIDGET_HOME_TIME		34
#define WIDGET_HOME_INFO		35
#include <gui/widgets/navigation.h>

#define WIDGET_COMPASS			36
#define WIDGET_COMPASS_ARROW	37
#define WIDGET_COMPASS_POINTS	38
#include "compass.h"

#define WIDGET_THERMAL_TIME     39
#define WIDGET_THERMAL_GAIN     40
#define WIDGET_THERMAL_ASS      41
#include "thermal.h"

#define WIDGET_WAYPOINT_ARROW       42
#define WIDGET_WAYPOINT_DISTANCE    43
#define WIDGET_WAYPOINT_TIME        44
#define WIDGET_WAYPOINT_INFO        45

#define WIDGET_AIRSPACE_INFO	    46
#include "airspace.h"

#define NUMBER_OF_WIDGETS			47

#define NUMBER_OF_SORTED_WIDGETS	47
#define SORTED_INDEX_FOR_TEMPERATURE	(NUMBER_OF_SORTED_WIDGETS - 4)

/**
 * Format a distance in a human readable format.
 *
 * @param text_number the text buffer to print the value into.
 * @param text_unit the text buffer to print the unit into.
 * @param distance the distance in km.
 */
extern void sprintf_distance(char *text_number, char *text_unit, float distance);

extern const uint8_t PROGMEM widget_sorted[NUMBER_OF_SORTED_WIDGETS];
uint8_t widget_sorted_get_index(uint8_t pos);

void widgets_draw(uint8_t page);

uint8_t widget_label_P(const char * label, uint8_t x, uint8_t y);
uint8_t widget_label(char * label, uint8_t x, uint8_t y);

void widget_value_int(char * value, uint8_t x, uint8_t y, uint8_t w, uint8_t h);
void widget_value_int_sub(char * value, char * sub, uint8_t x, uint8_t y, uint8_t w, uint8_t h);

void widget_value_txt(char * value, uint8_t x, uint8_t y, uint8_t w, uint8_t h);
void widget_value_txt2(char * value1, char * value2, uint8_t x, uint8_t y, uint8_t w, uint8_t h);
void widget_value_scroll(char * text, uint8_t x, uint8_t y, uint8_t w, uint8_t h);

void widget_arrow(int16_t angle, uint8_t x, uint8_t y, uint8_t w, uint8_t h);

uint8_t layout_get_number_of_widgets(uint8_t type);
void layout_get_widget_rect(uint8_t type, uint8_t widget, uint8_t * x, uint8_t * y, uint8_t * w, uint8_t * h);

uint8_t widget_get_type(uint8_t page, uint8_t widget);

//converts degrees to points N
void widget_deg_to_points(float deg, char * text);

extern widget widget_array[NUMBER_OF_WIDGETS];

#endif /* WIDGETS_H_ */
