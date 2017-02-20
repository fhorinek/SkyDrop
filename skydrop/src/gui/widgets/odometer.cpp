/*
 * odometer.cpp
 *
 *  Created on: 16.02.2017
 *      Author: tilmann@bubecks.de
 */

#include "odometer.h"

void widget_odometer_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("Odo"), x, y);

	char text[10];

	double distance = fc.odometer / 100000.0;        // cm to km
	if (config.altitude.alt1_flags & ALT_UNIT_I)
		distance *= FC_KM_TO_MILE;

	sprintf_P(text, PSTR("%.1f"), distance);

	widget_value_int(text, x, y + lh, w, h - lh);
}

void widget_ododistance_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("Home"), x, y);

	char text[10];

	if (fc.flight.state == FLIGHT_FLIGHT || fc.flight.state == FLIGHT_LAND ) {
		double distance;

		distance = gps_distance_2d(fc.gps_data.latitude, fc.gps_data.longtitude, fc.flight.autostart_lat, fc.flight.autostart_lon) / 100000.0;   // cm to km
		if (config.altitude.alt1_flags & ALT_UNIT_I)
			distance *= FC_KM_TO_MILE;

		sprintf_P(text, PSTR("%.1f"), distance);
	} else {
		sprintf_P(text, PSTR("---"));
	}

	widget_value_int(text, x, y + lh, w, h - lh);
}


void widget_odoback_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("Home"), x, y);

	y += lh / 2;

	if (fc.flight.state == FLIGHT_FLIGHT || fc.flight.state == FLIGHT_LAND ) {
		int16_t bearing_back = gps_bearing(fc.flight.autostart_lat, fc.flight.autostart_lon, fc.gps_data.latitude, fc.gps_data.longtitude );
		int16_t relative_direction = bearing_back - fc.gps_data.heading;
		while (relative_direction < 0)
			relative_direction += 360;

		widget_arrow(relative_direction, x, y, w, h);
	} else {
		char tmp[5];
		sprintf_P(tmp, PSTR("---"));
		widget_value_int(tmp, x, y + lh, w, h - lh);
	}
}

register_widget1(w_odo_meter,    "Odometer",       widget_odometer_draw);
register_widget1(w_odo_back,     "Home Arrow",     widget_odoback_draw);
register_widget1(w_odo_distance, "Home Distance",  widget_ododistance_draw);

