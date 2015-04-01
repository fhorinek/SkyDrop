#include "altitude.h"
#include "../pages.h"

void widget_alt1_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	uint8_t lh = widget_label(PSTR("Alt1"), x, y);

	char text[10];
	if (flight_data.baro_valid)
		sprintf(text, "%0.0f", flight_data.altitude1);
	else
		sprintf(text, "---");
	widget_value_int(text, x, y + lh, w, h - lh);
}

void widget_alt2_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	uint8_t lh = widget_label(PSTR("Alt2"), x, y);

	char text[10];
	if (flight_data.baro_valid)
		sprintf(text, "%0.0f", flight_data.altitude2);
	else
		sprintf(text, "---");
	widget_value_int(text, x, y + lh, w, h - lh);
}

void widget_alt1_menu_handler(uint8_t type, uint8_t * buff)
{
	gui_enter_widget_menu();

}

void widget_alt1_menu_draw()
{
	gui_draw_box();
	disp.LoadFont(F_VALUES_L);
	uint8_t h_v = disp.GetTextHeight();
	disp.LoadFont(F_TEXT_M);
	uint8_t h_t = disp.GetTextHeight();

	disp.GotoXY(GUI_BOX_LEFT + 1, GUI_BOX_TOP + 1);
	fprintf(lcd_out, "Altitude 1");

	disp.GotoXY(GUI_BOX_LEFT + 1, GUI_BOX_TOP + 1 + h_v);
	fprintf(lcd_out, "ALT1");

	disp.GotoXY(GUI_BOX_LEFT + 1, GUI_BOX_TOP + 1 + h_v + h_v);
	fprintf(lcd_out, "QNH1");

	disp.LoadFont(F_VALUES_L);
	char tmp[10];
	sprintf(tmp, "%0.1f", flight_data.altitude1);
	gui_raligh_text(tmp, GUI_BOX_RIGHT - 1, GUI_BOX_TOP + 1 + h_t);

	sprintf(tmp, "%0.2f", flight_data.QNH1 / 10);
	gui_raligh_text(tmp, GUI_BOX_RIGHT - 1, GUI_BOX_TOP + 1 + h_t + h_v);

}


register_widget(w_alt1, widget_alt1_draw, "Altitude 1", widget_alt1_menu_draw, widget_alt1_menu_handler);
register_widget(w_alt2, widget_alt2_draw, "Altitude 2", NULL, NULL);
