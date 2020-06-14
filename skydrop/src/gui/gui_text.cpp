#include "gui_text.h"

#define GUI_TEXT_BUFFER_LEN	64
char gui_text_buffer[GUI_TEXT_BUFFER_LEN];

uint8_t gui_text_max_len;
uint8_t gui_text_len;
uint8_t gui_text_pos;
uint8_t gui_text_mode;
uint8_t gui_text_index;

uint8_t_cb * gui_text_cb;

void gui_text_conf(char * text, uint8_t max_len, uint8_t_cb * cb)
{
	strcpy(gui_text_buffer, text);
	gui_text_max_len = max_len;
	gui_text_cb = cb;
	gui_text_len = strlen(text);
	gui_text_pos = gui_text_len;
	gui_text_index = 0;

	if (gui_text_len == 0)
		gui_text_mode = 1;
}

char gui_text_gen_c(uint8_t i, uint8_t mode)
{
	if (i <= 25)
	{
		if (mode == 0)
			return 'a' + i;
		if (mode == 1)
			return 'A' + i;
		if (mode == 2)
		{
			if (i <= 10)
				return '0' + i;
			if (i <= 22)
				return '!' + (i - 10);
			if (i == 23)
				return ':';
			if (i == 24)
				return '=';
			if (i == 25)
				return '_';
		}
	}
	if (i == 26)
		return ' ';
	if (i == 27)
		return '.';

	return 0;
}

void gui_text_del()
{
	if (gui_text_pos == 0)
		return;

	strcpy(gui_text_buffer + (gui_text_pos - 1), gui_text_buffer + gui_text_pos);

	gui_text_len--;
	gui_text_pos--;
}

void gui_text_loop()
{

	if (GUI_BLINK_TGL(50))
	{
		if (button_hold(B_LEFT))
			if (gui_text_index > 0)
				gui_text_index--;

		if (button_hold(B_RIGHT))
			if (gui_text_index < 32)
				gui_text_index++;

		if (button_hold(B_MIDDLE))
		{
			if (gui_text_index == 29)
				if (gui_text_pos > 0)
					gui_text_pos--;

			if (gui_text_index == 30)
				if (gui_text_pos < gui_text_len)
					gui_text_pos++;

			if (gui_text_index == 31)
				gui_text_del();
		}
	}


	disp.LoadFont(F_TEXT_M);
	uint8_t f_h = disp.GetTextHeight();

	disp.DrawRectangle(0, 0, GUI_DISP_WIDTH - 1, f_h +1, 1, 0);
	gui_text_buffer[gui_text_len] = 0;
	disp.GotoXY(2,2);

	uint8_t pad = 0;

	while (disp.GetTextWidth(gui_text_buffer + pad, gui_text_pos - pad) > (GUI_DISP_WIDTH * 5) / 6)
		pad++;

	fprintf_P(lcd_out, PSTR("%s"), gui_text_buffer + pad);
	if (GUI_BLINK_TGL(400))
	{
		uint8_t w = disp.GetTextWidth(gui_text_buffer + pad, gui_text_pos - pad);
		disp.DrawLine(w + 1, 2, w + 1, f_h - 1, 1);
	}

	uint8_t x, y;
	char c[4];

	for (uint8_t i = 0; i < 28; i++)
	{
		x = 8 * (i % 7);
		y = 9 * (i / 7) + f_h + 3;

		c[0] = gui_text_gen_c(i, gui_text_mode);
		c[1] = 0;

		gui_caligh_text(c, x + 5, y);

		if (i == gui_text_index)
			disp.Invert(x, y - 1, x + 8, y + 8);
	}

	//mode indicatior
	x = 72;
	y = f_h + 3;

	sprintf_P(c, PSTR("%d/3"), gui_text_mode + 1);
	gui_caligh_text(c, x, y);
	if (gui_text_index == 28)
		disp.Invert(x - 10, y - 1, x + 10, y + 7);

	//arrows
	x = 8 * 8 - 2;
	y = f_h + 3 + 9;

	strcpy_P(c, PSTR("<"));
	gui_caligh_text(c, x + 5, y);
	if (gui_text_index == 29)
		disp.Invert(x, y - 1, x + 8, y + 8);

	x = 8 * 9 + 2;
	y = f_h + 3 + 9;

	strcpy_P(c, PSTR(">"));
	gui_caligh_text(c, x + 5, y);
	if (gui_text_index == 30)
		disp.Invert(x, y - 1, x + 8, y + 8);


	//delete
	x = 72;
	y = f_h + 3 + 9 * 2;

	strcpy_P(c, PSTR("DEL"));
	gui_caligh_text(c, x, y);
	if (gui_text_index == 31)
		disp.Invert(x - 10, y - 1, x + 10, y + 7);

	//OK
	x = 72;
	y = f_h + 3 + 9 * 3;

	strcpy_P(c, PSTR("OK"));
	gui_caligh_text(c, x, y);
	if (gui_text_index == 32)
		disp.Invert(x - 10, y - 1, x + 10, y + 7);
}

void gui_text_add(char c)
{
	if (gui_text_len >= gui_text_max_len - 1)
		return;

	if (gui_text_len == 0)
	{
		gui_text_buffer[0] = c;
		gui_text_buffer[1] = 0;
	}
	else
	{
		for (uint8_t i = gui_text_len; i >= gui_text_pos; i--)
			gui_text_buffer[i + 1] = gui_text_buffer[i];
		gui_text_buffer[gui_text_pos] = c;
	}

	gui_text_len++;
	gui_text_pos++;
}


void gui_text_irqh(uint8_t type, uint8_t * buff)
{
	switch (type)
	{
		case(TASK_IRQ_BUTTON_L):
			if (*buff == BE_CLICK)
				if (gui_text_index > 0)
					gui_text_index--;
		break;

		case(TASK_IRQ_BUTTON_R):
			if (*buff == BE_CLICK)
				if (gui_text_index < 32)
					gui_text_index++;
		break;

		case(TASK_IRQ_BUTTON_M):
			if (*buff == BE_CLICK)
			{
				if (gui_text_index < 28)
				{
					char c = gui_text_gen_c(gui_text_index, gui_text_mode);
					gui_text_add(c);

					if (gui_text_mode == 1)
						gui_text_mode = 0;

					if (c == ' ' && gui_text_mode == 0)
						gui_text_mode = 1;
				}

				if (gui_text_index == 28)
					gui_text_mode = (gui_text_mode + 1) % 3;

				if (gui_text_index == 29)
					if (gui_text_pos > 0)
						gui_text_pos--;

				if (gui_text_index == 30)
					if (gui_text_pos < gui_text_len)
						gui_text_pos++;

				if (gui_text_index == 31)
					gui_text_del();

				if (gui_text_index == 32)
					gui_text_cb(GUI_TEXT_OK, gui_text_buffer);
			}

			if (*buff == BE_LONG)
			{
				if (gui_text_index != 29 && gui_text_index != 30 && gui_text_index != 31)
					gui_text_cb(GUI_TEXT_CANCEL, gui_text_buffer);
			}
		break;
	}
}
