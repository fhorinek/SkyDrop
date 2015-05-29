#include "usb.h"

void gui_usb_set_mode(uint8_t mode)
{
}

void gui_usb_init() {}

void gui_usb_stop() {}

void gui_usb_loop()
{
	char tmp[] = "USB mode";

	disp.LoadFont(F_TEXT_L);
	uint8_t f_h = disp.GetTextHeight();
	gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, GUI_DISP_HEIGHT / 2 - f_h / 2);

}

void gui_usb_irqh(uint8_t type, uint8_t * buff)
{

}


