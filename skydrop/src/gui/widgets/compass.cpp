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

	float dir = fc.compass.azimuth_filtered;
	char tmp[5];

	if	   ( 22.5 <= dir and dir <  67.5 )
		strcpy_P(tmp, PSTR("NE"));
	else if( 67.5 <= dir and dir < 112.5 )
		strcpy_P(tmp, PSTR("E"));
	else if(112.5 <= dir and dir < 157.5 )
		strcpy_P(tmp, PSTR("SE"));
	else if(157.5 <= dir and dir < 202.5 )
		strcpy_P(tmp, PSTR("S"));
	else if(202.5 <= dir and dir < 247.5 )
		strcpy_P(tmp, PSTR("SW"));
	else if(247.5 <= dir and dir < 292.5 )
		strcpy_P(tmp, PSTR("W"));
	else if(292.5 <= dir and dir < 337.5 )
		strcpy_P(tmp, PSTR("NW"));
	else
		strcpy_P(tmp, PSTR("N"));

	widget_value_txt(tmp, x, y + lh, w, h - lh);
}
register_widget1(w_compass_heading, "Compass heading", widget_compass_draw);
register_widget1(w_compass_arrow, "Compass arrow", widget_compass_arrow_draw);
register_widget1(w_compass_points, "Compass points", widget_compass_heading_draw);
