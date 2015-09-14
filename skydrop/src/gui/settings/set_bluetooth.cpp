#include "set_bluetooth.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"
#include "../../drivers/bluetooth/bt.h"

void gui_set_bluetooth_init()
{
	gui_list_set(gui_set_bluetooth_item, gui_set_bluetooth_action, 3, GUI_SETTINGS);
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
		case(1):
			config.system.use_bt = !config.system.use_bt;
			eeprom_busy_wait();
			eeprom_update_byte(&config_ee.system.use_bt, config.system.use_bt);

			if (config.system.use_bt)
				bt_module_init();
			else
				bt_module_deinit();
		break;
	}
}

void gui_set_bluetooth_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			sprintf_P(text, PSTR("Bluetooth module"));
			*flags |= GUI_LIST_SUB_TEXT;
			switch (bt_get_module_type())
			{
				case(BT_UNKNOWN):
					sprintf_P(sub_text, PSTR("Unknown"));
				break;
				case(BT_PAN1026):
					sprintf_P(sub_text, PSTR("pan1026 v4.0"));
				break;
				case(BT_PAN1322):
					sprintf_P(sub_text, PSTR("pan1322 v2.1"));
				break;
			}

		break;

		case (1):
			sprintf_P(text, PSTR("Enabled"));
			if (config.system.use_bt)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;
		break;

		case (2):
			sprintf_P(text, PSTR("Protocol"));
			*flags |= GUI_LIST_SUB_TEXT;
			switch(config.system.protocol)
			{
				case(PROTOCOL_DIGIFLY):
					sprintf_P(sub_text, PSTR("DigiFly"));
				break;

			}
		break;

	}
}

