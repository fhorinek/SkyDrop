#include "widgets.h"
#include "../../fc/conf.h"

uint8_t widget_menu_state;
uint8_t widget_menu_param1;
float widget_menu_fvalue1;

widget widget_array[NUMBER_OF_WIDGETS] = {
		w_dummy, w_debug_etc, w_debug_imu,
		w_vario, w_avg_vario, w_vario_bar, w_vario_history,
		w_alt1, w_alt2, w_alt3, w_alt4, w_alt5,
		w_acc_tot,
		w_time, w_date, w_ftime,
		w_temperature,
		w_ghdg, w_gspd, w_gpos, w_ghdg_arrow, w_gcom,
		w_battery,
		w_glide_ratio,
		w_ctrl_audio, w_ctrl_wlift,
		w_wspd, w_wdir, w_wdir_arrow,
		w_agl_height, w_agl_level,
		w_odo_meter, w_odo_back, w_odo_distance
};

const uint8_t PROGMEM widget_sorted[NUMBER_OF_SORTED_WIDGETS] =
{
	WIDGET_VARIO_BAR,
	WIDGET_VARIO,
	WIDGET_AVG_VARIO,
	WIDGET_VARIO_HISTORY,
	WIDGET_CTRL_WLIFT,
	WIDGET_CTRL_AUDIO,
	WIDGET_ALT1,
	WIDGET_ALT2,
	WIDGET_ALT3,
	WIDGET_ALT4,
	WIDGET_ALT5,
	WIDGET_AGL_HEIGHT,
	WIDGET_AGL_LEVEL,
	WIDGET_GROUND_SPD,
	WIDGET_GLIDE_RATIO,
	WIDGET_GHEADING,
	WIDGET_GHEADING_ARROW,
	WIDGET_GCOMPASS,
	WIDGET_POSITION,
	WIDGET_FTIME,
	WIDGET_TIME,
	WIDGET_ODO_METER,
	WIDGET_ODO_BACK,
	WIDGET_ODO_DISTANCE,
	WIDGET_WIND_DIR,
	WIDGET_WIND_DIR_ARROW,
	WIDGET_WIND_SPD,
	WIDGET_ACC_TOT,
	WIDGET_DATE,
	WIDGET_BATTERY,
	WIDGET_TEMPERATURE,
	WIDGET_EMPTY
};

uint8_t widget_sorted_get_index(uint8_t pos)
{
	return pgm_read_byte(&widget_sorted[pos]);
}

uint8_t widget_label_P(const char * label, uint8_t x, uint8_t y)
{
	if (config.gui.hide_label & (1 << active_page))
		return 0;

	disp.LoadFont(F_LABEL);
	disp.GotoXY(x + 1, y);
	fprintf_P(lcd_out, PSTR("%S"), label);
	return disp.GetAHeight() + 1;
}

uint8_t widget_label(char * label, uint8_t x, uint8_t y)
{
	if (config.gui.hide_label & (1 << active_page))
		return 0;

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

void widget_value_txt(char * value, uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	disp.LoadFont(F_TEXT_L);
	uint8_t text_w = disp.GetTextWidth(value);
	uint8_t text_h = disp.GetTextHeight();

	if (w < text_w || h < text_h)
	{
		disp.LoadFont(F_TEXT_M);
		text_w = disp.GetTextWidth(value);
		text_h = disp.GetTextHeight();
		if (w < text_w || h < text_h)
		{
			disp.LoadFont(F_TEXT_S);
			text_w = disp.GetTextWidth(value);
			text_h = disp.GetTextHeight();
			if (w < text_w || h < text_h)
				return;
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
	return config.gui.pages[page].widgets[widget];
}

void widgets_draw(uint8_t page)
{
	if (config.gui.pages[page].type == LAYOUT_OFF)
		return;

	const layout_desc * adr = layout_list[config.gui.pages[page].type];

	uint8_t active_widgets = pgm_read_byte(&adr->number_of_widgets);

	for (uint8_t i = 0; i < active_widgets; i++)
	{
		uint8_t x, y, w, h;

		x = pgm_read_byte(&adr->widgets[i].x);
		y = pgm_read_byte(&adr->widgets[i].y);
		w = pgm_read_byte(&adr->widgets[i].w);
		h = pgm_read_byte(&adr->widgets[i].h);


		uint8_t wtype = config.gui.pages[page].widgets[i];

		if (wtype != WIDGET_OFF)
			widget_array[wtype].draw(x, y, w, h, widget_array[wtype].flags);
	}
}

void widget_arrow(uint16_t angle, uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	uint8_t s = min(w, h);
	uint8_t mx = x + w / 2;
	uint8_t my = y + h / 2;
	float fsin = disp.get_sin(angle);
	float fcos = disp.get_cos(angle);

	uint8_t x1 = mx + fsin * s / 3;
	uint8_t y1 = my + fcos * s / 3;
	uint8_t x2 = mx - fsin * s / 5;
	uint8_t y2 = my - fcos * s / 5;

	fsin = disp.get_sin(angle + 25);
	fcos = disp.get_cos(angle + 25);
	uint8_t x3 = mx - fsin * s / 3;
	uint8_t y3 = my - fcos * s / 3;

	fsin = disp.get_sin(angle + 335);
	fcos = disp.get_cos(angle + 335);
	uint8_t x4 = mx - fsin * s / 3;
	uint8_t y4 = my - fcos * s / 3;

	disp.DrawLine(x1, y1, x3, y3, 1);
	disp.DrawLine(x2, y2, x3, y3, 1);
	disp.DrawLine(x2, y2, x4, y4, 1);
	disp.DrawLine(x1, y1, x4, y4, 1);
}
