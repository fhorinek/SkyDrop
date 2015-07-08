#include "widgets.h"
#include "../../fc/conf.h"

layout_t pages[NUMBER_OF_PAGES];

uint8_t widget_menu_state;
uint8_t widget_menu_param1;
float widget_menu_fvalue1;

widget widget_array[NUMBER_OF_WIDGETS] = {
		w_dummy, w_debug,
		w_vario, w_avg_vario, w_vario_bar,
		w_alt1, w_alt2, w_alt3, w_alt4, w_alt5,
		w_accx,
		w_time, w_ftime,
		w_temperature,
		w_ghdg, w_gspd, w_gpos,
		w_battery,
};

uint8_t widget_label_P(const char * label, uint8_t x, uint8_t y)
{
	disp.LoadFont(F_LABEL);
	disp.GotoXY(x + 1, y);
	fprintf_P(lcd_out, PSTR("%S"), label);
	return disp.GetAHeight() + 1;
}

uint8_t widget_label(char * label, uint8_t x, uint8_t y)
{
	disp.LoadFont(F_LABEL);
	disp.GotoXY(x + 1, y);
	fprintf_P(lcd_out, PSTR("%s"), label);
	return disp.GetAHeight() + 1;
}

void widget_value_int_sub(char * value, char * sub, uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	disp.LoadFont(F_TEXT_S);
	uint8_t text_ws = disp.GetTextWidth(sub);
	uint8_t text_hs = disp.GetTextHeight();

	disp.LoadFont(F_VALUES_XL);
	uint8_t text_w = disp.GetTextWidth(value);
	uint8_t text_h = disp.GetTextHeight();

	if (w < text_w || h < (text_h + text_hs))
	{
		disp.LoadFont(F_VALUES_L);
		text_w = disp.GetTextWidth(value);
		text_h = disp.GetTextHeight();
		if (w < text_w || h < (text_h + text_hs))
		{
			disp.LoadFont(F_VALUES_M);
			text_w = disp.GetTextWidth(value);
			text_h = disp.GetTextHeight();
			if (w < text_w || h < (text_h + text_hs))
			{
				disp.LoadFont(F_VALUES_S);
				text_w = disp.GetTextWidth(value);
				text_h = disp.GetTextHeight();
				if (w < text_w || h < text_h)
				{
					widget_value_int(value, x, y, w, h);
					return;
				}
			}
		}
	}

	disp.GotoXY(x + w / 2 - text_w / 2, y + h / 2 - (text_h + text_hs) / 2);
	fprintf_P(lcd_out, PSTR("%s"), value);
	disp.GotoXY(x + w / 2 - text_ws / 2, y + h / 2 - (text_h + text_hs) / 2 + text_h);
	disp.LoadFont(F_TEXT_S);
	fprintf_P(lcd_out, PSTR("%s"), sub);
}

void widget_value_int(char * value, uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	disp.LoadFont(F_VALUES_XL);
	uint8_t text_w = disp.GetTextWidth(value);
	uint8_t text_h = disp.GetTextHeight();

	if (w < text_w || h < text_h)
	{
		disp.LoadFont(F_VALUES_L);
		text_w = disp.GetTextWidth(value);
		text_h = disp.GetTextHeight();
		if (w < text_w || h < text_h)
		{
			disp.LoadFont(F_VALUES_M);
			text_w = disp.GetTextWidth(value);
			text_h = disp.GetTextHeight();
			if (w < text_w || h < text_h)
			{
				disp.LoadFont(F_VALUES_S);
				text_w = disp.GetTextWidth(value);
				text_h = disp.GetTextHeight();
				if (w < text_w || h < text_h)
					return;
			}
		}
	}

	disp.GotoXY(x + w / 2 - text_w / 2, y + h / 2 - text_h / 2);
	fprintf_P(lcd_out, PSTR("%s"), value);
}

void widget_value_txt2(char * value1, char * value2, uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	disp.LoadFont(F_TEXT_L);
	uint8_t text_w = disp.GetTextWidth(value1);
	uint8_t text_h = disp.GetTextHeight() * 2;

	if (w < text_w || h < text_h)
	{
		disp.LoadFont(F_TEXT_M);
		text_w = disp.GetTextWidth(value1);
		text_h = disp.GetTextHeight() * 2;
		if (w < text_w || h < text_h)
		{
			disp.LoadFont(F_TEXT_S);
			text_w = disp.GetTextWidth(value1);
			text_h = disp.GetTextHeight() * 2;
			if (w < text_w || h < text_h)
				return;
		}
	}

	disp.GotoXY(x + w / 2 - text_w / 2, y + h / 2 - text_h / 2);
	fprintf_P(lcd_out, PSTR("%s"), value1);
	disp.GotoXY(x + w / 2 - text_w / 2, y + h / 2);
	fprintf_P(lcd_out, PSTR("%s"), value2);
}

void widgets_init()
{
	eeprom_busy_wait();

	DEBUG("Widgets init\n");

	for (uint8_t i = 0; i < NUMBER_OF_PAGES; i++)
	{
		pages[i].type = eeprom_read_byte(&config.gui.pages[i].type);
		DEBUG("%d, %d\n", i, pages[i].type);
		for (uint8_t j = 0; j < WIDGES_PER_PAGE; j++)
			pages[i].widgets[j] = eeprom_read_byte(&config.gui.pages[i].widgets[j]);
	}
}


uint8_t layout_get_number_of_widgets(uint8_t type)
{
	const layout_desc * adr = layout_list[type];
	return pgm_read_byte(&adr->number_of_widgets);
}

void layout_get_widget_rect(uint8_t type, uint8_t widget, uint8_t * x, uint8_t * y, uint8_t * w, uint8_t * h)
{
	const layout_desc * adr = layout_list[type];

	*x = pgm_read_byte(&adr->widgets[widget].x);
	*y = pgm_read_byte(&adr->widgets[widget].y);
	*w = pgm_read_byte(&adr->widgets[widget].w);
	*h = pgm_read_byte(&adr->widgets[widget].h);
}


uint8_t widget_get_type(uint8_t page, uint8_t widget)
{
	return pages[page].widgets[widget];
}

void widgets_draw(uint8_t page)
{
	if (pages[page].type == LAYOUT_OFF)
		return;

	const layout_desc * adr = layout_list[pages[page].type];

	uint8_t active_widgets = pgm_read_byte(&adr->number_of_widgets);

	for (uint8_t i = 0; i < active_widgets; i++)
	{
		uint8_t x, y, w, h;

		x = pgm_read_byte(&adr->widgets[i].x);
		y = pgm_read_byte(&adr->widgets[i].y);
		w = pgm_read_byte(&adr->widgets[i].w);
		h = pgm_read_byte(&adr->widgets[i].h);


		uint8_t wtype = pages[page].widgets[i];

		if (wtype != WIDGET_OFF)
			widget_array[wtype].draw(x, y, w, h, widget_array[wtype].flags);
	}
}

