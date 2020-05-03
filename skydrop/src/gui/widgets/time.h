/*
 * dummy.h
 *
 *  Created on: 3.3.2015
 *      Author: horinek
 */

#ifndef GUI_WIDGET_TIME_H_
#define GUI_WIDGET_TIME_H_

#include "widgets.h"

extern widget w_time;
extern widget w_date;
extern widget w_flight_time;

void widget_time_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h);

#endif /* TIME_H_ */
