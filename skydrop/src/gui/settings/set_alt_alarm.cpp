#include "set_altimeters.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"

#include "set_alt_alarm.h"

//minimum alarm separation
#define ALARM_SEPARATION 10

void gui_set_alt_alarm_init()
{
	gui_list_set(gui_set_alt_alarm_item, gui_set_alt_alarm_action, 6, GUI_SET_ALTIMETERS);
}

void gui_set_alt_alarm_stop() {}

void gui_set_alt_alarm_loop()
{
	gui_list_draw();
}

void gui_set_alt_alarm_alarm1_cb(float val)
{
	gui_switch_task(GUI_SET_ALT_ALARM);

	config.altitude.alarm_1 = val;
	eeprom_busy_wait();
	eeprom_write_word(&config_ee.altitude.alarm_1, config.altitude.alarm_1);
}

void gui_set_alt_alarm_alarm2_cb(float val)
{
	gui_switch_task(GUI_SET_ALT_ALARM);

	config.altitude.alarm_2 = val;
	eeprom_busy_wait();
	eeprom_write_word(&config_ee.altitude.alarm_2, config.altitude.alarm_2);
}

void gui_set_alt_alarm_alarm_h1_cb(float val)
{
	gui_switch_task(GUI_SET_ALT_ALARM);

	config.altitude.alarm_h1 = val;
	eeprom_busy_wait();
	eeprom_write_word(&config_ee.altitude.alarm_h1, config.altitude.alarm_h1);
}

void gui_set_alt_alarm_reset_cb(float val)
{
	gui_switch_task(GUI_SET_ALT_ALARM);

	config.altitude.alarm_reset = val;
	eeprom_busy_wait();
	eeprom_write_word(&config_ee.altitude.alarm_reset, config.altitude.alarm_reset);
}

void gui_set_alt_alarm_confirm_secs_cb(float val)
{
	gui_switch_task(GUI_SET_ALT_ALARM);

	config.altitude.alarm_confirm_secs = val;
	eeprom_update_byte(&config_ee.altitude.alarm_confirm_secs, config.altitude.alarm_confirm_secs);
}

