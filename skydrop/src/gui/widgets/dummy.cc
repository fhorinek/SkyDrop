#include "dummy.h"

void widget_empty_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{}

int16_t max_x = 0;
int16_t max_y = 0;
int16_t max_z = 0;
int16_t min_x = 16384;
int16_t min_y = 16384;
int16_t min_z = 16384;


void widget_debug_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	disp.LoadFont(F_VALUES_S);

	vector_i16_t vec;

	memcpy(&vec, (void *)&fc.mag_data, sizeof(vector_i16_t));

	if (vec.x > max_x) max_x = vec.x;
	if (vec.x < min_x) min_x = vec.x;

	if (vec.y > max_y) max_y = vec.y;
	if (vec.y < min_y) min_y = vec.y;

	if (vec.z > max_z) max_z = vec.z;
	if (vec.z < min_z) min_z = vec.z;

	disp.GotoXY(10,0);
	fprintf(lcd_out, "%+5d %+5d %+5d", vec.x, max_x, min_x);

	disp.GotoXY(10,15);
	fprintf(lcd_out, "%+5d %+5d %+5d", vec.y, max_y, min_y);

	disp.GotoXY(10,30);
	fprintf(lcd_out, "%+5d %+5d %+5d", vec.z, max_z, min_z);
}

register_widget1(w_dummy, "Empty", widget_empty_draw);
register_widget1(w_debug, "Debug", widget_debug_draw);
