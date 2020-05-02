#include "battery.h"

#include "../../drivers/battery.h"

void widget_glide_ratio(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	uint8_t lh = widget_label_P(PSTR("Glide"), x, y);

	char text[10];
	if (fc.glide_ratio_valid && abs(fc.glide_ratio) < 1000000.0)
		sprintf_P(text, PSTR("%0.1f"), fc.glide_ratio);
	else
		strcpy_P(text, PSTR("---"));

	widget_value_int(text, x, y + lh, w, h - lh);
}

register_widget1(w_glide_ratio, "Glide ratio", widget_glide_ratio);
