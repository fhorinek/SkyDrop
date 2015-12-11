#include "set_system.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"

void gui_set_system_init()
{
	gui_list_set(gui_set_system_item, gui_set_system_action, 6, GUI_SETTINGS);
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

void gui_set_system_auto_power_off_cb(float ret)
{
	uint8_t val = ret;

	config.system.auto_power_off = val;
	eeprom_busy_wait();
	eeprom_update_byte(&config_ee.system.auto_power_off, config.system.auto_power_off);

	gui_switch_task(GUI_SET_SYSTEM);
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
		config.connectivity.usb_mode = !config.connectivity.usb_mode;
		eeprom_busy_wait();
		eeprom_update_byte(&config_ee.connectivity.usb_mode, config.connectivity.usb_mode);
	break;

	case(4):
		gui_value_conf_P(PSTR("Auto power-off"), GUI_VAL_NUMBER_DISABLE, PSTR("%0.0f min"), config.system.auto_power_off, 0, 120, 1, gui_set_system_auto_power_off_cb);
		gui_switch_task(GUI_SET_VAL);
	break;

	case(5):
		config.connectivity.uart_function = (config.connectivity.uart_function + 1) % NUMBER_OF_UART_FORWARD;
		eeprom_busy_wait();
		eeprom_update_byte(&config_ee.connectivity.uart_function, config.connectivity.uart_function);
		uart_stop();
		uart_init();
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
			if (config.connectivity.usb_mode == USB_MODE_MASSSTORAGE)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;
		break;

		case (4):
			sprintf_P(text, PSTR("Auto power-off"));
			*flags |= GUI_LIST_SUB_TEXT;
			if (config.system.auto_power_off > 0)
				sprintf_P(sub_text, PSTR("%u min"), config.system.auto_power_off);
			else
				sprintf_P(sub_text, PSTR("disabled"));
		break;

		case (5):
			sprintf_P(text, PSTR("Uart function"));
			*flags |= GUI_LIST_SUB_TEXT;
			switch (config.connectivity.uart_function)
			{
				case(UART_FORWARD_DEBUG):
					sprintf_P(sub_text, PSTR("Debug msg"));
				break;
				case(UART_FORWARD_OFF):
					sprintf_P(sub_text, PSTR("Uart off"));
				break;
				case(UART_FORWARD_9600):
					sprintf_P(sub_text, PSTR("Telemetry 9600"));
				break;
				case(UART_FORWARD_19200):
					sprintf_P(sub_text, PSTR("Telemetry 19200"));
				break;
				case(UART_FORWARD_38400):
					sprintf_P(sub_text, PSTR("Telemetry 38400"));
				break;
				case(UART_FORWARD_57600):
					sprintf_P(sub_text, PSTR("Telemetry 57600"));
				break;
				case(UART_FORWARD_115200):
					sprintf_P(sub_text, PSTR("Telemetry 115200"));
				break;
			}
		break;

	}
}

