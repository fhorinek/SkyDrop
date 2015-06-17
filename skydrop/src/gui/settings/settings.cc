#include "settings.h"
#include "../gui_list.h"

void gui_settings_init()
{
	gui_list_set(gui_settings_item, gui_settings_action, 8);
}

void gui_settings_stop()
{
}

void gui_settings_loop()
{
	gui_list_draw();
}

void gui_settings_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}


void gui_settings_action(uint8_t index)
{
	switch(index)
	{
	case(0):
		gui_switch_task(GUI_SET_VARIO);
	break;

	case(1):
		gui_switch_task(GUI_SET_AUDIO);
	break;

	case(2):
		gui_switch_task(GUI_SET_DISPLAY);
	break;

	case(3):
		gui_switch_task(GUI_SET_SYSTEM);
	break;

	case(4):
		gui_switch_task(GUI_SET_AUTOSTART);
	break;

	case(5):
		gui_switch_task(GUI_SET_GPS);
	break;

	case(6):
		gui_switch_task(GUI_SET_DEBUG);
	break;

	case(7):
		gui_switch_task(GUI_PAGES);
	break;
	}
}

void gui_settings_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			sprintf_P(text, PSTR("Vario"));
			*flags |= GUI_LIST_FOLDER;
		break;
		case (1):
			sprintf_P(text, PSTR("Audio"));
			*flags |= GUI_LIST_FOLDER;
		break;
		case (2):
			sprintf_P(text, PSTR("Display"));
			*flags |= GUI_LIST_FOLDER;
		break;
		case (3):
			sprintf_P(text, PSTR("System"));
			*flags |= GUI_LIST_FOLDER;
		break;
		case (4):
			sprintf_P(text, PSTR("Autostart"));
			*flags |= GUI_LIST_FOLDER;
		break;
		case (5):
			sprintf_P(text, PSTR("GPS"));
			*flags |= GUI_LIST_FOLDER;
		break;
		case (6):
			sprintf_P(text, PSTR("Debug"));
			*flags |= GUI_LIST_FOLDER;
		break;
		case (7):
			sprintf_P(text, PSTR("back"));
			*flags |= GUI_LIST_BACK;
		break;

	}
}

