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

	if (waypoint_task_active() && fc.gps_data.valid)
	{
		distance = fc.task.next_waypoint.distance;
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
			min = ((fc.flight.home_distance * 1000.0) / (fc.gps_data.ground_speed * FC_KNOTS_TO_MPS)) / 60.0;
		}
	}

	widget_time_draw(PSTR("Home"), min, x, y, w, h ,flags);
}

void widget_waypoint_time_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	float min = INFINITY;

	if (waypoint_task_active() && fc.gps_data.valid) {
		//this must be GPS heading not compass, since we have to move towards the target, not just pointing to it!
		int16_t relative_direction = fc.task.next_waypoint.bearing - fc.gps_data.heading;
		if (abs(relative_direction) < 45) {
			// Pilot is heading towards home.
			// distance is in km, ground_speed in knots. This gives seconds, but we need minutes:
			min = ((fc.task.next_waypoint.distance * 1000.0) / (fc.gps_data.ground_speed * FC_KNOTS_TO_MPS)) / 60.0;
		}
	}

	widget_time_draw(PSTR("WP"), min, x, y, w, h ,flags);
}

void widget_home_info_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("Home"), x, y);

	char text[21];
	//strings are not always n null term
	text[20] = 0;

	if (config.home.flags & HOME_TAKEOFF)
	{
		strcpy_P(text, PSTR("Take off"));
	}
	else if(config.home.flags  & HOME_LOADED)
	{
		strcpy(text, (char *)config.home.name);
	}
	else
	{
		strcpy_P(text, PSTR("Load"));
	}

	widget_value_txt(text, x, y + lh, w, h - lh);
}

void widget_home_info_irqh(uint8_t type, uint8_t * buff, uint8_t index)
{
	if (type == B_MIDDLE && *buff == BE_LONG)
	{
		gui_switch_task(GUI_NAVIGATION);
		gui_list_set_index(GUI_NAVIGATION, 4);
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
		gui_switch_task(GUI_NAVIGATION);
		gui_list_set_index(GUI_NAVIGATION, 1);
	}
}

void widget_waypoint_info_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint32_t oldClip;

	oldClip = disp.clip(x, y, x + w, y + h);

	const char *Home_P = PSTR("WP info");

	uint8_t lh = widget_label_P(Home_P, x, y);

	disp.LoadFont(F_TEXT_M);

	if (fc.task.active)
	{
		if (fc.task.waypoint_index < fc.task.waypoint_count - 1)
		{
			char text1[16];
			char text2[16];

			sprintf_P(text1, PSTR("%u/%u"), fc.task.waypoint_index, fc.task.waypoint_count);
			sprintf_P(text2, PSTR("%s"), fc.task.next_waypoint.twpt.wpt.name);

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

	if (waypoint_task_active() && fc.gps_data.valid)
	{
		relative_direction = fc.task.next_waypoint.bearing - fc.compass.azimuth_filtered;
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

