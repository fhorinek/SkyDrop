/*
 * airspace.cpp
 *
 *  Created on: 02.12.2018
 *      Author: tilmann@bubecks.de
 */

#include "airspace.h"
#include "../../fc/airspace.h"

void widget_invert_if_forbidden(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	if ( fc.airspace.forbidden )
	{
		if (GUI_BLINK_TGL(1000))
		{
			disp.Invert(x, y, x+w-2, y+h-2);
		}
	}
}

void widget_airspace_distance_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("CTR Dist"), x, y);

	char text[10];

	if (fc.airspace.angle == AIRSPACE_INVALID)
	{
		strcpy_P(text, PSTR("---"));
	}
	else
	{
		if (fc.airspace.distance_m == AIRSPACE_LONGEST_DISTANCE)
		{
			text[0] = '>';
			sprintf_distance(text + 1, fc.airspace.distance_m / 1000.0);
		}
		else
		{
			sprintf_distance(text, fc.airspace.distance_m / 1000.0);
		}
	}

	widget_value_txt(text, x, y + lh, w, h - lh);
	widget_invert_if_forbidden(x, y, w, h);
}

void widget_airspace_max_height_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("CTR Height"), x, y);
	bool flashing = false;
	char text[15];

	if (fc.airspace.angle == AIRSPACE_INVALID) {
		strcpy_P(text, PSTR("---"));
	}
	else
	{
		uint16_t max_height;
		const char *unit_P;

		if (config.altitude.alt1_flags & ALT_UNIT_I)
		{
			max_height = fc.airspace.max_height_m * FC_METER_TO_FEET;
			unit_P = PSTR("ft");
		}
		else
		{
			max_height = fc.airspace.max_height_m;
			unit_P = PSTR("m");
		}
		sprintf_P(text, PSTR("<%u%S"), max_height, unit_P);

		if (fc.altitude1 > fc.airspace.max_height_m )
		{
			if (GUI_BLINK_TGL(1000))
			{
				strcpy_P(text, PSTR("TOO HIGH!"));
				flashing = true;
			}
		}
	}

	widget_value_txt(text, x, y + lh, w, h - lh);
	if ( flashing )
	{
		disp.Invert(x, y, x+w-2, y+h-2);
	}
}

void widget_airspace_angle_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("CTR Exit"), x, y);

	if (fc.airspace.angle == AIRSPACE_INVALID)
	{
		char tmp[5];
		strcpy_P(tmp, PSTR("---"));
		widget_value_int(tmp, x, y + lh, w, h - lh);
	}
	else
	{
		int16_t relative_direction;

		relative_direction = fc.airspace.angle	- fc.compass.azimuth_filtered;
		widget_arrow(relative_direction, x, y, w, h);
	}
	widget_invert_if_forbidden(x, y, w, h);
}

register_widget1(w_airspace_distance, "CTR Distance", widget_airspace_distance_draw);
register_widget1(w_airspace_angle,    "CTR Arrow",    widget_airspace_angle_draw);
register_widget1(w_airspace_height,   "CTR Height",   widget_airspace_max_height_draw);

