#include "dummy.h"

void widget_empty_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{}

void widget_debug_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("free"), x, y);

	char text[8];
	sprintf_P(text, PSTR("%d"), freeRam());
	widget_value_int(text, x, y + lh, w, h - lh);
}

register_widget1(w_dummy, "Empty", widget_empty_draw);
register_widget1(w_debug, "Debug", widget_debug_draw);
