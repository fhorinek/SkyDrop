#include "set_bluetooth.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"
#include "../../drivers/bluetooth/bt.h"

void gui_set_bluetooth_init()
{
	gui_list_set(gui_set_bluetooth_item, gui_set_bluetooth_action, 4, GUI_SETTINGS);
}

void gui_set_bluetooth_stop() {}

void gui_set_bluetooth_loop()
{
	gui_list_draw();
}

void gui_set_bluetooth_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_set_bluetooth_action(uint8_t index)
{
	switch (index)
	{
		case(0):
			if (config.connectivity.use_bt)
				bt_module_deinit();

			eeprom_busy_wait();

			if (bt_get_module_type() == BT_PAN1026)
				eeprom_update_byte(&config_ro.bt_module_type, BT_PAN1322);
			else
				eeprom_update_byte(&config_ro.bt_module_type, BT_PAN1026);

			bt_init();
			if (config.connectivity.use_bt)
				bt_module_init();

		break;

		case(1):
			config.connectivity.use_bt = !config.connectivity.use_bt;

			if (config.connectivity.use_bt)
				bt_module_init();
			else
				bt_module_deinit();
		break;

		case(2):
			config.connectivity.protocol = (config.connectivity.protocol + 1) % NUMBER_OF_PROTOCOLS;
		break;

		case(3):
			config.connectivity.forward_gps = !config.connectivity.forward_gps;
		break;
	}
}

void gui_set_bluetooth_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			strcpy_P(text, PSTR("Bluetooth module"));
			*flags |= GUI_LIST_SUB_TEXT;
			switch (bt_get_module_type())
			{
				case(BT_UNKNOWN):
					strcpy_P(sub_text, PSTR("Unknown"));
				break;
				case(BT_PAN1026):
					strcpy_P(sub_text, PSTR("pan1026 v4.0"));
				break;
				case(BT_PAN1322):
					strcpy_P(sub_text, PSTR("pan1322 v2.1"));
				break;
			}

		break;

		case (1):
			strcpy_P(text, PSTR("Enabled"));
			if (config.connectivity.use_bt)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;
		break;

		case (2):
			strcpy_P(text, PSTR("Protocol"));
			*flags |= GUI_LIST_SUB_TEXT;
			switch(config.connectivity.protocol)
			{
				case(PROTOCOL_DIGIFLY):
					strcpy_P(sub_text, PSTR("DigiFly"));
				break;
				case(PROTOCOL_LK8EX1):
					strcpy_P(sub_text, PSTR("LK8EX1"));
				break;
				case(PROTOCOL_BLUEFLY):
					strcpy_P(sub_text, PSTR("BlueFly"));
				break;
//				case(PROTOCOL_SKYBEAN):
//					strcpy_P(sub_text, PSTR("SkyBean"));
//				break;
			}
		break;

		case (3):
			strcpy_P(text, PSTR("Forward GPS"));
			if (config.connectivity.forward_gps)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;
		break;

	}
}

