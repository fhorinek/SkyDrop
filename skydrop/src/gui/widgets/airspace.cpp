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
	if (fc.airspace.forbidden)
	{
		if (GUI_BLINK_TGL(1000))
		{
			disp.Invert(x, y, x+w-2, y+h-2);
		}
	}
}

void widget_airspace_distance_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("AS dist"), x, y);

	char text[10];

	if (fc.airspace.angle == AIRSPACE_INVALID)
	{
		strcpy_P(text, PSTR("---"));
	}
	else
	{
		if (fc.airspace.distance_m == AIRSPACE_TOO_FAR)
		{
			text[0] = '+';
			sprintf_distance(text + 1, 50);
		}
		else
		{
			sprintf_distance(text, fc.airspace.distance_m / 1000.0);
		}
	}

	widget_value_txt(text, x, y + lh, w, h - lh);
	widget_invert_if_forbidden(x, y, w, h);
}

void widget_airspace_limits_sprintf(char * text, uint16_t raw)
{
	if (raw == AIRSPACE_INVALID)
	{
		strcpy_P(text, PSTR("---"));
		return;
	}

	if (raw & AIR_AGL_FLAG && fc.agl.valid)
	{
		if (config.altitude.alt1_flags & ALT_UNIT_I)
			sprintf_P(text, PSTR("%u"), airspace_convert_alt_ft(raw) + fc.agl.ground_level / FC_METER_TO_FEET);
		else
			sprintf_P(text, PSTR("%u"), airspace_convert_alt_m(raw) + fc.agl.ground_level);

		return;
	}
	else if (raw & AIR_AGL_FLAG && !fc.agl.valid)
	{
		if (config.altitude.alt1_flags & ALT_UNIT_I)
			sprintf_P(text, PSTR("%uAGL"), airspace_convert_alt_ft(raw));
		else
			sprintf_P(text, PSTR("%uAGL"), airspace_convert_alt_m(raw));

		return;
	}
	else
	{
		if (config.altitude.alt1_flags & ALT_UNIT_I)
			sprintf_P(text, PSTR("%u"), airspace_convert_alt_ft(raw));
		else
			sprintf_P(text, PSTR("%u"), airspace_convert_alt_m(raw));

		return;
	}

}

void widget_airspace_limits_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("AS Limit"), x, y);
	char text1[15];
	char text2[15];

	if (fc.airspace.forbidden)
	{
		strcpy_P(text1, PSTR("Inside!"));
		widget_value_txt(text1, x, y + lh, w, h - lh);
		widget_invert_if_forbidden(x, y, w, h);
		return;
	}

	widget_airspace_limits_sprintf(text1, fc.airspace.max_alt);
	widget_airspace_limits_sprintf(text2, fc.airspace.min_alt);

	widget_value_txt2(text1, text2, x, y + lh, w, h - lh);
}

void widget_airspace_angle_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("AS near"), x, y);

	if (fc.airspace.angle == AIRSPACE_INVALID)
	{
		char tmp[5];
		strcpy_P(tmp, PSTR("---"));
		widget_value_int(tmp, x, y + lh, w, h - lh);
	}
	else
	{
		int16_t relative_direction;

		relative_direction = fc.airspace.angle - fc.compass.azimuth_filtered;
		widget_arrow(relative_direction, x, y, w, h);
	}

	widget_invert_if_forbidden(x, y, w, h);
}

void widget_airspace_info_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("AS Info"), x, y);
	char text1[15];
	char text2[15];

	if (fc.airspace.angle == AIRSPACE_INVALID)
	{
		strcpy_P(text1, PSTR("---"));
		widget_value_txt(text1, x, y + lh, w, h - lh);
	}
	else
	{
		if (config.altitude.alt1_flags & ALT_UNIT_I)
		{
			sprintf_P(text1, PSTR("%u%c"), airspace_convert_alt_ft(fc.airspace.ceiling), (fc.airspace.ceiling & AIR_AGL_FLAG) ? 'A' : 'M');
			sprintf_P(text2, PSTR("%u%c"), airspace_convert_alt_ft(fc.airspace.floor), (fc.airspace.floor & AIR_AGL_FLAG) ? 'A' : 'M');
		}
		else
		{
			sprintf_P(text1, PSTR("%u%c"), airspace_convert_alt_m(fc.airspace.ceiling), (fc.airspace.ceiling & AIR_AGL_FLAG) ? 'A' : 'M');
			sprintf_P(text2, PSTR("%u%c"), airspace_convert_alt_m(fc.airspace.floor), (fc.airspace.floor & AIR_AGL_FLAG) ? 'A' : 'M');
		}

		widget_value_txt2(text1, text2, x, y + lh, w, h - lh);
	}

	widget_invert_if_forbidden(x, y, w, h);
}

void widget_airspace_name_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("AS Name"), x, y);

	if (fc.airspace.angle == AIRSPACE_INVALID)
	{
		char text[4];
		strcpy_P(text, PSTR("---"));
		widget_value_txt(text, x, y + lh, w, h - lh);
	}
	else
	{
		widget_value_scroll((char *)fc.airspace.airspace_name, x, y + lh, w, h - lh);
	}

	widget_invert_if_forbidden(x, y, w, h);
}

register_widget1(w_airspace_distance, "Airspace Distance", widget_airspace_distance_draw);
register_widget1(w_airspace_angle,    "Airspace Arrow",    widget_airspace_angle_draw);
register_widget1(w_airspace_limits,   "Airspace Limits",   widget_airspace_limits_draw);
register_widget1(w_airspace_info,     "Airspace Info",     widget_airspace_info_draw);
register_widget1(w_airspace_name,     "Airspace Name",     widget_airspace_name_draw);
