#include "widgets.h"
#include "../../fc/conf.h"

uint8_t widget_menu_state;
uint8_t widget_menu_param1;
float widget_menu_fvalue1;

widget widget_array[NUMBER_OF_WIDGETS] = {
		//debug
		w_dummy,
		//vario
		w_vario, w_avg_vario, w_vario_bar, w_vario_history,
		//altitude
		w_alt1, w_alt2, w_alt3, w_alt4, w_alt5,
		//accelerometer
		w_acc_total,
		//time date
		w_time, w_date, w_flight_time, w_hike_mode,
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
		w_wind_speed, w_wind_dir, w_wind_dir_arrow, w_wind_points,
		//agl
		w_agl_height, w_agl_ground_level,
		//odometer
		w_odo_meter, w_odo_home_direction, w_odo_home_distance, w_odo_home_time, w_home_info,
		//compass
		w_compass_heading, w_compass_arrow, w_compass_points,
		//thermal
		w_thermal_time, w_thermal_gain, w_thermal_ass,
		//waypoints
		w_waypoint_direction, w_waypoint_distance, w_waypoint_time, w_waypoint_info,
		//airspace
		w_airspace_info
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

	//thermal
	WIDGET_THERMAL_TIME,
	WIDGET_THERMAL_GAIN,
	WIDGET_THERMAL_ASS,

	//time and date
	WIDGET_FTIME,
	WIDGET_HIKE,
	WIDGET_TIME,
	WIDGET_DATE,

	//odometer
	WIDGET_ODO_METER,
	WIDGET_ODO_BACK,
	WIDGET_ODO_DISTANCE,
	WIDGET_HOME_TIME,
	WIDGET_HOME_INFO,
  
   //waypoints
	WIDGET_WAYPOINT_ARROW,
	WIDGET_WAYPOINT_DISTANCE,
	WIDGET_WAYPOINT_TIME,
	WIDGET_WAYPOINT_INFO,

	//airspace
	WIDGET_AIRSPACE_INFO,

	//wind
	WIDGET_WIND_DIR,
	WIDGET_WIND_DIR_ARROW,
	WIDGET_WIND_DIR_POINTS,
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

/**
 * Format a distance in a human readable format.
 *
 * @param text_number the text buffer to print the value into.
 * @param text_unit the text buffer to print the unit into.
 * @param distance the distance in km.
 */
void sprintf_distance(char *text_number, char *text_unit, float distance)
{
	if (distance != INFINITY)
	{
		if (config.connectivity.gps_format_flags & GPS_DIST_UNIT_I)
	    {
			distance *= FC_KM_TO_MILE;
			strcpy_P(text_unit, PSTR("mi"));
	    } else
	        strcpy_P(text_unit, PSTR("km"));

		if (distance < 10.0)
			sprintf_P(text_number, PSTR("%.2f"), distance);
		else if (distance < 100.0)
			sprintf_P(text_number, PSTR("%.1f"), distance);
		else
			sprintf_P(text_number, PSTR("%.0f"), distance);

	} else {
		strcpy_P(text_number, PSTR("---"));
		text_unit[0] = 0;
	}
}

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
	uint8_t text_w1 = disp.GetTextWidth(value1);
	uint8_t text_w2 = disp.GetTextWidth(value2);
	uint8_t text_w = max(text_w1, text_w2);
	uint8_t text_h = disp.GetTextHeight() * 2;

	if (w < text_w || h < text_h)
	{
		disp.LoadFont(F_TEXT_M);
		text_w1 = disp.GetTextWidth(value1);
		text_w2 = disp.GetTextWidth(value2);
		uint8_t text_w = max(text_w1, text_w2);
		text_h = disp.GetTextHeight() * 2;
		if (w < text_w || h < text_h)
		{
			disp.LoadFont(F_TEXT_S);
			text_w1 = disp.GetTextWidth(value1);
			text_w2 = disp.GetTextWidth(value2);
			uint8_t text_w = max(text_w1, text_w2);
			text_h = disp.GetTextHeight() * 2;
			if (w < text_w || h < text_h)
			{
				widget_value_txt(value1, x, y, w, h);
				return;
			}
		}
	}

	disp.GotoXY(x + w / 2 - text_w1 / 2, y + h / 2 - text_h / 2);
	fprintf_P(lcd_out, PSTR("%s"), value1);
	disp.GotoXY(x + w / 2 - text_w2 / 2, y + h / 2);
	fprintf_P(lcd_out, PSTR("%s"), value2);
}

/**
 * Prints the text into the given box and scrolls the text if it is too wide.
 *
 * \param text the text to be shown.
 * \param x the X coordinate of the box
 * \param y the X coordinate of the box
 * \param w the width of the box
 * \param h the height of the box
 */
void widget_value_scroll(char * text, uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	disp.LoadFont(F_TEXT_L);
	uint8_t text_h = disp.GetTextHeight();

	if (h < text_h)
	{
		disp.LoadFont(F_TEXT_M);
		text_h = disp.GetTextHeight();
		if (h < text_h)
		{
			disp.LoadFont(F_TEXT_S);
			text_h = disp.GetTextHeight();
			if (h < text_h)
				return;
		}
	}

	uint16_t text_w = disp.GetTextWidth(text);
	if (text_w > w)
	{
		disp.clip(x, x + w);

		uint32_t offset = (task_get_ms_tick() / 100) % ((uint32_t)text_w + 15);
		int16_t scroll_x = x - (int16_t)offset;

		disp.GotoXY_16(scroll_x, y + h / 2 - text_h / 2);
		fputs(text, lcd_out);

		scroll_x += text_w + 15;
		if (scroll_x < x + w)
		{
			disp.GotoXY_16(scroll_x, y + h / 2 - text_h / 2);
			fputs(text, lcd_out);
		}

		disp.noclip();
	}
	else
	{
		gui_caligh_text(text, x + w / 2, y + h / 2 - text_h / 2);
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
			widget_array[wtype].draw(x, y, w, h);
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
	if (angle < 0 || angle > 359)
		angle = (angle % 360 + 360) % 360;

	uint8_t s = min(w, h);

	bool simple = s <= 16;

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
	disp.DrawLine(x1, y1, x4, y4, 1);

	if (simple)
	{
		disp.DrawLine(x3, y3, x4, y4, 1);

	}
	else
	{
		disp.DrawLine(x2, y2, x3, y3, 1);
		disp.DrawLine(x2, y2, x4, y4, 1);
	}
}

//converts degrees to points N
void widget_deg_to_points(float deg, char * text)
{
	if	( 22.5 <= deg and deg <  67.5 )
		strcpy_P(text, PSTR("NE"));
	else if( 67.5 <= deg and deg < 112.5 )
		strcpy_P(text, PSTR("E"));
	else if(112.5 <= deg and deg < 157.5 )
		strcpy_P(text, PSTR("SE"));
	else if(157.5 <= deg and deg < 202.5 )
		strcpy_P(text, PSTR("S"));
	else if(202.5 <= deg and deg < 247.5 )
		strcpy_P(text, PSTR("SW"));
	else if(247.5 <= deg and deg < 292.5 )
		strcpy_P(text, PSTR("W"));
	else if(292.5 <= deg and deg < 337.5 )
		strcpy_P(text, PSTR("NW"));
	else
		strcpy_P(text, PSTR("N"));

}
