#include "gui_dialog.h"

char gui_dialog_msg_line0[21];
char gui_dialog_msg_line1[21];
char gui_dialog_msg_line2[21];
char gui_dialog_msg_line3[21];
uint8_t gui_dialog_style;
uint8_cb * gui_dialog_cb;

void gui_dialog_set_P(const char * title, const char * msg, uint8_t style, uint8_cb * cb)
{
	char tmp1[21];
	char tmp2[64];

	strcpy_P(tmp1, title);
	strcpy_P(tmp2, msg);

	gui_dialog_set(tmp1, tmp2, style, cb);
}

void gui_dialog_set(char * title, char * msg, uint8_t style, uint8_cb * cb)
{
	strcpy(gui_dialog_msg_line0, title);

	gui_dialog_style = style;
	gui_dialog_cb = cb;

	char * ptr;
	if ((ptr = strchr(msg, '\n')) != NULL)
	{
		memcpy(gui_dialog_msg_line1, msg, ptr - msg + 1);
		gui_dialog_msg_line1[ptr - msg] = 0;
		msg = ptr + 1;
		if ((ptr = strchr(msg, '\n')) != NULL)
		{
			memcpy(gui_dialog_msg_line2, msg, ptr - msg + 1);
			gui_dialog_msg_line2[ptr - msg] = 0;
			strcpy(gui_dialog_msg_line3, ptr + 1);
		}
		else
		{
			strcpy(gui_dialog_msg_line2, msg);
			gui_dialog_msg_line3[0] = 0;
		}
	}
	else
	{
		strcpy(gui_dialog_msg_line1, msg);
		gui_dialog_msg_line2[0] = 0;
		gui_dialog_msg_line3[0] = 0;
	}
}

void gui_dialog_init() {}
void gui_dialog_stop() {}

