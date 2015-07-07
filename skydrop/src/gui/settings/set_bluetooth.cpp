#include "set_bluetooth.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"
#include "../../drivers/bluetooth/bt.h"

void gui_set_bluetooth_init()
{
	gui_list_set(gui_set_bluetooth_item, gui_set_bluetooth_action, 1, GUI_SETTINGS);
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

}

void gui_set_bluetooth_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	uint8_t bt_mod = bt_get_module_type();

	switch (index)
	{
		case (0):
			sprintf_P(text, PSTR("Bluetooth module"));
			*flags |= GUI_LIST_SUB_TEXT;
			switch (bt_mod)
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


	}
}

