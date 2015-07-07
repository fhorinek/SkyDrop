#include "temperature.h"

void widget_temperature_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("Temp"), x, y);

	char tmp[5], sub[5];
	sprintf_P(tmp, PSTR("%d.%d"), fc.temperature / 10, fc.temperature % 10);
	sprintf_P(sub, PSTR("%d%%"), fc.humidity / 100);

	widget_value_int_sub(tmp, sub, x, y + lh, w, h - lh);

}

register_widget1(w_temperature, "Temperature", widget_temperature_draw);
