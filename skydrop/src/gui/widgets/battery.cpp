#include "battery.h"

#include "../../drivers/battery.h"

void widget_battery_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("Bat"), x, y);

	char text[6];
	sprintf_P(text, PSTR("%d"), battery_per);
	widget_value_int(text, x, y + lh, w, h - lh);
}

register_widget1(w_battery, "Battery", widget_battery_draw);
