/*
 * odometer.cpp
 *
 *  Created on: 16.02.2017
 *      Author: tilmann@bubecks.de
 */

#include "odometer.h"

const uint8_t PROGMEM img_home[] = {
		5, 8, // width, heigth
		0x1C, 0x02, 0x19, 0x02, 0x1C };

const uint8_t PROGMEM img_time[] = {
		5, 8, // width, heigth
		0x0E, 0x11, 0x17, 0x15, 0x0E };

const uint8_t PROGMEM img_distance[] = {
		9, 8, // width, heigth
		0x04, 0x0E, 0x1F, 0x04, 0x04, 0x04, 0x1F, 0x0E,
		0x04 };

/**
 * Format a distance in a human readable format.
 *
 * @param text the text buffer to print into.
 * @param distance the distance in km.
 */
void sprintf_distance(char *text, float distance)
{
	const char *unit_P;

	if (config.altitude.alt1_flags & ALT_UNIT_I) {
		distance *= FC_KM_TO_MILE;
		unit_P = PSTR("mi");
	} else {
		unit_P = PSTR("km");
	}

	if ( distance < 100.0) {
		sprintf_P(text, PSTR("%.1f %S"), distance, unit_P);
	} else {
		sprintf_P(text, PSTR("%.0f %S"), distance, unit_P);
	}
}

void widget_odometer_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("Odo"), x, y);

	char text[10];

	float distance = fc.odometer / 100000.0;        // cm to km
	sprintf_distance(text, distance);

	widget_value_txt(text, x, y + lh, w, h - lh);
}

void widget_odometer_irqh(uint8_t type, uint8_t * buff, uint8_t index)
{
	if (type == B_MIDDLE && *buff == BE_LONG)
	{
		if (fc.odometer > 0)
		{
			gui_showmessage_P(PSTR("Odometer reset"));
			fc.odometer = 0;
		}
	}
}

void widget_ododistance_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	const char *Home_P = PSTR("Home");

	uint8_t lh = widget_label_P(Home_P, x, y);
	if (lh > 0)
		disp.DrawImage(img_distance, x + 1 + disp.GetTextWidth_P(Home_P) + 2, y);

	char text[10];

	if (fc.flight.home_valid && fc.gps_data.valid)
	{
		float distance = fc.flight.home_distance;
		sprintf_distance(text, distance);
	}
	else
	{
		sprintf_P(text, PSTR("---"));
	}

	widget_value_txt(text, x, y + lh, w, h - lh);
}

void widget_home_time_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	const char *Home_P = PSTR("Home");

	uint8_t lh = widget_label_P(Home_P, x, y);
	if (lh > 0)
		disp.DrawImage(img_time, x + 1 + disp.GetTextWidth_P(Home_P) + 2, y);

	char text[10];

	int16_t relative_direction = fc.flight.home_bearing - fc.gps_data.heading;

	if (fc.flight.home_valid && fc.gps_data.valid && abs(relative_direction) < 45)
	{
		// Pilot is heading towards home
		float min;

		// distance is in km, ground_speed in knots. This gives seconds, but we need minutes:
		min = ((fc.flight.home_distance * 1000.0) / (fc.gps_data.groud_speed * FC_KNOTS_TO_MPS)) / 60.0;
		sprintf_P(text, PSTR("%.0f min"), min);
	}
	else
	{
		strcpy_P(text, PSTR("---"));
	}

	widget_value_txt(text, x, y + lh, w, h - lh);
}

void widget_odoback_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	const char *Home_P = PSTR("Home");

	uint8_t lh = widget_label_P(Home_P, x, y);

	y += lh / 2;

	if (fc.flight.home_valid && fc.gps_data.valid)
	{
		int16_t relative_direction = fc.flight.home_bearing - fc.gps_data.heading;
		widget_arrow(relative_direction, x, y, w, h);
	}
	else
	{
		char tmp[5];
		strcpy_P(tmp, PSTR("---"));
		widget_value_int(tmp, x, y + lh, w, h - lh);
	}
}

register_widget2(w_odo_meter, "Odometer", widget_odometer_draw, 0, widget_odometer_irqh);
register_widget1(w_odo_home_direction, "Home Arrow", widget_odoback_draw);
register_widget1(w_odo_home_distance, "Home Distance", widget_ododistance_draw);
register_widget1(w_odo_home_time, "Home Time", widget_home_time_draw);

