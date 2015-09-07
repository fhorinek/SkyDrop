#include "time.h"

void widget_time_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t sec;
	uint8_t min;
	uint8_t hour;

	time_from_epoch(time_get_actual(), &sec, &min, &hour);

	uint8_t lh = widget_label_P(PSTR("Time"), x, y);

	char tmp[7];
	sprintf_P(tmp, PSTR("%02d:%02d"), hour, min);

	widget_value_int(tmp, x, y + lh, w, h - lh);
}

void widget_flight_time_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("FTime"), x, y);

	char tmp[7];
	if (fc.autostart_state == AUTOSTART_FLIGHT)
	{
		uint32_t diff = time_get_actual() - fc.epoch_flight_timer;
		uint8_t hour, min;

		hour = diff / 3600;
		diff %= 3600;

		min = diff / 60;
		diff %= 60;

		if (hour > 0)
		{
			sprintf_P(tmp, PSTR("%02d:%02d"), hour, min);
		}
		else
			sprintf_P(tmp, PSTR("%02d.%02d"), min, diff);

		widget_value_int(tmp, x, y + lh, w, h - lh);
	}

	if (fc.autostart_state == AUTOSTART_LAND)
	{
		uint32_t diff = fc.epoch_flight_timer;
		uint8_t hour, min;

		hour = diff / 3600;
		diff %= 3600;

		min = diff / 60;
		diff %= 60;

		if (hour > 0)
		{
			sprintf_P(tmp, PSTR("%02d:%02d"), hour, min);
		}
		else
			sprintf_P(tmp, PSTR("%02d.%02d"), min, diff);

		widget_value_int(tmp, x, y + lh, w, h - lh);
	}

	if (fc.autostart_state == AUTOSTART_WAIT)
	{
		sprintf_P(tmp, PSTR("Start"));
		widget_value_txt(tmp, x, y + lh, w, h - lh);
	}
}

void widget_flight_time_irqh(uint8_t type, uint8_t * buff, uint8_t index)
{
	if (type == B_MIDDLE && *buff == BE_LONG)
	{
		if (fc.autostart_state == AUTOSTART_WAIT)
		{
			fc_takeoff();
			return;
		}

		if (fc.autostart_state == AUTOSTART_FLIGHT)
		{
			fc_landing();
			return;
		}
	}
}

register_widget1(w_time, "Time", widget_time_draw);
register_widget2(w_ftime, "Flight time", widget_flight_time_draw, 0, widget_flight_time_irqh);
