#include "set_debug.h"
#include "../gui_list.h"
#include "../gui_dialog.h"
#include "../../fc/conf.h"

void gui_set_debug_init()
{
	gui_list_set(gui_set_debug_item, gui_set_debug_action, 5, GUI_SETTINGS);
}

void gui_set_debug_stop() {}

void gui_set_debug_loop()
{
	gui_list_draw();
}

void gui_set_debug_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_set_debug_reset_fc(uint8_t ret)
{
	if (ret == GUI_DIALOG_YES)
	{
		cfg_reset_factory_test();
	}
	else
		gui_switch_task(GUI_SET_DEBUG);
}

void gui_set_debug_delete_fc(uint8_t ret)
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
		case(0):
			if (hw_revision == HW_REW_1504)
				hw_revision = HW_REW_1506;
			else
				hw_revision = HW_REW_1504;

			eeprom_busy_wait();
			eeprom_update_byte(&config_ro.hw_revision, hw_revision);
			eeprom_busy_wait();
		break;

		case(1):
			if (config.system.debug_log == DEBUG_MAGIC_ON)
				config.system.debug_log = 0;
			else
				config.system.debug_log = DEBUG_MAGIC_ON;
		break;

		case(2):
			gui_dialog_set_P(PSTR("Confirmation"), PSTR("Clear\ndebug.log?"), GUI_STYLE_YESNO, gui_set_debug_delete_fc);
			gui_switch_task(GUI_DIALOG);
		break;

		case(3):
			config.system.debug_gps = !config.system.debug_gps;
		break;

		case(4):
			config.system.record_screen = !config.system.record_screen;
		break;
	}
}

void gui_set_debug_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			strcpy_P(text, PSTR("Board rev."));
			*flags |= GUI_LIST_SUB_TEXT;

			if (hw_revision == HW_REW_1504)
				strcpy_P(sub_text, PSTR("drop 1504"));
			else if (hw_revision == HW_REW_1506)
				strcpy_P(sub_text, PSTR("drop 1506"));
			else strcpy_P(sub_text, PSTR("???"));

		break;

		case (1):
			strcpy_P(text, PSTR("Debug log"));
			if (config.system.debug_log == DEBUG_MAGIC_ON)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;
		break;

		case (2):
			strcpy_P(text, PSTR("Clear log"));
			*flags |= GUI_LIST_FOLDER;
		break;

		case (3):
			strcpy_P(text, PSTR("Debug GPS"));
			if (config.system.debug_gps)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;
		break;

		case (4):
			strcpy_P(text, PSTR("Record screen"));
			if (config.system.record_screen)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;
		break;


	}
}

