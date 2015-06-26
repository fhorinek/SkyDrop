#include "dummy.h"

void widget_empty_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{}

void widget_debug_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{

}

register_widget1(w_dummy, "Empty", widget_empty_draw);
register_widget1(w_debug, "Debug", widget_debug_draw);
