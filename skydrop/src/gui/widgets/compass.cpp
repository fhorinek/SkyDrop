/*
 * compass.cpp
 *
 *  Created on: Mar 28, 2017
 *      Author: fiala
 */

#include "compass.h"

void widget_compass_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("Comp"), x, y);

	char tmp[5];
	sprintf_P(tmp, PSTR("%0.0f"), fc.compass.azimuth_filtered);

	widget_value_int(tmp, x, y + lh, w, h - lh);
}
void widget_compass_arrow_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("Comp"), x, y);

	y += lh / 2;

	int16_t relative_direction = - int(fc.compass.azimuth);

	widget_arrow(relative_direction, x, y, w, h);
}
void widget_compass_heading_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("Comp"), x, y);

	char tmp[5];

	widget_deg_to_points(fc.compass.azimuth_filtered, tmp);

	widget_value_txt(tmp, x, y + lh, w, h - lh);
}
register_widget1(w_compass_heading, "Compass heading", widget_compass_draw);
register_widget1(w_compass_arrow, "Compass arrow", widget_compass_arrow_draw);
register_widget1(w_compass_points, "Compass points", widget_compass_heading_draw);
