#include "set_system.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"

void gui_set_system_init()
{
	gui_list_set(gui_set_system_item, gui_set_system_action, 2);

	eeprom_busy_wait();

}

void gui_set_system_stop()
{
}

void gui_set_system_loop()
{
	gui_list_draw();
}

void gui_set_system_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_set_system_action(uint8_t index)
{
	switch(index)
	{
	case(0):
		fc.usb_mode = !fc.usb_mode;
		eeprom_busy_wait();
		eeprom_update_byte(&config.system.usb_mode, fc.usb_mode);
	break;

	case(1):
		gui_switch_task(GUI_SETTINGS);
	break;
	}
}

void gui_set_system_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			sprintf_P(text, PSTR("Mass Storage"));
			if (fc.usb_mode == USB_MODE_MASSSTORAGE)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;

		break;

		case (1):
			sprintf_P(text, PSTR("back"));
			*flags |= GUI_LIST_BACK;
		break;

	}
}

