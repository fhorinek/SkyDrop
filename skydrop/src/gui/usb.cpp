#include "usb.h"

#include "../fc/fc.h"

void gui_usb_set_mode(uint8_t mode)
{
}

void gui_usb_init() {}

void gui_usb_stop() {}

void gui_usb_loop()
{
	char tmp[16];

	disp.LoadFont(F_TEXT_L);
	uint8_t f_h = disp.GetTextHeight();
	strcpy_P(tmp, PSTR("USB mode"));
	gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, GUI_DISP_HEIGHT / 2 - f_h / 2);


	disp.LoadFont(F_TEXT_S);
	f_h = disp.GetTextHeight();

	strcpy_P(tmp, PSTR("PWR"));
	gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, GUI_DISP_HEIGHT - f_h);

//	strcpy_P(tmp, PSTR("RST"));
//	gui_raligh_text(tmp, GUI_DISP_WIDTH - 1, GUI_DISP_HEIGHT - f_h);


	sprintf_P(tmp, PSTR("batt %d %%"), battery_per);
	gui_raligh_text(tmp, GUI_DISP_WIDTH - 1, 0);

	disp.GotoXY(0, 0);
	fprintf_P(lcd_out, PSTR("build %04d"), BUILD_NUMBER);
	disp.GotoXY(0, 7);
	fprintf_P(lcd_out, PSTR("rev %04d"), (hw_revision == HW_REW_1504) ? 1504 : 1506);
}

void gui_usb_irqh(uint8_t type, uint8_t * buff)
{
	switch (type)
	{
	case(TASK_IRQ_BUTTON_M):
		if (*buff == BE_LONG)
		{
			task_set(TASK_ACTIVE);
		}
	break;

	case(TASK_IRQ_BUTTON_R):
		if (*buff == BE_LONG)
		{
			SystemReset();
		}
	break;

	}
}


