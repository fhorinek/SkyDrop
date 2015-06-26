#include "set_debug.h"
#include "../gui_list.h"

void gui_set_debug_init()
{
	gui_list_set(gui_set_debug_item, gui_set_debug_action, 3, GUI_SET_SYSTEM);
}

void gui_set_debug_stop()
{
}

void gui_set_debug_loop()
{
	gui_list_draw();
}

void gui_set_debug_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}


void gui_set_debug_action(uint8_t index)
{
	switch(index)
	{
	case(3):
		gui_switch_task(GUI_SETTINGS);
	break;
	}
}

extern uint8_t system_rst;

void gui_set_debug_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			if (system_rst & 0b00100000)
				sprintf_P(text, PSTR("RST:Software "));
			else
			if (system_rst & 0b00010000)
				sprintf_P(text, PSTR("RST:Programming"));
			else
			if (system_rst & 0b00001000)
				sprintf_P(text, PSTR("RST:Watchdog"));
			else
			if (system_rst & 0b00000100)
				sprintf_P(text, PSTR("RST:Brownout"));
			else
			if (system_rst & 0b00000010)
				sprintf_P(text, PSTR("RST:External"));
			else
			if (system_rst & 0b00000001)
				sprintf_P(text, PSTR("RST:Power On"));
			else
				sprintf_P(text, PSTR("RST:Unknown: %02X"), system_rst);
		break;

		case (1):
			sprintf_P(text, PSTR("FW %s"), fw_info.app_name);
		break;

		case (2):
			sprintf_P(text, PSTR("ADC raw %d"), battery_adc_raw);
		break;

	}
}

