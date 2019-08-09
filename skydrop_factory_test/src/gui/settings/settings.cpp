#include "settings.h"

#include "../splash.h"
#include "../gui_list.h"
#include "gui_filemanager.h"

void gui_settings_init()
{
	gui_list_set(gui_settings_item, gui_settings_action, 7, GUI_NONE);
}

void gui_settings_stop() {}

void gui_settings_loop()
{
	gui_list_draw();
}

void gui_settings_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_settings_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			strcpy_P(text, PSTR("Factory test"));
			*flags |= GUI_LIST_SUB_TEXT;
			if (cfg_factory_passed())
				strcpy_P(sub_text, PSTR("Passed"));
			else
			{
				if (GUI_BLINK_TGL(500))
					strcpy_P(sub_text, PSTR("Not passed!"));
				else
					strcpy_P(sub_text, PSTR(""));
			}

		break;

		case (1):
			strcpy_P(text, PSTR("FT override"));
			*flags |= GUI_LIST_SUB_TEXT;
			if (cfg_factory_passed())
				strcpy_P(sub_text, PSTR("Force reset"));
			else
				strcpy_P(sub_text, PSTR("Force pass"));
		break;

		case (2):
			strcpy_P(text, PSTR("GPS"));
			*flags |= GUI_LIST_FOLDER;
		break;

		case (3):
			strcpy_P(text, PSTR("Bluetooth"));
			*flags |= GUI_LIST_FOLDER;
		break;

		case (4):
			strcpy_P(text, PSTR("Settings"));
			*flags |= GUI_LIST_FOLDER;
		break;

		case (5):
			strcpy_P(text, PSTR("Debug"));
			*flags |= GUI_LIST_FOLDER;
		break;

		case (6):
			strcpy_P(text, PSTR("Power off"));
			*flags |= GUI_LIST_FOLDER;
		break;
	}
}


void gui_settings_action(uint8_t index)
{
	switch (index)
	{
	case (0):
		gui_switch_task(GUI_FTEST);
		break;

	case (1):
		if (cfg_factory_passed())
		{
			cfg_reset_factory_test();
		}
		else
		{
			eeprom_busy_wait();
			eeprom_update_byte(&config_ro.factory_passed, CFG_FACTORY_PASSED_hex);
			eeprom_busy_wait();
		}

	break;

	case (2):
		gui_switch_task(GUI_SET_GPS);
		break;

	case (3):
		gui_switch_task(GUI_SET_BLUETOOTH);
		break;

	case (4):
		gui_switch_task(GUI_SET_SYSTEM);
		break;

	case (5):
		gui_switch_task(GUI_SET_DEBUG);
		break;

	case (6):
		gui_splash_set_mode(SPLASH_OFF);
		gui_switch_task(GUI_SPLASH);
		break;

	}
}
