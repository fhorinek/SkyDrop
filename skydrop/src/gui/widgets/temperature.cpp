#include "temperature.h"

void widget_temperature_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	uint8_t lh = widget_label_P(PSTR("Temp"), x, y);

	char tmp[5], sub[5];

	if (sht21.present)
	{
		int16_t res = fc.temp.temp / 10;

		if (config.gui.disp_flags & CFG_DISP_FAHRENHEIT)
			res = (res * 1.8) + 32;

		sprintf_P(tmp, PSTR("%d"), res);
		sprintf_P(sub, PSTR("%d%%"), fc.temp.humid / 100);

		widget_value_int_sub(tmp, sub, x, y + lh, w, h - lh);
	}
	else
	{
		widget_value_txt((char *)"N/A", x, y + lh, w, h - lh);
	}
}

register_widget1(w_temperature, "Temperature", widget_temperature_draw);
