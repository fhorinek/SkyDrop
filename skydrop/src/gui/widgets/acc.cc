#include "acc.h"


void widget_accx_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label(PSTR("Acc X"), x, y);

	float val = fc.acc_data.x / 100.0;

	char text[10];
	sprintf(text, "%0.1f", val);

	widget_value_int(text, x, y + lh, w, h - lh);
}


//widget w_accx = {
//	widget_accx_draw,
//	NULL,
//	PSTR("Accelerometer X"),
//};

register_widget1(w_accx, "Accelerometer X", widget_accx_draw);
