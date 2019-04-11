/*
 * odometer.cpp
 *
 *  Created on: 16.02.2017
 *      Author: tilmann@bubecks.de
 */

#include "odometer.h"
#include "../../fc/waypoint.h"
#include "../gui_list.h"

#include <limits.h>

const uint8_t PROGMEM img_home[] =
	{5, 8, // width, heigth
		0x1C, 0x02, 0x19, 0x02, 0x1C };

const uint8_t PROGMEM img_time[] =
	{5, 8, // width, heigth
		0x0E, 0x11, 0x17, 0x15, 0x0E };

const uint8_t PROGMEM img_distance[] =
	{9, 8, // width, heigth
		0x04, 0x0E, 0x1F, 0x04, 0x04, 0x04, 0x1F, 0x0E, 0x04 };

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

void widget_distance_draw(const char *label_P, float distance, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(label_P, x, y);
	if (lh > 0)
		disp.DrawImage(img_distance, x + 1 + disp.GetTextWidth_P(label_P) + 2, y);

	char text[10];

	if (distance != INFINITY)
	{
		sprintf_distance(text, distance);
	}
	else
	{
		sprintf_P(text, PSTR("---"));
	}

	widget_value_txt(text, x, y + lh, w, h - lh);
}

void widget_ododistance_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	float distance;

	if (fc.flight.home_valid && fc.gps_data.valid)
	{
		distance = fc.flight.home_distance;
	}
	else
	{
		distance = INFINITY;
	}

	widget_distance_draw(PSTR("Home"), distance, x, y, w, h, flags);
}

void widget_waypoint_distance_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	float distance;

	if (fc.flight.next_waypoint.radius_m != 0 && fc.gps_data.valid && fc.flight.waypoint_no)
	{
		distance = fc.flight.next_waypoint.distance;
	}
	else
	{
		distance = INFINITY;
	}

	widget_distance_draw(PSTR("WP"), distance, x, y, w, h, flags);
}

void widget_time_draw(const char *label_P, float min, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(label_P, x, y);
	if (lh > 0)
		disp.DrawImage(img_time, x + 1 + disp.GetTextWidth_P(label_P) + 2, y);

	char text[10];

	if (min != INFINITY)
	{
		sprintf_P(text, PSTR("%.0f min"), min);
	}
	else
	{
		strcpy_P(text, PSTR("---"));
	}

	widget_value_txt(text, x, y + lh, w, h - lh);
}

void widget_home_time_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	float min = INFINITY;

	if (fc.flight.home_valid && fc.gps_data.valid) {
		//this must be GPS heading not compass, since we have to move towards the target, not just pointing to it!
		int16_t relative_direction = fc.flight.home_bearing - fc.gps_data.heading;
		if (abs(relative_direction) < 45) {
			// Pilot is heading towards home.
			// distance is in km, ground_speed in knots. This gives seconds, but we need minutes:
			min = ((fc.flight.home_distance * 1000.0) / (fc.gps_data.groud_speed * FC_KNOTS_TO_MPS)) / 60.0;
		}
	}

	widget_time_draw(PSTR("Home"), min, x, y, w, h ,flags);
}

void widget_waypoint_time_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	float min = INFINITY;

	if (fc.flight.next_waypoint.radius_m != 0 && fc.gps_data.valid) {
		//this must be GPS heading not compass, since we have to move towards the target, not just pointing to it!
		int16_t relative_direction = fc.flight.next_waypoint.bearing - fc.gps_data.heading;
		if (abs(relative_direction) < 45) {
			// Pilot is heading towards home.
			// distance is in km, ground_speed in knots. This gives seconds, but we need minutes:
			min = ((fc.flight.next_waypoint.distance * 1000.0) / (fc.gps_data.groud_speed * FC_KNOTS_TO_MPS)) / 60.0;
		}
	}

	widget_time_draw(PSTR("WP"), min, x, y, w, h ,flags);
}

void widget_home_info_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint32_t oldClip;

	oldClip = disp.clip(x, y, x + w, y + h);

	const char *Home_P = PSTR("Home info");

	uint8_t lh = widget_label_P(Home_P, x, y);

	disp.LoadFont(F_TEXT_M);

	uint8_t text_h = disp.GetTextHeight();

	char tmp[80];

	if (config.home.flags & HOME_SET_AS_TAKEOFF)
	{
		sprintf_P(tmp, PSTR("Take-off"));
		widget_value_txt(tmp, x, y + lh, w, h - lh);
	}
	else
	{
		if (config.home.flags & HOME_LOADED_FROM_SD)
		{
			y += lh + 1;

			if (config.home.name[0])
			{
				disp.GotoXY(x, y);
				fputs((const char *) config.home.name, lcd_out);
				y += text_h + 1;
			}

			if (config.home.freq[0])
			{
				disp.GotoXY(x, y);
				fprintf_P(lcd_out, PSTR("Freq: %s"), config.home.freq);
				y += text_h + 1;
			}

			if (config.home.rwy[0])
			{
				disp.GotoXY(x, y);
				sprintf_P(tmp, PSTR("Rwy: %s, %s"), config.home.rwy, config.home.traffic_pattern);
				widget_value_scroll(tmp, x, y, w, h);
				y += text_h + 1;
			}

			if (config.home.info[0])
			{
				disp.GotoXY(x, y);
				widget_value_scroll((char *) config.home.info, x, y, w, h);
			}
		}
		else //no home loaded
		{
			char text[7];
			sprintf_P(text, PSTR("Load"));

			widget_value_txt(text, x, y + lh, w, h - lh);
		}
	}


	disp.clip(oldClip);
}

