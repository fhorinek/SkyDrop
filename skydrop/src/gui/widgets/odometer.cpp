/*
 * odometer.cpp
 *
 *  Created on: 16.02.2017
 *      Author: tilmann@bubecks.de
 */

#include "odometer.h"

void widget_odometer_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("Odometer"), x, y);

	char text[10];

	sprintf_P(text, PSTR("%.2f"), fc.odometer / 100000.0);   // cm to km

	widget_value_int(text, x, y + lh, w, h - lh);
}

register_widget1(w_odo_meter, "Odometer", widget_odometer_draw);



