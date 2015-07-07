#include "gui_dialog.h"

char gui_dialog_title[16];
char gui_dialog_msg_line1[20];
char gui_dialog_msg_line2[20];
char gui_dialog_msg_line3[20];
uint8_t gui_dialog_style;
uint8_cb * gui_dialog_cb;

void gui_dialog_set_P(const char * title, const char * msg, uint8_t style, uint8_cb * cb)
{
	char tmp1[16];
	char tmp2[64];

	strcpy_P(tmp1, title);
	strcpy_P(tmp2, msg);

	gui_dialog_set(tmp1, tmp2, style, cb);
}

void gui_dialog_set(char * title, char * msg, uint8_t style, uint8_cb * cb)
{
	strcpy(gui_dialog_title, title);

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
	gui_dialog(gui_dialog_title);
	disp.LoadFont(F_TEXT_M);
	uint8_t f_h = disp.GetTextHeight();

	disp.GotoXY(GUI_DIALOG_LEFT, GUI_DIALOG_TOP + f_h * 0);
	fprintf_P(lcd_out, PSTR("%s"), gui_dialog_msg_line1);

	disp.GotoXY(GUI_DIALOG_LEFT, GUI_DIALOG_TOP + f_h * 1);
	fprintf_P(lcd_out, PSTR("%s"), gui_dialog_msg_line2);

	disp.GotoXY(GUI_DIALOG_LEFT, GUI_DIALOG_TOP + f_h * 2);
	fprintf_P(lcd_out, PSTR("%s"), gui_dialog_msg_line3);

	disp.LoadFont(F_TEXT_S);
	f_h = disp.GetAHeight();

	switch (gui_dialog_style)
	{
		case(GUI_STYLE_OK):
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

