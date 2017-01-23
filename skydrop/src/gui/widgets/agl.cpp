#include "agl.h"


void widget_agl_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("HAGL"), x, y);

	char text[10];

	int16_t val = (int16_t)(fc.altitude1 - fc.agl.ground_level);

	if (config.altitude.alt1_flags & ALT_UNIT_I)
		val *= FC_METER_TO_FEET;

	if (fc.agl.valid)
		sprintf_P(text, PSTR("%d"), val);
	else
		sprintf_P(text, PSTR("---"));

	widget_value_int(text, x, y + lh, w, h - lh);
}

void widget_gl_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("GLevel"), x, y);

	char text[10];

	int16_t val = (int16_t)(fc.agl.ground_level);

	if (config.altitude.alt1_flags & ALT_UNIT_I)
		val *= FC_METER_TO_FEET;

	if (fc.agl.valid)
		sprintf_P(text, PSTR("%d"), val);
	else
		sprintf_P(text, PSTR("---"));

	widget_value_int(text, x, y + lh, w, h - lh);
}


register_widget1(w_agl_height, "Height AGL", widget_agl_draw);
register_widget1(w_agl_level, "Ground Level", widget_gl_draw);
