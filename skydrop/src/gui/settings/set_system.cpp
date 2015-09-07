#include "set_system.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"

void gui_set_system_init()
{
	gui_list_set(gui_set_system_item, gui_set_system_action, 4, GUI_SETTINGS);
}

void gui_set_system_stop() {}

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
		gui_switch_task(GUI_SET_TIME);
	break;

	case(1):
		gui_switch_task(GUI_SET_DISPLAY);
	break;

	case(2):
		gui_switch_task(GUI_SET_AUDIO);
	break;

	case(3):
		config.system.usb_mode = !config.system.usb_mode ;
		eeprom_busy_wait();
		eeprom_update_byte(&config_ee.system.usb_mode, config.system.usb_mode );
	break;

	}
}

void gui_set_system_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t day;
	uint8_t wday;
	uint8_t month;
	uint16_t year;

	datetime_from_epoch(time_get_actual(), &sec, &min, &hour, &day, &wday, &month, &year);

	switch (index)
	{
		case (0):
			sprintf_P(text, PSTR("Time & Date"));
			*flags |= GUI_LIST_FOLDER;
		break;

		case (1):
			sprintf_P(text, PSTR("Display"));
			*flags |= GUI_LIST_FOLDER;
		break;

		case (2):
			sprintf_P(text, PSTR("Audio"));
			*flags |= GUI_LIST_FOLDER;
		break;


		case (3):
			sprintf_P(text, PSTR("Mass Storage"));
			if (config.system.usb_mode == USB_MODE_MASSSTORAGE)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;
		break;
	}
}

