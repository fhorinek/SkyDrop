#include "dummy.h"

void widget_dummy_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	disp.DrawRectangle(x, y, x + w - 1, y + h - 1, 1, 0);
	disp.DrawLine(x, y, x + w - 1, y + h - 1, 1);
	disp.DrawLine(x, y + h - 1, x + w - 1, y, 1);
}

register_widget(w_dummy, widget_dummy_draw, "Dummy", NULL, NULL);
