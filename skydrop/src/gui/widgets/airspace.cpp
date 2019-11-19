/*
 * airspace.cpp
 *
 *  Created on: 02.12.2018
 *      Author: tilmann@bubecks.de
 */

#include "airspace.h"
#include "../../fc/airspace.h"
#include "../../fc/agl.h"


void widget_airspace_distance_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{

}

void widget_airspace_limits_sprintf(char * text, uint16_t raw)
{


}

void widget_airspace_limits_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("AS Limit"), x, y);
	char text1[15];
	char text2[15];

	if (fc.airspace.inside)
	{
		strcpy_P(text1, PSTR("Inside!"));
		widget_value_txt(text1, x, y + lh, w, h - lh);
		return;
	}

	widget_airspace_limits_sprintf(text1, fc.airspace.max_alt);
	widget_airspace_limits_sprintf(text2, fc.airspace.min_alt);

	widget_value_txt2(text1, text2, x, y + lh, w, h - lh);
}

void widget_airspace_angle_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{

}

void widget_airspace_info_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{

}

void widget_airspace_name_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{

}

register_widget1(w_airspace_distance, "Airspace Distance", widget_airspace_distance_draw);
register_widget1(w_airspace_angle,    "Airspace Arrow",    widget_airspace_angle_draw);
register_widget1(w_airspace_limits,   "Airspace Limits",   widget_airspace_limits_draw);
register_widget1(w_airspace_info,     "Airspace Info",     widget_airspace_info_draw);
register_widget1(w_airspace_name,     "Airspace Name",     widget_airspace_name_draw);
