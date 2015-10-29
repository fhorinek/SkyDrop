#include "set_logger.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"

void gui_set_logger_init()
{
	gui_list_set(gui_set_logger_item, gui_set_logger_action, 3, GUI_SETTINGS);
}

void gui_set_logger_stop() {}

void gui_set_logger_loop()
{
	gui_list_draw();
}

void gui_set_logger_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_set_logger_action(uint8_t index)
{
	switch(index)
	{
		case(0):
			config.logger.enabled = !config.logger.enabled;
			eeprom_busy_wait();
			eeprom_update_byte(&config_ee.logger.enabled, config.logger.enabled);
		break;

		case(1):
			if (fc.logger_state == FLIGHT_LAND)
			{
				gui_showmessage_P(PSTR("Cannot change\nin flight!"));
				return;
			}
			config.logger.format = (config.logger.format + 1) % NUMBER_OF_FORMATS;
			eeprom_busy_wait();
			eeprom_update_byte(&config_ee.logger.format, config.logger.format);
		break;

		case(2):
			gui_switch_task(GUI_SET_AUTOSTART);
		break;
	}
}

void gui_set_logger_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			sprintf_P(text, PSTR("Enabled"));
			if (config.logger.enabled)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_ON;
		break;

		case (1):
			sprintf_P(text, PSTR("Format"));
			*flags |= GUI_LIST_SUB_TEXT;
			switch (config.logger.format)
			{
				case(LOGGER_IGC):
					sprintf_P(sub_text, PSTR("IGC"));
				break;

				case(LOGGER_KML):
					sprintf_P(sub_text, PSTR("KML (for Ash Ha)"));
				break;
			}
		break;

		case (2):
			sprintf_P(text, PSTR("Auto start/stop"));
			*flags |= GUI_LIST_SUB_TEXT;
			if (config.autostart.start_sensititvity > 0 && config.autostart.land_sensititvity > 0)
				sprintf_P(sub_text, PSTR("S +/-%dm L +/-%dm"), config.autostart.start_sensititvity, config.autostart.land_sensititvity);
			else if(config.autostart.start_sensititvity > 0)
				sprintf_P(sub_text, PSTR("S +/-%dm"), config.autostart.start_sensititvity);
			else if(config.autostart.land_sensititvity > 0)
				sprintf_P(sub_text, PSTR("L +/-%dm"), config.autostart.land_sensititvity);
			else
				sprintf_P(sub_text, PSTR("disabled"));
		break;
	}
}

