#include "wind.h"

void widget_wdir_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	uint8_t lh = widget_label_P(PSTR("WDir"), x, y);

	char tmp[5];
	if (fc.wind.valid)
		sprintf_P(tmp, PSTR("%0.0f"), fc.wind.direction);
	else
		strcpy_P(tmp, PSTR("---"));


	widget_value_int(tmp, x, y + lh, w, h - lh);
}

void widget_wspd_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	uint8_t lh = widget_label_P(PSTR("WSpd"), x, y);

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

		sprintf_P(tmp, PSTR("%0.0f"), val);
	}
	else
		strcpy_P(tmp, PSTR("---"));


	widget_value_int(tmp, x, y + lh, w, h - lh);
}

void widget_wdir_arrow_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	uint8_t lh = widget_label_P(PSTR("WDir"), x, y);


	if (fc.wind.valid)
	{
		//int16_t relative_direction = fc.wind.direction - fc.gps_data.heading;	//use GPS heading
		int16_t relative_direction = (fc.wind.direction - 180) - fc.compass.azimuth;	//use Compass heading

		y += lh / 2;

		widget_arrow(relative_direction, x, y, w, h);
	}
	else
	{
		char tmp[5];
		strcpy_P(tmp, PSTR("---"));
		widget_value_int(tmp, x, y + lh, w, h - lh);
	}

}

void widget_wdir_points_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	uint8_t lh = widget_label_P(PSTR("WDir"), x, y);

	char tmp[5];
	if (fc.wind.valid)
		widget_deg_to_points(fc.wind.direction, tmp);
	else
		strcpy_P(tmp, PSTR("---"));


	widget_value_txt(tmp, x, y + lh, w, h - lh);
}

register_widget1(w_wind_dir, "Wind direction", widget_wdir_draw);
register_widget1(w_wind_dir_arrow, "Wind arrow", widget_wdir_arrow_draw);
register_widget1(w_wind_speed, "Wind speed", widget_wspd_draw);
register_widget1(w_wind_points, "Wind dir points", widget_wdir_points_draw);
