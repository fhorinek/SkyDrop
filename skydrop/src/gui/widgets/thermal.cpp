#include "thermal.h"

void widget_thermal_time_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	uint8_t lh = widget_label_P(PSTR("TTime"), x, y);

	uint16_t sec = fc.flight.circling_time;

	char text[10];
	if (sec < 60)
	    sprintf_P(text, PSTR("%02u"), sec);
	else
	    sprintf_P(text, PSTR("%02u:%02u"), sec / 60, sec % 60);

	widget_value_int(text, x, y + lh, w, h - lh);
}

void widget_thermal_gain_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
    uint8_t lh = widget_label_P(PSTR("TGain"), x, y);

    int16_t diff = fc.flight.circling_gain;

    if (config.altitude.alt1_flags & VARIO_UNITS_I)
    	diff *= FC_METER_TO_FEET;

    char text[10];
    sprintf_P(text, PSTR("%+d"), diff);

    widget_value_int(text, x, y + lh, w, h - lh);
}

void widget_thermal_assistant_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	if (!fc.flight.circling)
	{
		char text[16];
		sprintf_P(text, PSTR("Not circling"));
		widget_value_txt(text, x, y, w, h);

		return;
	}

	uint8_t s = min(w, h);
	uint8_t mx = x + w / 2;
	uint8_t my = y + h / 2;

	uint8_t max_val = VARIO_CIRCLING_HISTORY_SCALE; // +/- 1m

	for(uint8_t i = 0; i < 8; i++)
	{
		max_val = max(max_val, abs(fc.flight.circling_history[i]));
	}

	for(uint8_t i = 0; i < 8; i++)
	{
		if (i == ((fc.gps_data.heading + 22) % 360) / 45)
		{
			int16_t angle = fc.gps_data.heading + 90;

			float fsin = disp.get_sin(angle);
			float fcos = disp.get_cos(angle);
			uint8_t x1 = mx + fsin * s / 3;
			uint8_t y1 = my + fcos * s / 3;

			angle += fc.flight.total_heading_change > 0 ? +90 : -90;

			uint8_t size = s / 3;
			widget_arrow(angle, x1 - size / 2, y1 - size / 2, size, size);
		}
		else
		{
			int16_t angle = 90 + i * 45;

			float fsin = disp.get_sin(angle);
			float fcos = disp.get_cos(angle);
			uint8_t x1 = mx + fsin * s / 3;
			uint8_t y1 = my + fcos * s / 3;

			int8_t rad = (fc.flight.circling_history[i] * (s / 8)) / max_val;

			if (rad > 0)
				disp.DrawCircle(x1, y1, rad, 1);
			else
			{
				disp.DrawLine(x1 - rad, y1 - rad, x1 + rad, y1 + rad, 1);
				disp.DrawLine(x1 - rad, y1 + rad, x1 + rad, y1 - rad, 1);
			}
		}
	}


}


register_widget1(w_thermal_time, "Thermal time", widget_thermal_time_draw);
register_widget1(w_thermal_gain, "Thermal gain", widget_thermal_gain_draw);
register_widget1(w_thermal_ass, "Thermal assistant", widget_thermal_assistant_draw);
