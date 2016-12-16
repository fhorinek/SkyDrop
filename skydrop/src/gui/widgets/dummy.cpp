#include "dummy.h"

void widget_empty_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{}

void widget_debug_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("stack"), x, y);

	char text[10];
	sprintf_P(text, PSTR("%04X"), debug_min_stack_pointer);
	widget_value_int(text, x, y + lh, w, h - lh);
}

void widget_imu_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	disp.LoadFont(F_TEXT_S);

	disp.GotoXY(8, 0);
	fprintf_P(lcd_out, PSTR("X"));
	disp.GotoXY(30, 0);
	fprintf_P(lcd_out, PSTR("Y"));
	disp.GotoXY(52, 0);
	fprintf_P(lcd_out, PSTR("Z"));

	disp.GotoXY(0, 12);
	fprintf_P(lcd_out, PSTR("A"));
	disp.GotoXY(8, 12);
	fprintf_P(lcd_out, PSTR("%04d"), fc.acc.raw.x);
	disp.GotoXY(30, 12);
	fprintf_P(lcd_out, PSTR("%04d"), fc.acc.raw.y);
	disp.GotoXY(52, 12);
	fprintf_P(lcd_out, PSTR("%04d"), fc.acc.raw.z);

	disp.GotoXY(0, 24);
	fprintf_P(lcd_out, PSTR("M"));
	disp.GotoXY(8, 24);
	fprintf_P(lcd_out, PSTR("%04d"), fc.mag.raw.x);
	disp.GotoXY(30, 24);
	fprintf_P(lcd_out, PSTR("%04d"), fc.mag.raw.y);
	disp.GotoXY(52, 24);
	fprintf_P(lcd_out, PSTR("%04d"), fc.mag.raw.z);

	disp.GotoXY(0, 36);
	fprintf_P(lcd_out, PSTR("G"));
	disp.GotoXY(8, 36);
	fprintf_P(lcd_out, PSTR("%04d"), fc.gyro.raw.x);
	disp.GotoXY(30, 36);
	fprintf_P(lcd_out, PSTR("%04d"), fc.gyro.raw.y);
	disp.GotoXY(52, 36);
	fprintf_P(lcd_out, PSTR("%04d"), fc.gyro.raw.z);

}

register_widget1(w_dummy, "Empty", widget_empty_draw);
register_widget1(w_debug_etc, "Debug_etc", widget_debug_draw);
register_widget1(w_debug_imu, "Debug_imu", widget_imu_draw);
