#include "set_autostart.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"

void gui_set_autostart_init()
{
	gui_list_set(gui_set_autostart_item, gui_set_autostart_action, 3, GUI_SET_LOGGER);
}

void gui_set_autostart_stop() {}

void gui_set_autostart_loop()
{
	gui_list_draw();
}

void gui_set_autostart_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_set_autostart_threshold_cb(float val)
{
	uint8_t tmp = val;
	eeprom_busy_wait();
	eeprom_update_byte(&config.autostart.sensititvity, tmp);
	fc.autostart_sensitivity = tmp;

	gui_switch_task(GUI_SET_AUTOSTART);
}

void gui_set_autostart_action(uint8_t index)
{
	switch(index)
	{
		case(1):
			gui_value_conf_P(PSTR("Threshold"), GUI_VAL_NUMBER, PSTR("+/-%0.0fm"), fc.autostart_sensitivity, 0, 100, 1, gui_set_autostart_threshold_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(2):
			fc.audio_supress = !fc.audio_supress;
			eeprom_busy_wait();
			eeprom_update_byte(&config.autostart.supress_audio, fc.audio_supress);
		break;
	}
}

void gui_set_autostart_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			sprintf_P(text, PSTR("State"));
			*flags |= GUI_LIST_SUB_TEXT;
			switch (fc.autostart_state)
			{
				case(AUTOSTART_WAIT):
					sprintf_P(sub_text, PSTR("Waiting"));
				break;
				case(AUTOSTART_FLIGHT):
					sprintf_P(sub_text, PSTR("Flying"));
				break;
				case(AUTOSTART_LAND):
					sprintf_P(sub_text, PSTR("Landed"));
				break;
			}

		break;

		case (1):
			sprintf_P(text, PSTR("Threshold"));
			if (fc.autostart_sensitivity)
				sprintf_P(sub_text, PSTR("+/-%dm"), fc.autostart_sensitivity);
			else
				sprintf_P(sub_text, PSTR("disabled"));
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (2):
			sprintf_P(text, PSTR("Suppress audio"));
			if (fc.audio_supress)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;
		break;
	}
}

