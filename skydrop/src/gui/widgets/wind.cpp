#include "wind.h"

void widget_wdir_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("Wdir"), x, y);

	char tmp[5];
	if (fc.wind.valid)
		sprintf_P(tmp, PSTR("%0.0f"), fc.wind.direction);
	else
		sprintf_P(tmp, PSTR("---"));


	widget_value_int(tmp, x, y + lh, w, h - lh);
}

void widget_wspd_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("Wspd"), x, y);

	char tmp[5];
	if (fc.wind.valid)
	{
		float val;

		switch(config.connectivity.gps_format_flags & GPS_SPD_MASK)
		{
			case(GPS_SPD_KPH):
				val = fc.wind.speed * FC_MPS_TO_KPH;
			break;

			case(GPS_SPD_MS):
				val = fc.wind.speed;
			break;

			case(GPS_SPD_MPH):
				val = fc.wind.speed * FC_MPS_TO_MPH;
			break;

			case(GPS_SPD_KNOT):
				val = fc.wind.speed * FC_MPS_TO_KNOTS;
			break;
		}

		sprintf_P(tmp, PSTR("%0.1f"), val);
	}
	else
		sprintf_P(tmp, PSTR("---"));


	widget_value_int(tmp, x, y + lh, w, h - lh);
}

void widget_wdir_arrow_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("WdirA"), x, y);

	y += lh / 2;

	if (fc.wind.valid)
	{
		float relative_direction = fc.wind.direction -fc.gps_data.heading;
		if (relative_direction < 0)
		{
			relative_direction += 360.0;
		}

		uint8_t s = min(w, h);
		uint8_t mx = x + w / 2;
		uint8_t my = y + h / 2;
		float fsin = disp.get_sin(relative_direction);
		float fcos = disp.get_cos(relative_direction);

		uint8_t x1 = mx + fsin * s / 3;
		uint8_t y1 = my + fcos * s / 3;
		uint8_t x2 = mx - fsin * s / 5;
		uint8_t y2 = my - fcos * s / 5;

		fsin = disp.get_sin(relative_direction + 25);
		fcos = disp.get_cos(relative_direction + 25);
		uint8_t x3 = mx - fsin * s / 3;
		uint8_t y3 = my - fcos * s / 3;

		fsin = disp.get_sin(relative_direction + 335);
		fcos = disp.get_cos(relative_direction + 335);
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

register_widget1(w_wdir, "Wind Direction", widget_wdir_draw);
register_widget1(w_wdir_arrow, "Wind DIR Arrow", widget_wdir_arrow_draw);
register_widget1(w_wspd, "Wind Speed", widget_wspd_draw);
