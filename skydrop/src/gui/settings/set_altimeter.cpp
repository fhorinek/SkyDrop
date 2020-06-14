#include "set_altimeter.h"

#include "../gui_list.h"
#include "../gui_value.h"
#include "../gui_dialog.h"

#include "../../fc/conf.h"

volatile uint8_t set_alt_index;
volatile uint8_t set_alt_flags;
volatile uint8_t set_alt_list_num;

void gui_set_altimeter_index(uint8_t index)
{
	set_alt_index = index;
}

void gui_set_altimeter_list()
{
	set_alt_list_num = 4; //mode, relative to, zero at take off, units
	if ((set_alt_flags & ALT_MODE_MASK) != ALT_DIFF)
		set_alt_list_num = 3; //mode, units, sync with gps
	if ((set_alt_flags & ALT_MODE_MASK) == ALT_ABS_GPS)
		set_alt_list_num = 2; //mode, units

	gui_list_set(gui_set_altimeter_item, gui_set_altimeter_action, set_alt_list_num, GUI_SET_ALTIMETERS);
}

void gui_set_altimeter_init()
{
	if (set_alt_index == 0)
		set_alt_flags = config.altitude.alt1_flags;
	else
		set_alt_flags = config.altitude.altimeter[set_alt_index - 1].flags;

	gui_set_altimeter_list();
}

void gui_set_altimeter_stop()
{
	if (set_alt_index == 0)
	{
		config.altitude.alt1_flags = set_alt_flags;
		
		ee_update_byte(&config_ee.altitude.alt1_flags, set_alt_flags);
	}
	else
	{
		config.altitude.altimeter[set_alt_index - 1].flags = set_alt_flags;
		
		ee_update_byte(&config_ee.altitude.altimeter[set_alt_index - 1].flags, set_alt_flags);
	}

}

void gui_set_altimeter_loop()
{
	gui_list_draw();
}

void gui_set_altimeter_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

bool set_alt_find_abs(uint8_t index, uint8_t rep)
{
	//cyclic binding
	if (rep > NUMBER_OF_ALTIMETERS + 1)
	{
		return false;
	}

	//alt1 is always absolute
	if (index == 0)
	{
		return true;
	}

	//prevent cyclic binding
	if (index == set_alt_index)
	{
		return false;
	}

	uint8_t flags = config.altitude.altimeter[index - 1].flags;

	if ((flags & ALT_MODE_MASK) == ALT_DIFF)
	{
		//binded altimeter is relative
		return set_alt_find_abs(flags & ALT_REL_MASK, rep + 1);
	}

	//else is absolute or gps
	return true;
}

void gui_set_altimeter_gps_alt(uint8_t ret)
{
	if (ret == GUI_DIALOG_YES)
	{
		if (fc.gps_data.valid)
		{
			uint8_t a_type  = set_alt_flags & ALT_MODE_MASK;

			if (a_type == ALT_ABS_QNH1)
			{
				config.altitude.QNH1 = fc_alt_to_qnh(fc.gps_data.altitude, fc.vario.pressure);
				fc.vario.error_over_time = 0;
				fc.vario.time_of_last_error_update = task_get_ms_tick();
				fc_manual_alt0_change(fc.gps_data.altitude);
			}

			if (a_type == ALT_ABS_QNH2)
			{
				config.altitude.QNH2 = fc_alt_to_qnh(fc.gps_data.altitude, fc.vario.pressure);
			}

			if (a_type == ALT_DIFF)
			{
				uint8_t a_index = set_alt_flags & ALT_REL_MASK;
				int16_t p_alt;

				if (a_index == 0)
					p_alt = fc.altitude1;
				else
					p_alt = fc.altitudes[a_index - 1];

				config.altitude.altimeter[set_alt_index - 1].delta = fc.gps_data.altitude - p_alt;
			}

		}
	}

	gui_switch_task(GUI_SET_ALTIMETER);
}

void gui_set_altimeter_action(uint8_t index)
{
	uint8_t tmp;

	if (index == 0) //Mode
	{
		if (set_alt_index != 0)
		{
			tmp = (set_alt_flags & ALT_MODE_MASK) >> 6;
			tmp = (tmp + 1) % 4;
			set_alt_flags = (set_alt_flags & ~ALT_MODE_MASK) | (tmp << 6);
			gui_set_altimeter_list();
		}
		else
		{
			gui_showmessage_P(PSTR("Can't change\nAlt1 mode"));
		}
		return;
	}

	if (set_alt_list_num == 4)
	{
		if (index == 1)
		{
			tmp = set_alt_flags & ALT_REL_MASK;

			do
			{
				tmp = (tmp + 1) % (NUMBER_OF_ALTIMETERS + 1);
			}
			while (!set_alt_find_abs(tmp, 0));

			set_alt_flags = (set_alt_flags & ~ALT_REL_MASK) | (tmp);
			return;
		}

		if (index == 2)
		{
			set_alt_flags ^= ALT_AUTO_ZERO;
			return;
		}
	}

	if ((index == 3 && set_alt_list_num == 4) || index == 1)
	{
		set_alt_flags ^= ALT_UNIT_I;
		return;
	}

	if (index == 2)
	{
		set_alt_flags ^= ALT_AUTO_GPS;
		return;
	}
}




void gui_set_altimeter_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	if (index == 0) //Same for every altmeter
	{
		strcpy_P(text, PSTR("Mode"));
		*flags =  GUI_LIST_SUB_TEXT;
		switch (set_alt_flags & 0b11000000)
		{
			case(ALT_ABS_QNH1):
				strcpy_P(sub_text, PSTR("Absolute QNH1"));
			break;
			case(ALT_ABS_QNH2):
				strcpy_P(sub_text, PSTR("Absolute QNH2"));
			break;
			case(ALT_ABS_GPS):
				strcpy_P(sub_text, PSTR("Absolute GPS"));
			break;
			case(ALT_DIFF):
				strcpy_P(sub_text, PSTR("Relative"));
		}
		return;
	}

	if (set_alt_list_num == 4) //Relative only
	{
		if (index == 1)
		{
			strcpy_P(text, PSTR("Relative to"));
			*flags =  GUI_LIST_SUB_TEXT;
			sprintf_P(sub_text, PSTR("Altimeter %d"), (set_alt_flags & 0b00001111) + 1);
			return;
		}

		if (index == 2)
		{
			strcpy_P(text, PSTR("Zero at Take off"));
			if (set_alt_flags & ALT_AUTO_ZERO)
				*flags =  GUI_LIST_CHECK_ON;
			else
				*flags =  GUI_LIST_CHECK_OFF;
			return;
		}
	}

	if ((index == 3 && set_alt_list_num == 4) || index == 1) //Units
	{
		strcpy_P(text, PSTR("Units"));
		*flags =  GUI_LIST_SUB_TEXT;
		if (set_alt_flags & ALT_UNIT_I)
			strcpy_P(sub_text, PSTR("imperial"));
		else
			strcpy_P(sub_text, PSTR("metric"));
		return;
	}

	if (index == 2) //for QNH only
	{
		strcpy_P(text, PSTR("Auto from GPS"));
		if (set_alt_flags & ALT_AUTO_GPS)
			*flags =  GUI_LIST_CHECK_ON;
		else
			*flags =  GUI_LIST_CHECK_OFF;

		return;
	}
}

