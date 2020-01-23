/*
 * airspace.cpp
 *
 *  Created on: 02.12.2018
 *      Author: tilmann@bubecks.de
 */

#include <math.h>

#include "airspace.h"
#include "../../fc/airspace.h"
#include "../../fc/agl.h"

const uint8_t PROGMEM img_glider[] =
	{ 8, 8, // width, heigth
	  0x04, 0x06, 0x03, 0xF1, 0xF1, 0x03, 0x06, 0x04 };

// Todo: share with odometer.cpp
const uint8_t PROGMEM img_home[] =
	{5, 8, // width, heigth
	 0x1C, 0x02, 0x19, 0x02, 0x1C };

const uint8_t PROGMEM img_up[] = 
	{8, 8, // width, heigth
	 0x08, 0x0C, 0xFE, 0xFF, 0xFF, 0xFE, 0x0C, 0x08 };

const uint8_t PROGMEM img_down[] =
	{8, 8, // width, heigth
	 0x10, 0x30, 0x7F, 0xFF, 0xFF, 0x7F, 0x30, 0x10 };

void sprintf_height_in_ft(char *text, int32_t height_ft)
{
	if (config.altitude.alt1_flags & ALT_UNIT_I)
		sprintf_P(text, PSTR("%ld"), height_ft);
	else
		sprintf_P(text, PSTR("%ld"), (int32_t)(height_ft / FC_METER_TO_FEET));
}

void widget_airspace_vertical_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	char text1[15];
	uint16_t max_ft;        // The absolute value in feet, which is maximum allowed
	uint16_t min_ft;        // The absolute value in feet, which is minimum allowed
	int above_ft;    		// How many feet is allowed above the glider?
	int below_ft;    		// How many feet is allowed below the glider?
	uint8_t text_hs;        // The height in pixel of the values to be shown
	int y_top;     			// The first pixel line of the airspace.
	int y_bottom;           // The last pixel line of the airspace
	int pixel_height;       // The height of the airspace in pixel on screen
	int x_right;     		// The last pixel on the right side of the widget
	int x_right_pos;        // We draw from right to left and store here the current position
	int glider_y;           // The lowest pixel of the glider on screen
	int width;

	static float glider_alt_m = 0;

	// Only show label if widget is wide enough
	if ( w > 60)
		widget_label_P(PSTR("AS Vert"), x, y + 1);

	disp.LoadFont(F_TEXT_M);
	text_hs = disp.GetTextHeight();

	x_right      = x + w - 2;     	// The last pixel on the right side of the widget
	x_right_pos  = x_right;    		// We draw from right to left and store here the current position

	y_top        = y + text_hs;     			// The first pixel line of the airspace.
	y_bottom     = y + h - text_hs - 1;         // Calculate position on screen, where allowed airspace ends
	pixel_height = y_bottom - y_top + 1;    	// How many pixels is airspace on screen?
	glider_y     = y_top + pixel_height / 2;    // Default position of glider in the middle

#define SIM_ALTITUDE
#ifdef SIM_ALTITUDE
	static uint32_t next_anim = 0;
	if ( task_get_ms_tick() > next_anim ) {
		next_anim = task_get_ms_tick() + 5000;
		if ( glider_alt_m > 1300 ) glider_alt_m = 0;
		glider_alt_m += 100;
	}
#else
	glider_alt_m = fc.altitude1;
