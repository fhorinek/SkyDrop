#include "widgets.h"
#include "../../fc/conf.h"

uint8_t widget_menu_state;
uint8_t widget_menu_param1;
float widget_menu_fvalue1;

widget widget_array[NUMBER_OF_WIDGETS] = {
		//debug
		w_dummy, w_debug_etc, w_debug_imu,
		//vario
		w_vario, w_avg_vario, w_vario_bar, w_vario_history,
		//altitude
		w_alt1, w_alt2, w_alt3, w_alt4, w_alt5,
		//accelerometer
		w_acc_total,
		//time date
		w_time, w_date, w_flight_time,
		//temperature
		w_temperature,
		//gps
		w_gps_hdg, w_gps_speed, w_gps_position, w_gps_hdg_arrow, w_gps_hdg_points,
		//battery
		w_battery,
		//flight
		w_glide_ratio,
		//controls
		w_ctrl_audio, w_ctrl_wlift,
		//wind
		w_wind_speed, w_wind_dir, w_wind_dir_arrow,
		//agl
		w_agl_height, w_agl_ground_level,
		//odometer
		w_odo_meter, w_odo_home_direction, w_odo_home_distance, w_odo_home_time,
		//compass
		w_compass_heading, w_compass_arrow, w_compass_points,
};

// Whenever you change something here, you have to do "Clean Project" in Eclipse:
const uint8_t PROGMEM widget_sorted[NUMBER_OF_SORTED_WIDGETS] =
{
	//vario
	WIDGET_VARIO_BAR,
	WIDGET_VARIO,
	WIDGET_AVG_VARIO,
	WIDGET_VARIO_HISTORY,

	//altitude
	WIDGET_ALT1,
	WIDGET_ALT2,
	WIDGET_ALT3,
	WIDGET_ALT4,
	WIDGET_ALT5,

	//agl
	WIDGET_AGL_HEIGHT,
	WIDGET_AGL_LEVEL,

	//gps
	WIDGET_GROUND_SPD,
	WIDGET_GLIDE_RATIO,
	WIDGET_GHEADING,
	WIDGET_GHEADING_ARROW,
	WIDGET_GHEADING_POINTS,
	WIDGET_POSITION,

	//time and date
	WIDGET_FTIME,
	WIDGET_TIME,
	WIDGET_DATE,

	//odometer
	WIDGET_ODO_METER,
	WIDGET_ODO_BACK,
	WIDGET_ODO_DISTANCE,
	WIDGET_HOME_TIME,

	//wind
	WIDGET_WIND_DIR,
	WIDGET_WIND_DIR_ARROW,
	WIDGET_WIND_SPD,

	//acc
	WIDGET_ACC_TOT,

	//compass
	WIDGET_COMPASS,
	WIDGET_COMPASS_ARROW,
	WIDGET_COMPASS_POINTS,

	//etc
	WIDGET_BATTERY,
	WIDGET_TEMPERATURE,

	//ctrl
	WIDGET_CTRL_WLIFT,
	WIDGET_CTRL_AUDIO,

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

/**
 * Prints the text (containing digits) into the given box. Values are printed in
 * a bigger font and with a limited character set:
 *
 * Available characters:
 *   042 2A = *		043 2B = +		044 2C = ,		045 2D = -		046 2E = .
 *   047 2F = /		048 30 = 0		049 31 = 1		050 32 = 2		051 33 = 3
 *   052 34 = 4		053 35 = 5		054 36 = 6		055 37 = 7		056 38 = 8
 *   057 39 = 9
 *
 * \param value the text to be shown.
 * \param x the X coordinate of the box
 * \param y the X coordinate of the box
 * \param w the width of the box
 * \param h the height of the box
 */
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

/**
 * Prints the text into the given box.
 *
 * \param value the text to be shown.
 * \param x the X coordinate of the box
 * \param y the X coordinate of the box
 * \param w the width of the box
 * \param h the height of the box
 */
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

/**
 * Draw an arrow at the given position.
 *
 * \param angle the angle of the arrow (0=up, 90=right, 180=down, 270=left) and
 *              anything between.
 * \param x the X position of center
 * \param y the Y position of center
 * \param w the width
 * \param h the height
 */
void widget_arrow(int16_t angle, uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	// make sure, that angle is always between 0 and 359:
	if (angle < 0 || angle > 359) {
		angle = (angle % 360 + 360) % 360;
	}

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
