#include "usb.h"

#include "../fc/fc.h"
#include "../task_usb/task_usb.h"

uint16_t usb_loop_time = 0;

#define USB_LED_MAX		0x10

void gui_usb_set_mode(uint8_t mode)
{
}

void gui_usb_init() {}

void gui_usb_stop() {}

void gui_usb_loop()
{
	char tmp[16];

	if (battery_per == BATTERY_FULL)
		led_set(0, USB_LED_MAX, 0);
	else
		led_set(USB_LED_MAX, 0, 0);

	disp.LoadFont(F_TEXT_L);
	uint8_t f_h = disp.GetTextHeight();
	if (task_usb_sd_ready)
	{
		if (usb_int_state == USB_NOT_RDY)
		{
			if (battery_per == BATTERY_FULL)
				sprintf_P(tmp, PSTR("Battery full"), battery_per);
			else
				sprintf_P(tmp, PSTR("Charging"), battery_per);
		}
		else
		{
			strcpy_P(tmp, PSTR("USB mode"));
		}

		gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, GUI_DISP_HEIGHT / 2 - f_h + 2);
	}
	else
	{
		strcpy_P(tmp, PSTR("No SD card"));
		gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, GUI_DISP_HEIGHT / 2 - f_h / 2);
	}

	disp.LoadFont(F_TEXT_S);
	f_h = disp.GetTextHeight();

	if (usb_int_state != USB_NOT_RDY)
	{
		disp.SetDrawLayer(1);
		disp.CopyToLayerXPart(0, -1, 3, 6);
		disp.SetDrawLayer(0);
		disp.DrawLine(GUI_DISP_WIDTH - 2, 39 - usb_loop_time, GUI_DISP_WIDTH - 2, 39, 1);
		disp.CopyToLayerPart(1, 3, 0, 6, GUI_DISP_WIDTH - 1);

		if (usb_loop_time < 1)
			strcpy_P(tmp, PSTR("idle"));
		else
			strcpy_P(tmp, PSTR("busy"));

		gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, GUI_DISP_HEIGHT / 2 + f_h);
	}

	strcpy_P(tmp, PSTR("PWR"));
	gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, GUI_DISP_HEIGHT - f_h);

//	strcpy_P(tmp, PSTR("RST"));
//	gui_raligh_text(tmp, GUI_DISP_WIDTH - 1, GUI_DISP_HEIGHT - f_h);

	if (usb_int_state != USB_NOT_RDY)
	{
		if (battery_per == BATTERY_FULL)
			sprintf_P(tmp, PSTR("Full"), battery_per);
		else
			sprintf_P(tmp, PSTR("Charging"), battery_per);

		gui_raligh_text(tmp, GUI_DISP_WIDTH - 1, 0);
	}

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
			task_set(TASK_POWERDOWN);
		}
	break;

	}
}


