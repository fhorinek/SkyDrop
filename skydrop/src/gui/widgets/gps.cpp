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
	uint8_t lh = widget_label_P(PSTR("GHdgA"), x, y);

	y += lh / 2;

	if (fc.gps_data.valid)
	{
		uint8_t s = min(w, h);
		uint8_t mx = x + w / 2;
		uint8_t my = y + h / 2;
		float fsin = disp.get_sin(fc.gps_data.heading);
		float fcos = disp.get_cos(fc.gps_data.heading);

		uint8_t x1 = mx + fsin * s / 3;
		uint8_t y1 = my + fcos * s / 3;
		uint8_t x2 = mx - fsin * s / 5;
		uint8_t y2 = my - fcos * s / 5;

		fsin = disp.get_sin(fc.gps_data.heading + 25);
		fcos = disp.get_cos(fc.gps_data.heading + 25);
		uint8_t x3 = mx - fsin * s / 3;
		uint8_t y3 = my - fcos * s / 3;

		fsin = disp.get_sin(fc.gps_data.heading + 335);
		fcos = disp.get_cos(fc.gps_data.heading + 335);
		uint8_t x4 = mx - fsin * s / 3;
		uint8_t y4 = my - fcos * s / 3;


		disp.DrawLine(x1, y1, x3, y3, 1);
		disp.DrawLine(x2, y2, x3, y3, 1);
		disp.DrawLine(x2, y2, x4, y4, 1);
		disp.DrawLine(x1, y1, x4, y4, 1);
	}
	else
	{
		char tmp[5];
		sprintf_P(tmp, PSTR("---"));
		widget_value_int(tmp, x, y + lh, w, h - lh);
	}

}

register_widget1(w_ghdg, "GPS Heading", widget_ghdg_draw);
register_widget1(w_ghdg_arrow, "GPS HDG Arrow", widget_ghdg_arrow_draw);
register_widget1(w_gspd, "Ground Speed", widget_gspd_draw);
register_widget1(w_gpos, "GPS position", widget_gpos_draw);

