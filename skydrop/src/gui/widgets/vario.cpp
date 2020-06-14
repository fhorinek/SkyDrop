#include "vario.h"

void widget_vario_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	uint8_t lh = widget_label_P(PSTR("Vario"), x, y);

	float val = fc.vario.digital;

	if (config.vario.flags & VARIO_UNITS_I)
		val *= FC_MPS_TO_100FPM;

	if (abs(val) < 0.09)
		val = 0;

	char text[10];
	if (fc.vario.valid)
		sprintf_P(text, PSTR("%0.1f"), val);
	else
		sprintf_P(text, PSTR("-.-"), val);
	widget_value_int(text, x, y + lh, w, h - lh);
}

void widget_avg_vario_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	uint8_t lh = widget_label_P(PSTR("Avg"), x, y);

	float val = fc.vario.avg;

	if (config.vario.flags & VARIO_UNITS_I)
		val *= FC_MPS_TO_100FPM;

	if (abs(val) < 0.09)
		val = 0;

	char text[10];
	if (fc.vario.valid)
		sprintf_P(text, PSTR("%0.1f"), val);
	else
		sprintf_P(text, PSTR("-.-"), val);
	widget_value_int(text, x, y + lh, w, h - lh);
}

void widget_vario_bar_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
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

	vario = fc.vario.vario;

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

int8_t clamp(float in, uint8_t limit)
{
	if (in > limit) return limit;
	if (in < -limit) return -limit;
	return in;
}

void widget_vario_history_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	uint8_t lh = 0;//widget_label_P(PSTR("VHist"), x, y);

	uint8_t max = VARIO_HISTORY_SCALE / 2;
	for (uint8_t i = 0; i < VARIO_HISTORY_SIZE; i++)
	{
		if (abs(fc.vario.history[i]) > max)
			max = abs(fc.vario.history[i]);
	}

	max /= VARIO_HISTORY_SCALE / 2;
	max++;
	max *= VARIO_HISTORY_SCALE / 2;

	uint8_t middle = y + lh + (h - lh) / 2;
	uint8_t max_h = (h - lh) / 2 - 1;
	uint8_t ptr = fc.vario.history_pointer;
	float step_x = (w - 1) / (float)(VARIO_HISTORY_SIZE - 1);
	float step_y = -max_h / (float)max;

	uint8_t old_y = clamp(fc.vario.history[ptr] * step_y, max_h) + middle;
	for (uint8_t i = 1; i < VARIO_HISTORY_SIZE - 1; i++)
	{
		ptr = (ptr + 1) % VARIO_HISTORY_SIZE;
		uint8_t new_y = clamp(fc.vario.history[ptr] * step_y, max_h) + middle;

		disp.DrawLine(x + (i - 1) * step_x, old_y, x + i * step_x, new_y, 1);
		old_y = new_y;
	}

	//1m
	for (uint8_t ix = x; ix < x + w - 1; ix++)
		if (ix % 2)
			disp.PutPixel(ix, middle + VARIO_HISTORY_SCALE * step_y, 1);

	//middle line
	disp.DrawLine(x, middle, x + w - 1, middle, 1);
}

register_widget1(w_vario, "Vario", widget_vario_draw);
register_widget1(w_avg_vario, "AVG vario", widget_avg_vario_draw);
register_widget1(w_vario_bar, "Vario bar", widget_vario_bar_draw);
register_widget1(w_vario_history, "Vario history", widget_vario_history_draw);
