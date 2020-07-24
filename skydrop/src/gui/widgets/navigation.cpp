/*
 * odometer.cpp
 *
 *  Created on: 16.02.2017
 *      Author: tilmann@bubecks.de
 */

#include "navigation.h"
#include "../../fc/waypoint.h"
#include "../gui_list.h"

#include <limits.h>

#define DISABLE_ICONS

#ifndef DISABLE_ICONS
const uint8_t PROGMEM img_home[] =
	{5, 8, // width, heigth
		0x1C, 0x02, 0x19, 0x02, 0x1C };

const uint8_t PROGMEM img_time[] =
	{5, 8, // width, heigth
		0x0E, 0x11, 0x17, 0x15, 0x0E };

const uint8_t PROGMEM img_distance[] =
	{9, 8, // width, heigth
		0x04, 0x0E, 0x1F, 0x04, 0x04, 0x04, 0x1F, 0x0E, 0x04 };
#endif

#define NAV_INFO_PERIOD		10000

void widget_distance_draw(const char *label_P, float distance, uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	uint8_t lh = widget_label_P(label_P, x, y);
#ifndef DISABLE_ICONS
	if (lh > 0)
		disp.DrawImage(img_distance, x + 1 + disp.GetTextWidth_P(label_P) + 2, y);
#endif

	char text1[12];
	char text2[12];

	sprintf_distance(text1, text2, distance);
	widget_value_txt2(text1, text2, x, y + lh, w, h - lh);
}

void widget_odometer_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
  widget_distance_draw(PSTR("Odo"), fc.odometer / 100000.0, x, y, w, h);   // cm to km
}

void widget_odometer_irqh(uint8_t type, uint8_t * buff)
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

void widget_home_distance_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
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

	widget_distance_draw(PSTR("Home"), distance, x, y, w, h);
}

void widget_waypoint_distance_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
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

	widget_distance_draw(PSTR("WP"), distance, x, y, w, h);
}

void widget_nav_time_draw(const char *label_P, uint16_t sec, uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	uint8_t lh = widget_label_P(label_P, x, y);
#ifndef DISABLE_ICONS
	if (lh > 0)
		disp.DrawImage(img_time, x + 1 + disp.GetTextWidth_P(label_P) + 2, y);
#endif

	char text[10];

	if (sec != 0xFFFF)
	{
		sprintf_P(text, PSTR("%u min"), sec / 60);
	}
	else
	{
		strcpy_P(text, PSTR("---"));
	}

	widget_value_txt(text, x, y + lh, w, h - lh);
}

void widget_home_time_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	uint16_t sec = 0xFFFF;

	if (fc.flight.home_valid && fc.gps_data.valid) {
		//this must be GPS heading not compass, since we have to move towards the target, not just pointing to it!
		int16_t relative_direction = fc.flight.home_bearing - fc.gps_data.heading;
		if (abs(relative_direction) < 45) {
			// Pilot is heading towards home.
			// distance is in km, ground_speed in knots. This gives seconds, but we need minutes:
			sec = ((fc.flight.home_distance * 1000.0) / (fc.gps_data.ground_speed * FC_KNOTS_TO_MPS));
		}
	}

	widget_nav_time_draw(PSTR("Home"), sec, x, y, w, h);
}

void widget_waypoint_time_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	uint16_t sec = 0xFFFF;

	if (waypoint_task_active() && fc.gps_data.valid) {
		//this must be GPS heading not compass, since we have to move towards the target, not just pointing to it!
		int16_t relative_direction = fc.task.next_waypoint.bearing - fc.gps_data.heading;
		if (abs(relative_direction) < 45) {
			// Pilot is heading towards home.
			// distance is in km, ground_speed in knots. This gives seconds, but we need minutes:
			sec = ((fc.task.next_waypoint.distance * 1000.0) / (fc.gps_data.ground_speed * FC_KNOTS_TO_MPS));
		}
	}

	widget_nav_time_draw(PSTR("WP"), sec, x, y, w, h);
}

void widget_home_info_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
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

void widget_home_info_irqh(uint8_t type, uint8_t * buff)
{
	if (type == B_MIDDLE && *buff == BE_LONG)
	{
		gui_switch_task(GUI_NAVIGATION);
		gui_list_set_index(GUI_NAVIGATION, 4);
	}
}

void widget_waypoint_info_irqh(uint8_t type, uint8_t * buff)
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



