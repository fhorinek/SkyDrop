#include "set_logger.h"

#include "../gui_list.h"
#include "../gui_value.h"
#include "../gui_text.h"

#include "../../fc/conf.h"
#include "../../fc/logger/logger.h"

void gui_set_logger_init()
{
	gui_list_set(gui_set_logger_item, gui_set_logger_action, 6, GUI_SETTINGS);
}

void gui_set_logger_pilot_cb(uint8_t ret, char * buff)
{
	if (ret == GUI_TEXT_OK)
	{
		strcpy((char *)config.logger.pilot, buff);
		eeprom_update_block((void *)config.logger.pilot, (void *)config_ee.logger.pilot, LOG_TEXT_LEN);
	}

	gui_switch_task(GUI_SET_LOGGER);
}

void gui_set_logger_glider_type_cb(uint8_t ret, char * buff)
{
	if (ret == GUI_TEXT_OK)
	{
		strcpy((char *)config.logger.glider_type, buff);
		eeprom_update_block((void *)config.logger.glider_type, (void *)config_ee.logger.glider_type, LOG_TEXT_LEN);
	}

	gui_switch_task(GUI_SET_LOGGER);
}

void gui_set_logger_glider_id_cb(uint8_t ret, char * buff)
{
	if (ret == GUI_TEXT_OK)
	{
		strcpy((char *)config.logger.glider_id, buff);
		eeprom_update_block((void *)config.logger.glider_id, (void *)config_ee.logger.glider_id, LOG_TEXT_LEN);
	}

	gui_switch_task(GUI_SET_LOGGER);
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
			if (logger_active())
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

		case(3):
			gui_text_conf((char *)config.logger.pilot, LOG_TEXT_LEN, gui_set_logger_pilot_cb);
			gui_switch_task(GUI_TEXT);
		break;

		case(4):
			gui_text_conf((char *)config.logger.glider_type, LOG_TEXT_LEN, gui_set_logger_glider_type_cb);
			gui_switch_task(GUI_TEXT);
		break;

		case(5):
			gui_text_conf((char *)config.logger.glider_id, LOG_TEXT_LEN, gui_set_logger_glider_id_cb);
			gui_switch_task(GUI_TEXT);
		break;
	}
}

void gui_set_logger_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			strcpy_P(text, PSTR("Enabled"));
			if (config.logger.enabled)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;
		break;

		case (1):
			strcpy_P(text, PSTR("Format"));
			*flags |= GUI_LIST_SUB_TEXT;
			switch (config.logger.format)
			{
				case(LOGGER_IGC):
					strcpy_P(sub_text, PSTR("IGC"));
				break;

				case(LOGGER_KML):
					strcpy_P(sub_text, PSTR("KML"));
				break;

				case(LOGGER_RAW):
					strcpy_P(sub_text, PSTR("RAW (IMU diag)"));
				break;

				case(LOGGER_AERO):
					strcpy_P(sub_text, PSTR("Aero (acc only)"));
				break;
			}
		break;

		case (2):
			strcpy_P(text, PSTR("Auto start/land"));
			*flags |= GUI_LIST_SUB_TEXT;
			if (config.autostart.flags & AUTOSTART_ALWAYS_ENABLED)
				strcpy_P(sub_text, PSTR("record always"));
			else if (config.autostart.start_sensititvity > 0 && config.autostart.land_sensititvity > 0)
				sprintf_P(sub_text, PSTR("S +/-%dm L +/-%dm"), config.autostart.start_sensititvity, config.autostart.land_sensititvity);
			else if(config.autostart.start_sensititvity > 0)
				sprintf_P(sub_text, PSTR("S +/-%dm"), config.autostart.start_sensititvity);
			else if(config.autostart.land_sensititvity > 0)
				sprintf_P(sub_text, PSTR("L +/-%dm"), config.autostart.land_sensititvity);
			else
				strcpy_P(sub_text, PSTR("disabled"));
		break;

		case (3):
			strcpy_P(text, PSTR("Pilot name"));
			*flags |= GUI_LIST_SUB_TEXT;

			if (strlen((char *)config.logger.pilot) == 0)
				strcpy_P(sub_text, PSTR("<empty>"));
			else
				gui_fit_text((char *)config.logger.pilot, sub_text, GUI_DISP_WIDTH - 2);
		break;

		case (4):
			strcpy_P(text, PSTR("Glider type"));
			*flags |= GUI_LIST_SUB_TEXT;

			if (strlen((char *)config.logger.glider_type) == 0)
				strcpy_P(sub_text, PSTR("<empty>"));
			else
				gui_fit_text((char *)config.logger.glider_type, sub_text, GUI_DISP_WIDTH - 2);
		break;

		case (5):
			strcpy_P(text, PSTR("Glider id"));
			*flags |= GUI_LIST_SUB_TEXT;

			if (strlen((char *)config.logger.glider_id) == 0)
				strcpy_P(sub_text, PSTR("<empty>"));
			else
				gui_fit_text((char *)config.logger.glider_id, sub_text, GUI_DISP_WIDTH - 2);
		break;
	}
}

