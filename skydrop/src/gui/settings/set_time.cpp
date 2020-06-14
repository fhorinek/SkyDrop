#include "set_time.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"

void gui_set_time_init()
{
	gui_list_set(gui_set_time_item, gui_set_time_action, 5, GUI_SET_SYSTEM);
}

void gui_set_time_time_cb(float val)
{
	gui_switch_task(GUI_SET_TIME);
	time_set_flags();
}

void gui_set_time_timezone_cb(float val)
{
	int8_t tmp = val * 2;
	
	config.system.time_zone = tmp;
	ee_update_block((void *)&config.system.time_zone, (void *)&config_ee.system.time_zone, sizeof(int8_t));
	gui_switch_task(GUI_SET_TIME);
}

void gui_set_time_action(uint8_t index)
{
	switch(index)
	{
	case(0):
		gui_value_conf_P(PSTR("Time"), GUI_VAL_SYSTEM_TIME, PSTR(""), 0, 0, 0, 1, gui_set_time_time_cb);
		gui_switch_task(GUI_SET_VAL);
	break;

	case(1):
		gui_value_conf_P(PSTR("Date"), GUI_VAL_SYSTEM_DATE, PSTR(""), 0, 0, 0, 1, gui_set_time_time_cb);
		gui_switch_task(GUI_SET_VAL);
	break;

	case(2):
		gui_value_conf_P(PSTR("Time zone"), GUI_VAL_NUMBER, PSTR("UTC %+0.1f"), config.system.time_zone / 2.0, -12, 12, 0.5, gui_set_time_timezone_cb);
		gui_switch_task(GUI_SET_VAL);
	break;

	case(3):
		config.system.time_flags ^= TIME_DST;

		if (config.system.time_flags & TIME_DST)
		{
			config.system.time_zone = config.system.time_zone + 2;
			time_set_local(time_get_local() + 3600);
		}
		else
		{
			config.system.time_zone = config.system.time_zone - 2;
			time_set_local(time_get_local() - 3600);
		}

		
		ee_update_block((void *)&config.system.time_zone, (void *)&config_ee.system.time_zone, sizeof(int8_t));
		ee_update_byte(&config_ee.system.time_flags, config.system.time_flags);
	break;

	case(4):
		config.system.time_flags ^= TIME_SYNC;
		
		ee_update_byte(&config_ee.system.time_flags, config.system.time_flags);
	break;
	}
}

void gui_set_time_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t day;
	uint8_t wday;
	uint8_t month;
	uint16_t year;

	datetime_from_epoch(time_get_local(), &sec, &min, &hour, &day, &wday, &month, &year);

	switch (index)
	{
		case (0):
			strcpy_P(text, PSTR("Time"));
			sprintf_P(sub_text, PSTR("%02d:%02d.%02d"), hour, min, sec);
			*flags = GUI_LIST_SUB_TEXT;
		break;

		case (1):
			strcpy_P(text, PSTR("Date"));
			sprintf_P(sub_text, PSTR("%02d/%02d/%04d"), day, month, year);
			*flags = GUI_LIST_SUB_TEXT;
		break;

		case (2):
			strcpy_P(text, PSTR("Time Zone"));
			sprintf_P(sub_text, PSTR("UTC %+0.1f"), config.system.time_zone / 2.0);
			*flags = GUI_LIST_SUB_TEXT;
		break;

		case (3):
			strcpy_P(text, PSTR("DST"));
			if (config.system.time_flags & TIME_DST)
				*flags = GUI_LIST_CHECK_ON;
			else
				*flags = GUI_LIST_CHECK_OFF;
		break;

		case (4):
			strcpy_P(text, PSTR("Sync with GPS"));
			if (config.system.time_flags & TIME_SYNC)
				*flags = GUI_LIST_CHECK_ON;
			else
				*flags = GUI_LIST_CHECK_OFF;
		break;
	}
}