void gui_dialog_loop()
{
	if (gui_dialog_style & GUI_STYLE_NO_TITLE)
	{
		//no title mode (GUI_STYLE_NO_TITLE is set)
		disp.DrawRectangle(0, 0, GUI_DISP_WIDTH - 2, GUI_DISP_HEIGHT - 2, 1, false);
	}
	else
	{
		//normal mode
		gui_dialog(gui_dialog_msg_line0);
	}

	disp.LoadFont(F_TEXT_M);
	uint8_t f_h = disp.GetTextHeight();

	if ((gui_dialog_style & GUI_STYLE_MASK) == GUI_STYLE_STATS)
	{
		//if new flight has begin exit the stat screen
		if (fc.flight.state != FLIGHT_LAND)
		{
			gui_dialog_cb(1);
		}

		uint32_t diff = fc.flight.timer / 1000;
		uint8_t hour, min;

		hour = diff / 3600;
		diff %= 3600;

		min = diff / 60;
		diff %= 60;

		char tmp[32];

		disp.GotoXY(GUI_DIALOG_LEFT, GUI_DIALOG_TOP + f_h * 0);
		if (hour > 0)
			fprintf_P(lcd_out, PSTR("%02d:%02d"), hour, min);
		else
			fprintf_P(lcd_out, PSTR("%02d.%02d"), min, diff);

		strcpy_P(tmp, PSTR("Alt"));
		gui_raligh_text(tmp, (GUI_DIALOG_WIDTH * 2) / 3 - 2, GUI_DIALOG_TOP + f_h * 0);
		strcpy_P(tmp, PSTR("Vario"));
		gui_raligh_text(tmp, GUI_DIALOG_RIGHT + 1, GUI_DIALOG_TOP + f_h * 0);

		sprintf_P(tmp, PSTR("%dm"), fc.flight.stats.max_alt);
		gui_raligh_text(tmp, (GUI_DIALOG_WIDTH * 2) / 3 - 2, GUI_DIALOG_TOP + f_h * 1);
		sprintf_P(tmp, PSTR("%dm"), fc.flight.stats.min_alt);
		gui_raligh_text(tmp, (GUI_DIALOG_WIDTH * 2) / 3 - 2, GUI_DIALOG_TOP + f_h * 2);

		sprintf_P(tmp, PSTR("%0.1fm"), (float)fc.flight.stats.max_climb / 100.0);
		gui_raligh_text(tmp, GUI_DIALOG_RIGHT + 1, GUI_DIALOG_TOP + f_h * 1);
		sprintf_P(tmp, PSTR("%0.1fm"), (float)fc.flight.stats.max_sink / 100.0);
		gui_raligh_text(tmp, GUI_DIALOG_RIGHT + 1, GUI_DIALOG_TOP + f_h * 2);

		disp.LoadFont(F_TEXT_S);
		disp.GotoXY(GUI_DIALOG_LEFT, GUI_DIALOG_TOP + f_h * 1 + 2);
		fprintf_P(lcd_out, PSTR("max"));
		disp.GotoXY(GUI_DIALOG_LEFT, GUI_DIALOG_TOP + f_h * 2 + 2);
		fprintf_P(lcd_out, PSTR("min"));
	}
	else
	{
		uint8_t top;

		if (gui_dialog_style & GUI_STYLE_NO_TITLE)
		{
			//no title mode (GUI_STYLE_NO_TITLE is set)
			top = 3;

			disp.GotoXY(GUI_DIALOG_LEFT, top + f_h * 0);
			fprintf_P(lcd_out, PSTR("%s"), gui_dialog_msg_line0);
		}
		else
		{
			//normal mode
			top = GUI_DIALOG_TOP - f_h;
		}

		disp.GotoXY(GUI_DIALOG_LEFT, top + f_h * 1);
		fprintf_P(lcd_out, PSTR("%s"), gui_dialog_msg_line1);

		disp.GotoXY(GUI_DIALOG_LEFT, top + f_h * 2);
		fprintf_P(lcd_out, PSTR("%s"), gui_dialog_msg_line2);

		disp.GotoXY(GUI_DIALOG_LEFT, top + f_h * 3);
		fprintf_P(lcd_out, PSTR("%s"), gui_dialog_msg_line3);
	}

	disp.LoadFont(F_TEXT_S);
	f_h = disp.GetAHeight();

	switch (gui_dialog_style & GUI_STYLE_MASK)
	{
		case(GUI_STYLE_OK):
		case(GUI_STYLE_STATS):
			gui_caligh_text_P(PSTR("OK"), GUI_DIALOG_LEFT + GUI_DIALOG_WIDTH / 2, GUI_DIALOG_BOTTOM - f_h);
		break;

		case(GUI_STYLE_OKCANCEL):
			gui_caligh_text_P(PSTR("OK"), GUI_DIALOG_LEFT + GUI_DIALOG_WIDTH / 2, GUI_DIALOG_BOTTOM - f_h);
			gui_raligh_text_P(PSTR("Cancel"), GUI_DIALOG_RIGHT - 1, GUI_DIALOG_BOTTOM - f_h);
		break;

		case(GUI_STYLE_YESNO):
			gui_caligh_text_P(PSTR("Yes"), GUI_DIALOG_LEFT + GUI_DIALOG_WIDTH / 2, GUI_DIALOG_BOTTOM - f_h);
			gui_raligh_text_P(PSTR("No"), GUI_DIALOG_RIGHT - 1, GUI_DIALOG_BOTTOM - f_h);
		break;

		case(GUI_STYLE_TIMESET):
			gui_caligh_text_P(PSTR("GPS"), GUI_DIALOG_LEFT + GUI_DIALOG_WIDTH / 2, GUI_DIALOG_BOTTOM - f_h);
			gui_raligh_text_P(PSTR("Manual"), GUI_DIALOG_RIGHT - 1, GUI_DIALOG_BOTTOM - f_h);
		break;

		case(GUI_STYLE_FORMAT):
			gui_caligh_text_P(PSTR("No"), GUI_DIALOG_LEFT + GUI_DIALOG_WIDTH / 2, GUI_DIALOG_BOTTOM - f_h);
			gui_raligh_text_P(PSTR("Yes"), GUI_DIALOG_RIGHT - 1, GUI_DIALOG_BOTTOM - f_h);
		break;
	}
}

void gui_dialog_irqh(uint8_t type, uint8_t * buff)
{
	switch (type)
	{
		case(TASK_IRQ_BUTTON_L):
			if (*buff == BE_CLICK)
				gui_dialog_cb(0);
		break;

		case(TASK_IRQ_BUTTON_M):
			if (*buff == BE_CLICK)
				gui_dialog_cb(1);
		break;

		case(TASK_IRQ_BUTTON_R):
			if (*buff == BE_CLICK)
				gui_dialog_cb(2);
		break;
	}
}