void widget_home_info_irqh(uint8_t type, uint8_t * buff, uint8_t index)
{
	if (type == B_MIDDLE && *buff == BE_LONG)
	{
		gui_switch_task(GUI_HOME);
		gui_list_set_index(GUI_HOME, 2);
	}
}

void widget_waypoint_info_irqh(uint8_t type, uint8_t * buff, uint8_t index)
{
	if (type == TASK_IRQ_BUTTON_L || type == TASK_IRQ_BUTTON_R)
	{
		if (*buff == BE_CLICK || *buff == BE_DBL_CLICK)
		{
			if (type == TASK_IRQ_BUTTON_L)
			{
				waypoint_goto_prev();
			}
			else
			{
				waypoint_goto_next();
			}
		}
	}

	if (type == B_MIDDLE && *buff == BE_LONG)
	{
		gui_switch_task(GUI_HOME);
		gui_list_set_index(GUI_HOME, 4);
	}
}

void widget_waypoint_info_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint32_t oldClip;

	oldClip = disp.clip(x, y, x + w, y + h);

	const char *Home_P = PSTR("WP info");

	uint8_t lh = widget_label_P(Home_P, x, y);

	disp.LoadFont(F_TEXT_M);

	if (fc.flight.waypoints_count)
	{
		if (fc.flight.waypoint_no)
		{
			char text1[16];
			char text2[16];

			sprintf_P(text1, PSTR("%u/%u"), fc.flight.waypoint_no, fc.flight.waypoints_count);
			sprintf_P(text2, PSTR("%s"), fc.flight.next_waypoint.name);

			widget_value_int_sub(text1, text2, x, y + lh, w, h - lh);
		}
		else
		{
			char text[6];
			sprintf_P(text, PSTR("Done!"));

			widget_value_txt(text, x, y + lh, w, h - lh);
		}

	}
	else
	{
		char text[7];
		sprintf_P(text, PSTR("Load"));

		widget_value_txt(text, x, y + lh, w, h - lh);
	}


	disp.clip(oldClip);
}

void widget_direction_draw(const char *label_P, int16_t direction, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(label_P, x, y);

	if (direction != INT_MAX)
	{
		y += lh / 2;

		widget_arrow(direction, x, y, w, h);
	}
	else
	{
		char tmp[5];
		strcpy_P(tmp, PSTR("---"));
		widget_value_int(tmp, x, y + lh, w, h - lh);
	}
}

void widget_odoback_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	int16_t relative_direction;

	if (fc.flight.home_valid && fc.gps_data.valid)
	{
		relative_direction = fc.flight.home_bearing	- fc.compass.azimuth_filtered;
	}
	else
	{
		relative_direction = INT_MAX;
	}

	widget_direction_draw(PSTR("Home"), relative_direction, x, y, w, h, flags);
}

void widget_waypoint_direction_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	int16_t relative_direction;

	if (fc.flight.waypoint_no != 0 && fc.gps_data.valid)
	{
		relative_direction = fc.flight.next_waypoint.bearing - fc.compass.azimuth_filtered;
	}
	else
	{
		relative_direction = INT_MAX;
	}
	widget_direction_draw(PSTR("WP"), relative_direction, x, y, w, h, flags);
}

register_widget2(w_odo_meter, "Odometer", widget_odometer_draw, 0, widget_odometer_irqh);
register_widget1(w_odo_home_direction, "Home Arrow", widget_odoback_draw);
register_widget1(w_odo_home_distance, "Home Distance", widget_ododistance_draw);
register_widget1(w_odo_home_time, "Home Time", widget_home_time_draw);

register_widget2(w_home_info, "Home Info", widget_home_info_draw, 0, widget_home_info_irqh);

register_widget1(w_waypoint_direction, "Waypoint Arrow", widget_waypoint_direction_draw);
register_widget1(w_waypoint_distance, "Waypoint Distance", widget_waypoint_distance_draw);
register_widget1(w_waypoint_time, "Waypoint Time", widget_waypoint_time_draw);
register_widget2(w_waypoint_info, "Waypoint Info", widget_waypoint_info_draw, 0, widget_waypoint_info_irqh);

