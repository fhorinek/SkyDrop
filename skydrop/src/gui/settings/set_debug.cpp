#include "set_debug.h"
#include "../gui_list.h"
#include "../gui_dialog.h"
#include "../../fc/conf.h"

#ifndef DISABLE_DEBUG
void gui_set_debug_init()
{
	gui_list_set(gui_set_debug_item, gui_set_debug_action, 6, GUI_SETTINGS);
}

void gui_set_debug_delete_log(uint8_t ret)
{
	if (ret == GUI_DIALOG_YES)
	{
		assert(f_unlink(DEBUG_FILE) == FR_OK);
	}

	gui_switch_task(GUI_SET_DEBUG);
}

void gui_set_debug_action(uint8_t index)
{
	switch(index)
	{
		case(2):
			if (config.system.debug_log == DEBUG_MAGIC_ON)
				config.system.debug_log = 0;
			else
				config.system.debug_log = DEBUG_MAGIC_ON;
			
			ee_update_byte(&config_ee.system.debug_log, config.system.debug_log);
		break;

		case(3):
			gui_dialog_set_P(PSTR("Confirmation"), PSTR("Clear\ndebug.log?"), GUI_STYLE_YESNO, gui_set_debug_delete_log);
			gui_switch_task(GUI_DIALOG);
		break;

		case(4):
			config.system.debug_gps = !config.system.debug_gps;
			
			ee_update_byte(&config_ee.system.debug_gps, config.system.debug_gps);
		break;

		case(5):
			config.system.record_screen = !config.system.record_screen;
			
			ee_update_byte(&config_ee.system.record_screen, config.system.record_screen);
		break;
	}
}

void gui_set_debug_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			strcpy_P(text, PSTR("Firmware"));
			*flags = GUI_LIST_SUB_TEXT;
			sprintf_P(sub_text, PSTR("%s"), fw_info.app_name);
		break;


		case (1):
			strcpy_P(text, PSTR("Board rev."));
			*flags = GUI_LIST_SUB_TEXT;

			if (hw_revision == HW_REW_1504)
				strcpy_P(sub_text, PSTR("drop 1504"));
			else if (hw_revision == HW_REW_1506)
				strcpy_P(sub_text, PSTR("drop 1506"));
			else strcpy_P(sub_text, PSTR("???"));

		break;

		case (2):
			strcpy_P(text, PSTR("Debug log"));
			if (config.system.debug_log == DEBUG_MAGIC_ON)
				*flags = GUI_LIST_CHECK_ON;
			else
				*flags = GUI_LIST_CHECK_OFF;
		break;

		case (3):
			strcpy_P(text, PSTR("Clear log"));
		break;

		case (4):
			strcpy_P(text, PSTR("Debug GPS"));
			if (config.system.debug_gps)
				*flags = GUI_LIST_CHECK_ON;
			else
				*flags = GUI_LIST_CHECK_OFF;
		break;

		case (5):
			strcpy_P(text, PSTR("Record screen"));
			if (config.system.record_screen)
				*flags = GUI_LIST_CHECK_ON;
			else
				*flags = GUI_LIST_CHECK_OFF;
		break;


	}
}
#endif
