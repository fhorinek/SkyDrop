#include "battery.h"

#include "../../drivers/battery.h"

void widget_battery_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	uint8_t lh = widget_label_P(PSTR("Bat"), x, y);
	char text1[8];
	char text2[8] = { 0 };

	if (battery_per == BATTERY_CHARGING)
		strcpy_P(text1, PSTR("Chrg"));
	else if (battery_per == BATTERY_FULL)
		strcpy_P(text1, PSTR("Full"));
	else
	{
		sprintf_P(text1, PSTR("%d%%"), battery_per);
		if (battery_calibrated())
			sprintf_P(text2, PSTR("%dmin"), battery_runtime_minutes() * battery_per / 100);
	}

	if (text2[0])
		widget_value_txt2(text1, text2, x, y + lh, w, h - lh);
	else
		widget_value_txt(text1, x, y + lh, w, h - lh);
}

register_widget1(w_battery, "Battery", widget_battery_draw);
