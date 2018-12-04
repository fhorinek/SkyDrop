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
#define WIDGET_VARIO_HISTORY	6
#include "vario.h"

#define WIDGET_ALT1				7
#define WIDGET_ALT2				8
#define WIDGET_ALT3				9
#define WIDGET_ALT4				10
#define WIDGET_ALT5				11
#include "altitude.h"

#define WIDGET_ACC_TOT			12
#include "acc.h"

#define WIDGET_TIME				13
#define WIDGET_DATE				14
#define WIDGET_FTIME			15
#include "time.h"

#define WIDGET_TEMPERATURE		16
#include "temperature.h"

#define WIDGET_GHEADING			17
#define WIDGET_GROUND_SPD		18
#define WIDGET_POSITION			19
#define WIDGET_GHEADING_ARROW	20
#define WIDGET_GHEADING_POINTS	21
#include "gps.h"

#define WIDGET_BATTERY			22
#include "battery.h"

#define WIDGET_GLIDE_RATIO		23
#include "flight.h"

#define WIDGET_CTRL_AUDIO		24
#define WIDGET_CTRL_WLIFT		25
#include "ctrl.h"

#define WIDGET_WIND_SPD			26
#define WIDGET_WIND_DIR			27
#define WIDGET_WIND_DIR_ARROW	28
#define WIDGET_WIND_DIR_POINTS	29
#include "wind.h"

#define WIDGET_AGL_HEIGHT		30
#define WIDGET_AGL_LEVEL		31
#include "agl.h"

#define WIDGET_ODO_METER		32
#define WIDGET_ODO_BACK			33
#define WIDGET_ODO_DISTANCE		34
#define WIDGET_HOME_TIME		35
#define WIDGET_HOME_INFO		36
#include "odometer.h"

#define WIDGET_COMPASS			37
#define WIDGET_COMPASS_ARROW	38
#define WIDGET_COMPASS_POINTS	39
#include "compass.h"

#define WIDGET_THERMAL_TIME     40
#define WIDGET_THERMAL_GAIN     41
#include "thermal.h"

#define WIDGET_WAYPOINT_ARROW       42
#define WIDGET_WAYPOINT_DISTANCE    43
#define WIDGET_WAYPOINT_TIME        44
#define WIDGET_WAYPOINT_INFO        45

#define WIDGET_AIRSPACE_ARROW       46
#define WIDGET_AIRSPACE_DISTANCE    47
#define WIDGET_AIRSPACE_MAX_HEIGHT  48
#include "airspace.h"

#define NUMBER_OF_WIDGETS		49

#define NUMBER_OF_SORTED_WIDGETS	47

/**
 * Format a distance in a human readable format.
 *
 * @param text the text buffer to print into.
 * @param distance the distance in km.
 */
extern void sprintf_distance(char *text, float distance);

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
