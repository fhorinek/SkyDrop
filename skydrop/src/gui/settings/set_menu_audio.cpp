#include "set_menu_audio.h"

#include "../gui_list.h"


void gui_set_menu_audio_init()
{
	gui_list_set(gui_set_menu_audio_item, gui_set_menu_audio_action, 4, GUI_SET_AUDIO);
}

void gui_set_menu_audio_stop()
{
	
	ee_update_byte(&config_ee.gui.menu_audio_flags, config.gui.menu_audio_flags);
}

void gui_set_menu_audio_action(uint8_t index)
{
	switch(index)
	{
		case(0):
			config.gui.menu_audio_flags = config.gui.menu_audio_flags ^ CFG_AUDIO_MENU_SPLASH;
		break;

		case(1):
			config.gui.menu_audio_flags = config.gui.menu_audio_flags ^ CFG_AUDIO_MENU_PAGES;
		break;

		case(2):
			config.gui.menu_audio_flags = config.gui.menu_audio_flags ^ CFG_AUDIO_MENU_BUTTONS;
		break;

		case(3):
			config.gui.menu_audio_flags = config.gui.menu_audio_flags ^ CFG_AUDIO_MENU_GPS;
		break;
	}
}

void gui_set_menu_audio_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch(index)
	{
		case (0):
			strcpy_P(text, PSTR("On/Off sound"));
			if (config.gui.menu_audio_flags & CFG_AUDIO_MENU_SPLASH)
				*flags = GUI_LIST_CHECK_ON;
			else
				*flags = GUI_LIST_CHECK_OFF;
		break;

		case (1):
			strcpy_P(text, PSTR("Page sound"));
			if (config.gui.menu_audio_flags & CFG_AUDIO_MENU_PAGES)
				*flags = GUI_LIST_CHECK_ON;
			else
				*flags = GUI_LIST_CHECK_OFF;
		break;

		case (2):
			strcpy_P(text, PSTR("Button sound"));
			if (config.gui.menu_audio_flags & CFG_AUDIO_MENU_BUTTONS)
				*flags = GUI_LIST_CHECK_ON;
			else
				*flags = GUI_LIST_CHECK_OFF;
		break;

		case (3):
			strcpy_P(text, PSTR("GPS ready"));
			if (config.gui.menu_audio_flags & CFG_AUDIO_MENU_GPS)
				*flags = GUI_LIST_CHECK_ON;
			else
				*flags = GUI_LIST_CHECK_OFF;
		break;
	}
}