void widget_waypoint_info_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	const char *Home_P = PSTR("WP info");

	uint8_t lh = widget_label_P(Home_P, x, y);

	disp.LoadFont(F_TEXT_M);

	if (fc.task.active)
	{
		bool show_next = true;

		char text1[16];
		char text2[16];
		uint8_t hour;
		uint8_t min;
		uint8_t sec;
		uint32_t time;

		switch (waypoint_task_mode())
		{
			case(TASK_MODE_PREPARE):

				if (GUI_BLINK_TGL(NAV_INFO_PERIOD))
				{
					if (fc.task.head.flags & CFG_TASK_FLAGS_START_ENTER)
					{
						if (fc.task.inside_before_start)
						{

							sprintf_P(text1, PSTR("Exit"));
							sprintf_P(text2, PSTR("TP"));
							widget_value_txt2(text1, text2, x, y + lh, w, h - lh);

							show_next = false;
						}
					}
					else
					{
						if (!fc.task.inside_before_start)
						{
							sprintf_P(text1, PSTR("Enter"));
							sprintf_P(text2, PSTR("TP"));
							widget_value_txt2(text1, text2, x, y + lh, w, h - lh);
							show_next = false;
						}
					}
				}

				if (show_next)
				{
					sprintf_P(text1, PSTR("Start in"));
					time = waypoint_task_time_to_start();
					hour = time / 3600;
					min = (time % 3600) / 60;
					sec = time % 60;
					if (hour > 0)
						sprintf_P(text2, PSTR("%02u:%02u"), hour, min);
					else
						sprintf_P(text2, PSTR("%02u.%02u"), min, sec);

					widget_value_txt2(text1, text2, x, y + lh, w, h - lh);

					show_next = false;
				}
				else
				{
					//if Exit of Enter warning is present blink
					if (GUI_BLINK_TGL(1000))
					{
						disp.Invert(x, y + lh, x + w - 1, y + h - 1);

						disp.PutPixel(x, 			y + lh,   DISP_COLOR_WHITE);
						disp.PutPixel(x + w - 1, 	y + lh,   DISP_COLOR_WHITE);
						disp.PutPixel(x, 			y + h -1, DISP_COLOR_WHITE);
						disp.PutPixel(x + w - 1, 	y + h -1, DISP_COLOR_WHITE);
					}
				}


			break;

			case(TASK_MODE_ACTIVE):
				if (!(fc.task.head.deadline_hour & CFG_TASK_HOUR_DISABLED) && GUI_BLINK_TGL(NAV_INFO_PERIOD))
				{
					sprintf_P(text1, PSTR("End in"));
					time = waypoint_task_time_to_deadline();
					hour = time / 3600;
					min = (time % 3600) / 60;
					sec = time % 60;

					if (hour > 0)
						sprintf_P(text2, PSTR("%02u:%02u"), hour, min);
					else
						sprintf_P(text2, PSTR("%02u.%02u"), min, sec);

					widget_value_txt2(text1, text2, x, y + lh, w, h - lh);
					show_next = false;
				}
			break;

			case(TASK_MODE_ENDED):
				if (GUI_BLINK_TGL(NAV_INFO_PERIOD))
				{
					sprintf_P(text1, PSTR("Ended"));
					sprintf_P(text2, PSTR("%02u:%02u"), fc.task.head.deadline_hour, fc.task.head.deadline_min);
					widget_value_txt2(text1, text2, x, y + lh, w, h - lh);
					show_next = false;
				}
			break;
		}

		if (show_next)
		{

			if (fc.task.waypoint_count <= 1)
			{
				char text[6];
				sprintf_P(text, PSTR("Empty"));

				widget_value_txt(text, x, y + lh, w, h - lh);
			}
			else
			if (fc.task.waypoint_index < fc.task.waypoint_count)
			{
				char text1[16];
				char text2[16];

				sprintf_P(text1, PSTR("%u/%u"), fc.task.waypoint_index, fc.task.waypoint_count - 1);
				sprintf_P(text2, PSTR("%s"), fc.task.next_waypoint.twpt.wpt.name);

				widget_value_txt2(text1, text2, x, y + lh, w, h - lh);
			}
			else
			{
				char text[6];
				sprintf_P(text, PSTR("Done!"));

				widget_value_txt(text, x, y + lh, w, h - lh);
			}
		}

	}
	else
	{
		char text[7];
		sprintf_P(text, PSTR("Load"));

		widget_value_txt(text, x, y + lh, w, h - lh);
	}
}

void widget_direction_draw(const char *label_P, int16_t direction, uint8_t x, uint8_t y, uint8_t w, uint8_t h)
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

void widget_home_arrow_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	int16_t relative_direction;

	if (fc.flight.home_valid && fc.gps_data.valid)
	{
		relative_direction = fc.flight.home_bearing - fc.gps_data.heading;
	}
	else
	{
		relative_direction = INT_MAX;
	}

	widget_direction_draw(PSTR("Home"), relative_direction, x, y, w, h);
}

void widget_waypoint_direction_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	int16_t relative_direction;

	if (waypoint_task_active() && fc.gps_data.valid)
	{
		relative_direction = fc.task.next_waypoint.bearing - fc.gps_data.heading;
	}
	else
	{
		relative_direction = INT_MAX;
	}
	widget_direction_draw(PSTR("WP"), relative_direction, x, y, w, h);
}

register_widget2(w_odo_meter, "Odometer", widget_odometer_draw, 0, widget_odometer_irqh);

register_widget1(w_odo_home_direction, "Home Arrow", widget_home_arrow_draw);
register_widget1(w_odo_home_distance, "Home Distance", widget_home_distance_draw);
register_widget1(w_odo_home_time, "Home Time", widget_home_time_draw);
register_widget2(w_home_info, "Home Info", widget_home_info_draw, 0, widget_home_info_irqh);

register_widget1(w_waypoint_direction, "Waypoint Arrow", widget_waypoint_direction_draw);
register_widget1(w_waypoint_distance, "Waypoint Distance", widget_waypoint_distance_draw);
register_widget1(w_waypoint_time, "Waypoint Time", widget_waypoint_time_draw);
register_widget2(w_waypoint_info, "Waypoint Info", widget_waypoint_info_draw, 0, widget_waypoint_info_irqh);

