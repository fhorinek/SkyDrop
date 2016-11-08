#include "gps.h"

void widget_ghdg_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("GHdg"), x, y);

	char tmp[5];
	if (fc.gps_data.valid)
		sprintf_P(tmp, PSTR("%0.0f"), fc.gps_data.heading);
	else
		sprintf_P(tmp, PSTR("---"));


	widget_value_int(tmp, x, y + lh, w, h - lh);
}

void widget_gspd_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("GSpd"), x, y);

	char tmp[5];
	if (fc.gps_data.valid)
	{
		float val;

		switch(config.connectivity.gps_format_flags & GPS_SPD_MASK)
		{
			case(GPS_SPD_KPH):
				val = fc.gps_data.groud_speed * FC_KNOTS_TO_KPH;
			break;

			case(GPS_SPD_MS):
				val = fc.gps_data.groud_speed * FC_KNOTS_TO_MPS;
			break;

			case(GPS_SPD_MPH):
				val = fc.gps_data.groud_speed * FC_KNOTS_TO_MPH;
			break;

			case(GPS_SPD_KNOT):
				val = fc.gps_data.groud_speed;
			break;
		}

		sprintf_P(tmp, PSTR("%0.0f"), val);
	}
	else
		sprintf_P(tmp, PSTR("---"));


	widget_value_int(tmp, x, y + lh, w, h - lh);
}

void widget_gpos_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("GPos"), x, y);

	if (fc.gps_data.valid)
	{
		widget_value_txt2((char *)fc.gps_data.cache_gui_latitude,
				(char *)fc.gps_data.cache_gui_longtitude, x, y + lh, w, h - lh);
	}
	else
	{
		char tmp[4];
		sprintf_P(tmp, PSTR("---"));
		widget_value_int(tmp, x, y + lh, w, h - lh);
	}


}

void widget_ghdg_arrow_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("GHdg"), x, y);

	y += lh / 2;

	if (fc.gps_data.valid)
	{
		widget_arrow(fc.gps_data.heading, x, y, w, h);
	}
	else
	{
		char tmp[5];
		sprintf_P(tmp, PSTR("---"));
		widget_value_int(tmp, x, y + lh, w, h - lh);
	}

}

void widget_gcompass_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("GCom"), x, y);

	y += lh / 2;

	if (fc.gps_data.valid)
	{
		int16_t compass = 360 - fc.gps_data.heading;

		widget_arrow(compass, x, y, w, h);
	}
	else
	{
		char tmp[5];
		sprintf_P(tmp, PSTR("---"));
		widget_value_int(tmp, x, y + lh, w, h - lh);
	}

}

register_widget1(w_ghdg, "GPS heading", widget_ghdg_draw);
register_widget1(w_ghdg_arrow, "GPS HDG arrow", widget_ghdg_arrow_draw);
register_widget1(w_gspd, "Ground speed", widget_gspd_draw);
register_widget1(w_gpos, "GPS position", widget_gpos_draw);
register_widget1(w_gcom, "GPS compass ", widget_gcompass_draw);

