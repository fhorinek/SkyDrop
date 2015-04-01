#include "widgets.h"
#include "../../fc/conf.h"
#include "layouts/layouts.h"

layout_t pages[NUMBER_OF_PAGES];



widget widget_array[] ={
		w_dummy,
		w_vario, w_avg_vario, w_vario_bar,
		w_alt1, w_alt2,
		w_accx,
};

uint8_t widget_label(const char * label, uint8_t x, uint8_t y)
{
	disp.LoadFont(F_LABEL);
	disp.GotoXY(x + 1, y);
	fprintf_P(lcd_out, PSTR("%S"), label);
	return disp.GetAHeight();
}

void widget_value_int(char * value, uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	disp.LoadFont(F_VALUES_L);
	uint8_t text_w = disp.GetTextWidth(value);
	if (w < text_w)
	{
		disp.LoadFont(F_VALUES_M);
		text_w = disp.GetTextWidth(value);
		if (w < text_w)
		{
			disp.LoadFont(F_VALUES_S);
			text_w = disp.GetTextWidth(value);
			if (w < text_w)
				return;
		}
	}

	uint8_t text_h = disp.GetAHeight();
	disp.GotoXY(x + w / 2 - text_w / 2, y + h / 2 - text_h / 2);
	fprintf_P(lcd_out, PSTR("%s"), value);
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
			widget_array[wtype].draw(x, y, w, h);
	}
}

