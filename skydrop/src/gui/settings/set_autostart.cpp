#include "set_autostart.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"

void gui_set_autostart_init()
{
	gui_list_set(gui_set_autostart_item, gui_set_autostart_action, 6, GUI_SET_LOGGER);
}

void gui_set_autostart_start_threshold_cb(float val)
{
	uint8_t tmp = val;
	
	ee_update_byte(&config_ee.autostart.start_sensititvity, tmp);
	config.autostart.start_sensititvity = tmp;

	gui_switch_task(GUI_SET_AUTOSTART);
}

void gui_set_autostart_land_threshold_cb(float val)
{
	uint8_t tmp = val;
	
	ee_update_byte(&config_ee.autostart.land_sensititvity, tmp);
	config.autostart.land_sensititvity = tmp;

	gui_switch_task(GUI_SET_AUTOSTART);
}

void gui_set_autostart_timeout_cb(float val)
{
	uint8_t tmp = val;
	
	ee_update_byte(&config_ee.autostart.timeout, tmp);
	config.autostart.timeout = tmp;

	gui_switch_task(GUI_SET_AUTOSTART);
}

void gui_set_autostart_gps_speed_cb(float val)
{
	uint8_t tmp = val;

	ee_update_byte(&config_ee.autostart.gps_speed, tmp);
	config.autostart.gps_speed = tmp;

	gui_switch_task(GUI_SET_AUTOSTART);
}

void gui_set_autostart_action(uint8_t index)
{
	switch(index)
	{
		case(0):
			gui_value_conf_P(PSTR("Start threshold"), GUI_VAL_NUMBER_DISABLE, PSTR("+/-%0.0fm"), config.autostart.start_sensititvity, 0, 100, 1, gui_set_autostart_start_threshold_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(1):
			gui_value_conf_P(PSTR("Land threshold"), GUI_VAL_NUMBER_DISABLE, PSTR("+/-%0.0fm"), config.autostart.land_sensititvity, 0, 100, 1, gui_set_autostart_land_threshold_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(2):
			gui_value_conf_P(PSTR("GPS Speed"), GUI_VAL_NUMBER_DISABLE, PSTR("> %0.0fkm/h"), config.autostart.gps_speed, 0, 20, 1, gui_set_autostart_gps_speed_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(3):
			gui_value_conf_P(PSTR("Timeout"), GUI_VAL_NUMBER, PSTR("%0.0f sec"), config.autostart.timeout, 30, 240, 1, gui_set_autostart_timeout_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(4):
			config.autostart.flags ^= AUTOSTART_SUPRESS_AUDIO;
			
			ee_update_byte(&config_ee.autostart.flags, config.autostart.flags);
		break;

		case(5):
			config.autostart.flags ^= AUTOSTART_ALWAYS_ENABLED;
			
			ee_update_byte(&config_ee.autostart.flags, config.autostart.flags);
		break;
	}
}

void gui_set_autostart_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			strcpy_P(text, PSTR("Start threshold"));
			if (config.autostart.start_sensititvity > 0)
				sprintf_P(sub_text, PSTR("+/-%dm"), config.autostart.start_sensititvity);
			else
				strcpy_P(sub_text, PSTR("disabled"));

			*flags =  GUI_LIST_SUB_TEXT;
		break;


		case (1):
			strcpy_P(text, PSTR("Land threshold"));
			if (config.autostart.land_sensititvity > 0)
				sprintf_P(sub_text, PSTR("+/-%dm"), config.autostart.land_sensititvity);
			else
				strcpy_P(sub_text, PSTR("disabled"));
			*flags =  GUI_LIST_SUB_TEXT;
		break;

		case (2):
			strcpy_P(text, PSTR("GPS Speed"));
			if (config.autostart.gps_speed > 0)
				sprintf_P(sub_text, PSTR("> %dkm/h"), config.autostart.gps_speed);
			else
				strcpy_P(sub_text, PSTR("disabled"));
			*flags =  GUI_LIST_SUB_TEXT;
		break;

		case (3):
			strcpy_P(text, PSTR("Timeout"));
			sprintf_P(sub_text, PSTR("%d sec"), config.autostart.timeout);
			*flags =  GUI_LIST_SUB_TEXT;
		break;

		case (4):
			strcpy_P(text, PSTR("Suppress audio"));
			if (config.autostart.flags & AUTOSTART_SUPRESS_AUDIO)
				*flags =  GUI_LIST_CHECK_ON;
			else
				*flags =  GUI_LIST_CHECK_OFF;
		break;

		case (5):
			strcpy_P(text, PSTR("Record always"));
			if (config.autostart.flags & AUTOSTART_ALWAYS_ENABLED)
				*flags =  GUI_LIST_CHECK_ON;
			else
				*flags =  GUI_LIST_CHECK_OFF;
		break;
	}
}

