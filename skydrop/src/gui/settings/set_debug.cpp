#include "set_debug.h"
#include "../gui_list.h"
#include "../gui_dialog.h"
#include "../../fc/conf.h"

void gui_set_debug_init()
{
	gui_list_set(gui_set_debug_item, gui_set_debug_action, 8, GUI_SETTINGS);
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
//		case(3):
//			gui_dialog_set_P(PSTR("Confirmation"), PSTR("Do you want to\nreset Factory\ntest?"), GUI_STYLE_YESNO, gui_set_debug_reset_fc);
//			gui_switch_task(GUI_DIALOG);
//		break;

		case(2):
			battery_next_meas = task_get_ms_tick();
		break;

		case(4):
			if (config.system.debug_log == DEBUG_MAGIC_ON)
				config.system.debug_log = 0;
			else
				config.system.debug_log = DEBUG_MAGIC_ON;
			eeprom_busy_wait();
			eeprom_update_byte(&config_ee.system.debug_log, config.system.debug_log);
		break;

		case(5):
			gui_dialog_set_P(PSTR("Confirmation"), PSTR("Clear\ndebug.log?"), GUI_STYLE_YESNO, gui_set_debug_delete_fc);
			gui_switch_task(GUI_DIALOG);
		break;

		case(6):
			if (config.system.debug_gps == DEBUG_MAGIC_ON)
				config.system.debug_gps = 0;
			else
				config.system.debug_gps = DEBUG_MAGIC_ON;
			eeprom_busy_wait();
			eeprom_update_byte(&config_ee.system.debug_gps, config.system.debug_gps);
		break;
	}
}

extern uint8_t system_rst;

void gui_set_debug_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			sprintf_P(text, PSTR("Reset"));
			*flags |= GUI_LIST_SUB_TEXT;

			if (system_rst & 0b00100000)
				sprintf_P(sub_text, PSTR("Software"));
			else
			if (system_rst & 0b00010000)
				sprintf_P(sub_text, PSTR("Programming"));
			else
			if (system_rst & 0b00001000)
				sprintf_P(sub_text, PSTR("Watchdog"));
			else
			if (system_rst & 0b00000100)
				sprintf_P(sub_text, PSTR("Brownout"));
			else
			if (system_rst & 0b00000010)
				sprintf_P(sub_text, PSTR("External"));
			else
			if (system_rst & 0b00000001)
				sprintf_P(sub_text, PSTR("Power On"));
			else
				sprintf_P(sub_text, PSTR("Unknown: %02X"), system_rst);
		break;

		case (1):
			sprintf_P(text, PSTR("Firmware"));
			*flags |= GUI_LIST_SUB_TEXT;
			sprintf_P(sub_text, PSTR("%s"), fw_info.app_name);
		break;

		case (2):
			sprintf_P(text, PSTR("ADC raw"));
			*flags |= GUI_LIST_SUB_TEXT;
			sprintf_P(sub_text, PSTR("%d"), battery_adc_raw);
		break;

//		case (3):
//			sprintf_P(text, PSTR("Reset Factory test"));
//		break;

		case (3):
			sprintf_P(text, PSTR("Board rev."));
			*flags |= GUI_LIST_SUB_TEXT;

			if (hw_revision == HW_REW_1504)
				sprintf_P(sub_text, PSTR("drop 1504"));
			else if (hw_revision == HW_REW_1506)
				sprintf_P(sub_text, PSTR("drop 1506"));
			else sprintf_P(sub_text, PSTR("???"));

		break;

		case (4):
			sprintf_P(text, PSTR("Debug log"));
			if (config.system.debug_log == DEBUG_MAGIC_ON)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;
		break;

		case (5):
			sprintf_P(text, PSTR("Clear log"));
			*flags |= GUI_LIST_FOLDER;
		break;

		case (6):
			sprintf_P(text, PSTR("Debug GPS"));
			if (config.system.debug_gps == DEBUG_MAGIC_ON)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;
		break;

		case (7):
			sprintf_P(text, PSTR("WDT last PC"));
			*flags |= GUI_LIST_SUB_TEXT;

			sprintf_P(sub_text, PSTR("0x%lX"), debug_last_pc);
		break;

	}
}

