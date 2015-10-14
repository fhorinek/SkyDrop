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

		switch(config.system.gps_format_flags & GPS_SPD_MASK)
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

	char tmp1[16];
	char tmp2[16];

	float decimal;
	float deg;
	float min;
	float sec;
	uint8_t c;

	if (fc.gps_data.valid)
	{
		switch (config.system.gps_format_flags & GPS_FORMAT_MASK)
		{
			case(GPS_DDdddddd):
				sprintf_P(tmp1, PSTR("%+02.6f"), fc.gps_data.latitude);
				sprintf_P(tmp2, PSTR("%+03.6f"), fc.gps_data.longtitude);
			break;
			case(GPS_DDMMmmm):
				c = (fc.gps_data.latitude < 0) ? 'S' : 'N';
				decimal = abs(fc.gps_data.latitude);
				deg = floor(decimal);
				min = (decimal - deg) * 60;
				sprintf_P(tmp1, PSTR("%02.0f*%02.3f%c"), deg, min, c);

				c = (fc.gps_data.longtitude < 0) ? 'W' : 'E';
				decimal = abs(fc.gps_data.longtitude);
				deg = floor(decimal);
				min = (decimal - deg) * 60;
				sprintf_P(tmp2, PSTR("%03.0f*%02.3f%c"), deg, min, c);
			break;
			case(GPS_DDMMSS):
				c = (fc.gps_data.latitude < 0) ? 'S' : 'N';
				decimal = abs(fc.gps_data.latitude);
				deg = floor(decimal);
				min = floor((decimal - deg) * 60);
				sec = floor((decimal - deg - min / 60) * 3600);
				sprintf_P(tmp1, PSTR("%02.0f*%02.0f'%02.0f\"%c"), deg, min, sec, c);

				c = (fc.gps_data.longtitude < 0) ? 'W' : 'E';
				decimal = abs(fc.gps_data.longtitude);
				deg = floor(decimal);
				min = floor((decimal - deg) * 60);
				sec = floor((decimal - deg - min / 60) * 3600);
				sprintf_P(tmp2, PSTR("%03.0f*%02.0f'%02.0f\"%c"), deg, min, sec, c);

			break;
		}

		widget_value_txt2(tmp1, tmp2, x, y + lh, w, h - lh);
	}
	else
	{
		sprintf_P(tmp1, PSTR("---"));
		widget_value_int(tmp1, x, y + lh, w, h - lh);
	}


}

register_widget1(w_ghdg, "GPS Heading", widget_ghdg_draw);
register_widget1(w_gspd, "Ground Speed", widget_gspd_draw);
register_widget1(w_gpos, "GPS position", widget_gpos_draw);

