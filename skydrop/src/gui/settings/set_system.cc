#include "set_system.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"

void gui_set_system_init()
{
	gui_list_set(gui_set_system_item, gui_set_system_action, 5);

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

void gui_set_system_time_cb(float val)
{
	gui_switch_task(GUI_SET_SYSTEM);
}

void gui_set_system_timezone_cb(float val)
{
	int8_t tmp = val * 2;
	eeprom_busy_wait();
	eeprom_update_block((void *)&fc.time_zone, &config.system.time_zone, sizeof(int8_t));
	fc.time_zone = tmp;
	gui_switch_task(GUI_SET_SYSTEM);
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
		gui_value_conf_P(PSTR("Time"), GUI_VAL_TIME, PSTR(""), 0, 0, 0, 1, gui_set_system_time_cb);
		gui_switch_task(GUI_SET_VAL);
	break;

	case(2):
		gui_value_conf_P(PSTR("Date"), GUI_VAL_DATE, PSTR(""), 0, 0, 0, 1, gui_set_system_time_cb);
		gui_switch_task(GUI_SET_VAL);
	break;

	case(3):
		gui_value_conf_P(PSTR("Time zone"), GUI_VAL_NUMBER, PSTR("UTC %+0.1f"), fc.time_zone / 2.0, -12, 12, 0.5, gui_set_system_timezone_cb);
		gui_switch_task(GUI_SET_VAL);
	break;

	case(4):
		gui_switch_task(GUI_SETTINGS);
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
			sprintf_P(text, PSTR("Mass Storage"));
			if (fc.usb_mode == USB_MODE_MASSSTORAGE)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;

		break;

		case (1):
			sprintf_P(text, PSTR("Time"));
			sprintf_P(sub_text, PSTR("%02d:%02d.%02d"), hour, min, sec);
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (2):
			sprintf_P(text, PSTR("Date"));
			sprintf_P(sub_text, PSTR("%02d/%02d/%04d"), day, month, year);
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (3):
			sprintf_P(text, PSTR("Time Zone"));
			sprintf_P(sub_text, PSTR("UTC %+0.1f"), fc.time_zone / 2.0);
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (4):
			sprintf_P(text, PSTR("back"));
			*flags |= GUI_LIST_BACK;
		break;

	}
}

