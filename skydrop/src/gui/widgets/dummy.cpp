#include "dummy.h"

void widget_empty_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{}

register_widget1(w_dummy, "Empty", widget_empty_draw);