#endif

	DEBUG("\nglider_alt_m=%f ft, %f m\n", glider_alt_m * FC_METER_TO_FEET, glider_alt_m);
	DEBUG("fc.agl.ground_level=%d m\n", fc.agl.ground_level);
	DEBUG("fc.airspace.max_alt=0x%x, %d ft, %d m\n", fc.airspace.max_alt, airspace_convert_alt_ft(fc.airspace.max_alt), airspace_convert_alt_m(fc.airspace.max_alt));
	DEBUG("fc.airspace.min_alt=0x%x, %d ft, %d m\n", fc.airspace.min_alt, airspace_convert_alt_ft(fc.airspace.min_alt), airspace_convert_alt_m(fc.airspace.min_alt));

	// [1] Calculate absolute value of upper airspace (max_ft) and how much feet are
	//     left above glider (above_ft).
	if (fc.airspace.max_alt == AIRSPACE_INVALID ||
			((fc.airspace.max_alt & AIR_AGL_FLAG) && fc.agl.ground_level == AGL_INVALID) )
	{
		// We do not have a maximum altitude (or it is based on a AGL value which is not available).
		// => Show nothing "above" us. No glider as well.
		widget_value_textr("---", x_right, y + 1);
		max_ft = AIRSPACE_INVALID;
	}
	else
	{
		// We have a valid maximum altitude (either AGL or MSL)
		if (fc.airspace.max_alt & AIR_AGL_FLAG)
			max_ft = airspace_convert_alt_ft(fc.airspace.max_alt) + fc.agl.ground_level * FC_METER_TO_FEET;
		else
			max_ft = fc.airspace.max_alt;

		// max_ft = widget_airspace_limits_sprintf(text1, fc.airspace.max_alt);
		sprintf_height_in_ft(text1, max_ft);
		widget_value_textr(text1, x_right, y + 1);

		above_ft = max_ft - glider_alt_m * FC_METER_TO_FEET;
		DEBUG("above_ft=%d ft, %d m\n", above_ft, (int)(above_ft / FC_METER_TO_FEET));
		sprintf_height_in_ft(text1, above_ft);
		width = widget_value_textr(text1, x_right, y_top + 1);
		x_right_pos = x_right - width;
	}
	disp.Invert(x, y, x_right, y_top - 1);    	// Draw the upper airspace

	// [2] Calculate absolute value of lower airspace (min_ft) and how much feet are
	//     left below glider (below_ft).
	if (fc.airspace.min_alt == AIRSPACE_INVALID)
	{
		// There is no airspace below us. We are flying above ground.
		if (fc.agl.ground_level != AGL_INVALID)
			min_ft = fc.agl.ground_level * FC_METER_TO_FEET;
		else
			min_ft = AIRSPACE_INVALID;
	}
	else
	{
		// We are flying above a airspace. Try to find its absolute minimum (in ft).
		if (fc.airspace.min_alt & AIR_AGL_FLAG)
			// This is a value AGL
			if (fc.agl.ground_level != AGL_INVALID)
				min_ft = fc.agl.ground_level * FC_METER_TO_FEET + airspace_convert_alt_ft(fc.airspace.min_alt);
			else
				min_ft = AIRSPACE_INVALID;
		else
			// This is MSL value
			min_ft = airspace_convert_alt_ft(fc.airspace.min_alt);
	}

	debug_flush();

	DEBUG("max_ft=0x%x, %d ft, %d m\n", max_ft, max_ft, (int)(max_ft / FC_METER_TO_FEET));
	DEBUG("min_ft=0x%x, %d ft, %d m\n", min_ft, min_ft, (int)(min_ft / FC_METER_TO_FEET));

	DEBUG("y_top=%d, y_bottom=%d\n", y_top, y_bottom);
	DEBUG("pixel_height=%d, glider_y(default,middle)=%d\n", pixel_height, glider_y);

	debug_flush();

	if (min_ft != AIRSPACE_INVALID)
	{
		// Show absolute minimum (min_ft)
		sprintf_height_in_ft(text1, min_ft);
		widget_value_textr(text1, x_right, y_bottom + 2);

		// Calculate and show number of feet from glider to minimum
		below_ft = glider_alt_m * FC_METER_TO_FEET - min_ft;
		DEBUG("below_ft=%d\n", below_ft);
		sprintf_height_in_ft(text1, below_ft);
		width = widget_value_textr(text1, x_right, y_bottom - text_hs + 2);

		// Find leftmost right position
		x_right_pos = min(x_right_pos, x_right - width);

		if (max_ft != AIRSPACE_INVALID )
		{
			// Calculate height of airspace and relative position of glider on screen
			int airspace_height_ft = max_ft - min_ft;
			glider_y = y_bottom - round((glider_alt_m * FC_METER_TO_FEET - min_ft) / airspace_height_ft * (pixel_height - img_glider[1]));
			DEBUG("airspace_height_ft=%d ft, %d m\n", airspace_height_ft, (int)(airspace_height_ft / FC_METER_TO_FEET));
			DEBUG("glider_y=%d\n", glider_y);
		}
	}
	disp.Invert(x, y + h, x_right, y_bottom + 1);	// Draw the lower airspace

	// Ensure, that glider stays within widget
	if (glider_y >= y + h) glider_y = y + h;
	if (glider_y <= y + 5) glider_y = y + 5;

	int x_left = x_right_pos - 4 - 6 - img_glider[0] - 25;
	if ( x_left < x ) x_left = x;

	if (fc.airspace.min_alt == AIRSPACE_INVALID)
	{
		// There is no airspace below us. We are flying above ground.
		// Draw houses on ground. :)
		disp.DrawImage(img_home, x_left + 3, y_bottom - 4);
		disp.DrawImage(img_home, x_left + 12, y_bottom - 4);
	}

	// Clear area of glider, so that it is also visible, when entering top or bottom airspace:
	disp.DrawRectangle(x_left, glider_y - 7, x_right_pos, glider_y, DISP_COLOR_WHITE, true);

	x_right_pos = x_right_pos - 4 - img_glider[0];
	disp.DrawImage(img_glider, x_right_pos, glider_y - img_glider[1] + 1);
	x_right_pos = x_right_pos - 6;

	// Show arrow up/down if we are too high/low
	if (GUI_BLINK_TGL(1000))
	{
		if (above_ft < 0)
			disp.DrawImage(img_down, x_right_pos, y + h/2 - 4);
		if (below_ft < 0)
			disp.DrawImage(img_up, x_right_pos, y + h/2 - 4);
	}

	if ( x_right_pos > x + 20 )
	{
		// If there is enough width, then also draw height of glider
		DEBUG("glider_y line=%d\n", glider_y - img_glider[1] / 2);
		disp.DrawLine(x_right_pos, glider_y - img_glider[1] / 2 + 1, x_right_pos + 4, glider_y - img_glider[1] / 2 + 1);
		x_right_pos = x_right_pos - 1;

		sprintf_height_in_ft(text1, (int32_t)(glider_alt_m * FC_METER_TO_FEET));
		width = widget_value_textr(text1, x_right_pos, glider_y - text_hs + 2);

#define SIZE_LEFT 15
		if ( x_right_pos > x + SIZE_LEFT )
		{
			// If there is enough width, then also draw airspace arrow and distance:
			if (fc.airspace.angle != AIRSPACE_INVALID)
			{
				disp.DrawLine(SIZE_LEFT, y_top, SIZE_LEFT, y_bottom);

				int16_t relative_direction;

				relative_direction = fc.airspace.angle - fc.compass.azimuth_filtered;
				widget_arrow(relative_direction, x, y_top + 1, SIZE_LEFT, SIZE_LEFT);

				if (fc.airspace.distance_m == AIRSPACE_TOO_FAR)
				{
					text1[0] = '+';
					sprintf_distance(text1 + 1, 50);
				}
				else
				{
					sprintf_distance(text1, fc.airspace.distance_m / 1000.0);
				}
				widget_value_txt(text1, x, y_top + SIZE_LEFT +2, SIZE_LEFT, SIZE_LEFT);
			}
		}
	}
}

void widget_airspace_info_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
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
		sprintf_distance(text1, fc.airspace.distance_m / 1000.0);
		if (config.connectivity.gps_format_flags & GPS_DIST_UNIT_I)
			strcpy_P(text2, PSTR("mi"));
		else
			strcpy_P(text2, PSTR("km"));

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
register_widget1(w_airspace_vertical, "Airspace Vertical", widget_airspace_vertical_draw);
