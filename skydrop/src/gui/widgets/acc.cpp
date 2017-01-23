#include "acc.h"

void widget_acc_total_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("Acc"), x, y);

	float val = fc.acc.total_filtered;

	char text[10];
	sprintf_P(text, PSTR("%0.1f"), val);

	widget_value_int(text, x, y + lh, w, h - lh);
}

register_widget1(w_acc_tot, "Accelerometer", widget_acc_total_draw);
