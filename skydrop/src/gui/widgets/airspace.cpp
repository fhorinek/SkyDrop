/*
 * airspace.cpp
 *
 *  Created on: 02.12.2018
 *      Author: tilmann@bubecks.de
 */

#include "airspace.h"
#include "../../fc/airspace.h"
#include "../../fc/agl.h"

void widget_airspace_info_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	char text1[12];
	char text2[12];

	if (x != 0 || y != 0 || w < 78 || h < 40)
	{
		disp.DrawLine(x, y, x + w, y + h, 1);
		disp.DrawLine(x, y + h, x + w, y, 1);

		return;
	}

	if (!fc.airspace.file_valid)
	{
		char text[8];
		strcpy_P(text, PSTR("No data"));
		widget_value_txt(text, x, y, w, h);

		return;
	}

	if (fc.airspace.airspace_index == AIR_INDEX_INVALID)
	{
		char text[16];
		strcpy_P(text, PSTR("No restriction"));
		widget_value_txt(text, x, y, w, h);

		return;
	}

	uint8_t y_spc = h - 40;

	//Name
	widget_value_scroll((char *)fc.airspace.airspace_name, 0, 0, w, 12);
	//Class
	airspace_class_to_text(text1, fc.airspace.airspace_class);

	disp.LoadFont(F_LABEL);
	uint8_t f_h = disp.GetAHeight();
	gui_raligh_text(text1, w - 2, 10 + y_spc / 4);

	disp.GotoXY(10, 12);
	if (fc.airspace.inside)
	{
		strcpy_P(text1, PSTR("INSIDE!"));
		fprintf(lcd_out, text1);

		if (GUI_BLINK_TGL(500))
		{
			uint8_t f_w = disp.GetTextWidth(text1);
			disp.Invert(8, 10, 10 + f_w, 13 + f_h);
		}
	}
	else if (fc.airspace.distance_m <= config.airspaces.warning_m && config.airspaces.warning_m > 0)
	{
		strcpy_P(text1, PSTR("NEAR!"));
		fprintf(lcd_out, text1);
		if (GUI_BLINK_TGL(1500))
		{
			uint8_t f_w = disp.GetTextWidth(text1);
			disp.Invert(8, 10, 10 + f_w, 13 + f_h);
		}
	}
	else
	{
		strcpy_P(text1, PSTR("Outside"));
		fprintf(lcd_out, text1);
	}

	if (GUI_BLINK_TGL(5000))
	{
		//Direction
		int16_t relative_direction;
		relative_direction = fc.airspace.angle - fc.compass.azimuth_filtered;
		widget_arrow(relative_direction, 0, 18 + y_spc / 2, 22, 20);

		//Distance
		sprintf_distance(text1, text2, fc.airspace.distance_m / 1000.0);
		widget_value_int_sub(text1, text2, w - 60, 18 + y_spc / 2, 20, 20);
	}
	else
	{
		widget_alt_draw(16, 20 + y_spc / 4, 20, 20, 1);

		/*
		if (fc.agl.ground_level != AGL_INVALID || !(fc.airspace.floor & AIR_AGL_FLAG || fc.airspace.ceiling & AIR_AGL_FLAG))
		{
			uint8_t line_x = w - 39;
			uint8_t line_y = 17 + y_spc / 4;
			uint8_t line_h = line_y - h - 2;
			disp.DrawLine(line_x, line_y, line_x, line_h + line_y, 1);

			int16_t flr = airspace_convert_alt_m(fc.airspace.floor) + (fc.airspace.floor & AIR_AGL_FLAG) ? fc.agl.ground_level : 0;
			int16_t cei = airspace_convert_alt_m(fc.airspace.ceiling) + (fc.airspace.ceiling & AIR_AGL_FLAG) ? fc.agl.ground_level : 0;

			int16_t delta = cei - flr;

			widget_arrow(90, 0, 18 + y_spc / 2, 20, 20);
		}
		*/
	}

	//Floor & ceil
	disp.LoadFont(F_TEXT_S);
	f_h = disp.GetAHeight();

	const char * msl = PSTR("Msl");
	const char * agl = PSTR("Agl");

	if (config.altitude.alt1_flags & ALT_UNIT_I)
	{
		sprintf_P(text1, PSTR("%u %S"), airspace_convert_alt_ft(fc.airspace.ceiling), (fc.airspace.ceiling & AIR_AGL_FLAG) ? agl : msl);
		sprintf_P(text2, PSTR("%u %S"), airspace_convert_alt_ft(fc.airspace.floor), (fc.airspace.floor & AIR_AGL_FLAG) ? agl : msl);
	}
	else
	{
		sprintf_P(text1, PSTR("%u %S"), airspace_convert_alt_m(fc.airspace.ceiling), (fc.airspace.ceiling & AIR_AGL_FLAG) ? agl : msl);
		sprintf_P(text2, PSTR("%u %S"), airspace_convert_alt_m(fc.airspace.floor), (fc.airspace.floor & AIR_AGL_FLAG) ? agl : msl);
	}

	disp.GotoXY(w - 36, 17 + y_spc / 4);
	fprintf_P(lcd_out, PSTR("Ceiling"));
	disp.GotoXY(w - 32, 23 + y_spc / 4);
	fprintf(lcd_out, text1);

	disp.GotoXY(w - 36, 29 + y_spc / 2);
	fprintf_P(lcd_out, PSTR("Floor"));
	disp.GotoXY(w- 32, 35 + y_spc / 2);
	fprintf(lcd_out, text2);
}

register_widget1(w_airspace_info, "Airspace Info", widget_airspace_info_draw);