void gui_set_alt_alarm_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_set_alt_alarm_action(uint8_t index)
{
	switch(index)
	{
		case (0):
			config.altitude.alarm_enabled =! config.altitude.alarm_enabled;
			eeprom_update_byte(&config_ee.altitude.alarm_enabled, config.altitude.alarm_enabled);
		break;

		case (1):
			gui_value_conf_P(PSTR("Confirm time"), GUI_VAL_NUMBER, PSTR("%0.0f secs"), config.altitude.alarm_confirm_secs, 0, 255, 1, gui_set_alt_alarm_confirm_secs_cb);

			gui_switch_task(GUI_SET_VAL);
		break;

		case (2):
			if (config.altitude.alt1_flags & ALT_UNIT_I)
				gui_value_conf_P(PSTR("Alarm 1 (Low)"), GUI_VAL_NUMBER, PSTR("%0.0f ft"), config.altitude.alarm_1, config.altitude.alarm_2 + ALARM_SEPARATION, 10000, 5, gui_set_alt_alarm_alarm1_cb, FC_METER_TO_FEET);
			else
				gui_value_conf_P(PSTR("Alarm 1 (Low)"), GUI_VAL_NUMBER, PSTR("%0.0f m"), config.altitude.alarm_1, config.altitude.alarm_2 + ALARM_SEPARATION, 10000, 5, gui_set_alt_alarm_alarm1_cb);

			gui_switch_task(GUI_SET_VAL);
		break;

		case (3):
			if (config.altitude.alt1_flags & ALT_UNIT_I)
				gui_value_conf_P(PSTR("Alarm 2 (Lowest)"), GUI_VAL_NUMBER, PSTR("%0.0f ft"), config.altitude.alarm_2, 0, config.altitude.alarm_1 - ALARM_SEPARATION, 5, gui_set_alt_alarm_alarm2_cb, FC_METER_TO_FEET);
			else
				gui_value_conf_P(PSTR("Alarm 2 (Lowest)"), GUI_VAL_NUMBER, PSTR("%0.0f m"), config.altitude.alarm_2, 0, config.altitude.alarm_1 - ALARM_SEPARATION, 5, gui_set_alt_alarm_alarm2_cb);

			gui_switch_task(GUI_SET_VAL);
		break;

		case (4):
			if (config.altitude.alt1_flags & ALT_UNIT_I)
				gui_value_conf_P(PSTR("Alarm 3 (High)"), GUI_VAL_NUMBER, PSTR("%0.0f ft"), config.altitude.alarm_h1, config.altitude.alarm_1 - ALARM_SEPARATION, 5000, 5, gui_set_alt_alarm_alarm_h1_cb, FC_METER_TO_FEET);
			else
				gui_value_conf_P(PSTR("Alarm 3 (High)"), GUI_VAL_NUMBER, PSTR("%0.0f m"), config.altitude.alarm_h1, config.altitude.alarm_1 - ALARM_SEPARATION, 5000, 5, gui_set_alt_alarm_alarm_h1_cb);

			gui_switch_task(GUI_SET_VAL);
		break;

		case (5):
			if (config.altitude.alt1_flags & ALT_UNIT_I)
				gui_value_conf_P(PSTR("Alarm reset"), GUI_VAL_NUMBER, PSTR("%0.0f ft"), config.altitude.alarm_reset, 10, 1000, 1, gui_set_alt_alarm_reset_cb, FC_METER_TO_FEET);
			else
				gui_value_conf_P(PSTR("Alarm reset"), GUI_VAL_NUMBER, PSTR("%0.0f m"), config.altitude.alarm_reset, 10, 1000, 1, gui_set_alt_alarm_reset_cb);

			gui_switch_task(GUI_SET_VAL);
		break;
	}
}

void gui_set_alt_alarm_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			strcpy_P(text, PSTR("Enabled"));
			if (config.altitude.alarm_enabled)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;
		break;

		case (1):
			strcpy_P(text, PSTR("Confirm time"));
			if (config.altitude.alarm_confirm_secs == 0)
				sprintf_P(sub_text, PSTR("<Forever>"));
			else
				sprintf_P(sub_text, PSTR("%u secs"), config.altitude.alarm_confirm_secs);
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (2):
			strcpy_P(text, PSTR("Alarm 1 (Low)"));
			if (config.altitude.alt1_flags & ALT_UNIT_I)
				sprintf_P(sub_text, PSTR("%d ft"), config.altitude.alarm_1 * FC_METER_TO_FEET);
			else
				sprintf_P(sub_text, PSTR("%d m"), config.altitude.alarm_1);
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (3):
			strcpy_P(text, PSTR("Alarm 2 (Lowest)"));
			if (config.altitude.alt1_flags & ALT_UNIT_I)
				sprintf_P(sub_text, PSTR("%d ft"), config.altitude.alarm_2 * FC_METER_TO_FEET);
			else
				sprintf_P(sub_text, PSTR("%d m"), config.altitude.alarm_2);
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (4):
			strcpy_P(text, PSTR("Alarm 3 (High)"));
			if (config.altitude.alt1_flags & ALT_UNIT_I)
				sprintf_P(sub_text, PSTR("%d ft"), config.altitude.alarm_h1 * FC_METER_TO_FEET);
			else
				sprintf_P(sub_text, PSTR("%d m"), config.altitude.alarm_h1);
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (5):
			strcpy_P(text, PSTR("Reset"));
			if (config.altitude.alt1_flags & ALT_UNIT_I)
				sprintf_P(sub_text, PSTR("%d ft"), config.altitude.alarm_reset * FC_METER_TO_FEET);
			else
				sprintf_P(sub_text, PSTR("%d m"), config.altitude.alarm_reset);
			*flags |= GUI_LIST_SUB_TEXT;
		break;
	}
}

