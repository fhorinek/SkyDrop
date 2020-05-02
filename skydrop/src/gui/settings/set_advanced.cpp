#include "set_advanced.h"

#include "../gui_list.h"
#include "../gui_value.h"
#include "../gui_dialog.h"
#include "gui_accel_calib.h"

#include "../../fc/conf.h"
#include "../../drivers/storage/storage.h"

void gui_set_advanced_init()
{
	gui_list_set(gui_set_advanced_item, gui_set_advanced_action, 5, GUI_SET_SYSTEM);
}

void gui_set_advanced_restore_cfg_cb(uint8_t ret)
{
	if (ret == GUI_DIALOG_FORMAT)
	{
		gui_showmessage_P(PSTR("Restoring default\nconfiguration..."));
		gui_force_loop();
		cfg_restore_defaults();
		gui_showmessage_P(PSTR("Done"));
	}
	gui_switch_task(GUI_SET_ADVANCED);
}

void gui_set_advanced_action(uint8_t index)
{
	switch(index)
	{
	case(0):
	{
		char id[23];
		char msg[32];
		GetID_str(id);
		uint8_t len;

		strcpy_P(msg, PSTR("Device id:\n"));
		len = strlen(msg);

		memcpy(msg + len, id, 11);
		msg[len + 11] = '\n';
		memcpy(msg + len + 12, id + 11, 11);
		msg[len + 12 + 11] = 0;

		gui_showmessage(msg);
		gui_forcemessage();
	}
	break;

	case(1):
		config.connectivity.usb_mode = !config.connectivity.usb_mode;
		
		ee_update_byte(&config_ee.connectivity.usb_mode, config.connectivity.usb_mode);
	break;

	case(2):
		config.connectivity.uart_function = (config.connectivity.uart_function + 1) % NUMBER_OF_UART_FORWARD;
		
		ee_update_byte(&config_ee.connectivity.uart_function, config.connectivity.uart_function);
		uart_stop();
		uart_init();
	break;

	case(3):
		gui_switch_task(GUI_SET_CALIB);
	break;

	case(4):
		gui_dialog_set_P(PSTR("Warning"), PSTR("This will wipe\nall your settings!\nContinue?"), GUI_STYLE_FORMAT, gui_set_advanced_restore_cfg_cb);
		gui_switch_task(GUI_DIALOG);
	break;
	}
}

void gui_set_advanced_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			strcpy_P(text, PSTR("Device id"));
		break;

		case (1):
			strcpy_P(text, PSTR("Mass Storage"));
			if (config.connectivity.usb_mode == USB_MODE_MASSSTORAGE)
				*flags =  GUI_LIST_CHECK_ON;
			else
				*flags =  GUI_LIST_CHECK_OFF;
		break;

		case (2):
			strcpy_P(text, PSTR("Uart function"));
			*flags =  GUI_LIST_SUB_TEXT;
			switch (config.connectivity.uart_function)
			{
				case(UART_FORWARD_DEBUG):
					strcpy_P(sub_text, PSTR("Debug msg"));
				break;
				case(UART_FORWARD_OFF):
					strcpy_P(sub_text, PSTR("Uart off"));
				break;
				case(UART_FORWARD_9600):
					sprintf_P(sub_text, PSTR("Telemetry %u00"), 96);
				break;
				case(UART_FORWARD_19200):
					sprintf_P(sub_text, PSTR("Telemetry %u00"), 192);
				break;
				case(UART_FORWARD_38400):
					sprintf_P(sub_text, PSTR("Telemetry %u00"), 384);
				break;
				case(UART_FORWARD_57600):
					sprintf_P(sub_text, PSTR("Telemetry %u00"), 576);
				break;
				case(UART_FORWARD_115200):
					sprintf_P(sub_text, PSTR("Telemetry %u00"), 1152);
				break;
			}
		break;

		case (3):
			strcpy_P(text, PSTR("Calibration"));
		break;

		case (4):
			strcpy_P(text, PSTR("Factory reset"));
		break;
	}
}

