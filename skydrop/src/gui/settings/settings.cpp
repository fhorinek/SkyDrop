#include "settings.h"

#include "../gui_list.h"
#include "../gui_filemanager.h"

#include "fc/logger/logger.h"

void gui_settings_init()
{
#ifndef DISABLE_DEBUG
	gui_list_set(gui_settings_item, gui_settings_action, 10, GUI_PAGES);
#else
	gui_list_set(gui_settings_item, gui_settings_action, 9, GUI_PAGES);
#endif
}

void gui_settings_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			strcpy_P(text, PSTR("Vario"));
		break;

		case (1):
			strcpy_P(text, PSTR("Altimeters"));
		break;

		case (2):
			strcpy_P(text, PSTR("Logger"));
		break;

		case (3):
			strcpy_P(text, PSTR("Flight logs"));
		break;

		case (4):
			strcpy_P(text, PSTR("Navigation"));
		break;

		case (5):
			strcpy_P(text, PSTR("Airspaces"));
		break;

		case (6):
			strcpy_P(text, PSTR("GPS"));
		break;

		case (7):
			strcpy_P(text, PSTR("Bluetooth"));
		break;

		case (8):
			strcpy_P(text, PSTR("Settings"));
		break;
#ifndef DISABLE_DEBUG
		case (9):
			strcpy_P(text, PSTR("Debug"));
		break;
#endif
	}
}


void gui_settings_action(uint8_t index)
{
	switch (index)
	{
	case (0):
		gui_switch_task(GUI_SET_VARIO);
		break;

	case (1):
		gui_switch_task(GUI_SET_ALTIMETERS);
		break;

	case (2):
		gui_switch_task(GUI_SET_LOGGER);
		break;

	case (3):
		gui_filemanager_set_dir((char *) LOG_DIR_ABS);
		gui_filemanager_level = 0;
		gui_filemanager_set_tasks(GUI_FLIGHTDETAIL, GUI_SETTINGS);
		gui_switch_task(GUI_FILEMANAGER);
		break;

	case (4):
		gui_switch_task(GUI_NAVIGATION);
		break;

	case (5):
		gui_switch_task(GUI_SET_AIRSPACE);
		break;

	case (6):
		gui_switch_task(GUI_SET_GPS);
		break;

	case (7):
		gui_switch_task(GUI_SET_BLUETOOTH);
		break;

	case (8):
		gui_switch_task(GUI_SET_SYSTEM);
		break;

#ifndef DISABLE_DEBUG
	case (9):
		gui_switch_task(GUI_SET_DEBUG);
		break;
#endif
	}
}
