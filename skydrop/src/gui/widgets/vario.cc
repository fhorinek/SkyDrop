#include "vario.h"

void widget_vario_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label(PSTR("Vario"), x, y);

	float val = fc.digital_vario;
	if (abs(val) < 0.09)
		val = 0;

	char text[10];
	if (fc.baro_valid)
		sprintf(text, "%0.1f", val);
	else
		sprintf(text, "-.-", val);
	widget_value_int(text, x, y + lh, w, h - lh);
}

void widget_avg_vario_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label(PSTR("Avg"), x, y);

	float val = fc.avg_vario;
	if (abs(val) < 0.09)
		val = 0;

	char text[10];
	if (fc.baro_valid)
		sprintf(text, "%0.1f", val);
	else
		sprintf(text, "-.-", val);
	widget_value_int(text, x, y + lh, w, h - lh);
}

void widget_vario_bar_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	disp.DrawRectangle(x, y, x + w - 1, y + h - 1, 1, 0);

	//middle
	uint8_t m_line =  y + h / 2;
	disp.DrawLine(x, m_line, x + w - 1, m_line, 1);
	//bar lines
	uint8_t y_line = m_line;
	uint8_t y_space = h / 6;
	disp.DrawLine(x + (w / 3) * 2, y_line - y_space * 1, x + w - 1, y_line - y_space * 1, 1);
	disp.DrawLine(x + (w / 3) * 2, y_line - y_space * 2, x + w - 1, y_line - y_space * 2, 1);
	disp.DrawLine(x, y_line + y_space * 1, x + w / 3, y_line + y_space * 1, 1);
	disp.DrawLine(x, y_line + y_space * 2, x + w / 3, y_line + y_space * 2, 1);

	float vario;

	vario = fc.vario;

//	if (flight_data.baro_valid)
//		vario = flight_data.vario;
//	else
//		vario = 0;


	if (abs(vario) > 0.09)
	{
		uint8_t y_top = 0;
		uint8_t y_bottom = 0;


		if (vario > 0)
		{
			if (vario <= 3)
			{
				y_bottom  = h / 2 - 1;
				y_top = (h / 2) - (vario * ((h / 2) - 2)) / 3;
			}
			else
			{
				if (vario > 6)
					vario = 6;

				y_bottom = (h / 2) - ((vario - 3) * ((h / 2) - 1)) / 3;
				y_top = 2;
			}
		}
		else
		{
			vario *= -1;
			if (vario <= 3)
			{
				y_top  = h / 2 + 1;
				y_bottom = (h / 2) + (vario * ((h / 2) - 3)) / 3 + 1;
			}
			else
			{
				if (vario > 6)
					vario = 6;

				y_top = (h / 2) + ((vario - 3) * ((h / 2) - 1)) / 3 + 1;
				y_bottom = h - 3;
			}
		}

		if (y_top < 2)
			y_top = 2;
		if (y_top > h - 4)
			y_top = h - 4;

		if (y_bottom < 3)
			y_bottom = 3;
		if (y_bottom > h - 2)
			y_bottom = h - 2;


		disp.Invert(x + 2, y + y_top, x + w - 3, y + y_bottom);
	}
}

register_widget1(w_vario, "Vario", widget_vario_draw);
register_widget1(w_avg_vario, "AVG vario", widget_avg_vario_draw);
register_widget1(w_vario_bar, "Vario bar", widget_vario_bar_draw);
