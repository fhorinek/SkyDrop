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
	if (fc.autostart_state == AUTOSTART_FLIGHT || fc.epoch_flight_start != 0)
	{
		uint32_t diff = time_get_actual() - fc.epoch_flight_start;
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
	}
	else
	{
		sprintf_P(tmp, PSTR("---"));
	}


	widget_value_int(tmp, x, y + lh, w, h - lh);
}

register_widget1(w_time, "Time", widget_time_draw);
register_widget1(w_ftime, "Flight time", widget_flight_time_draw);
