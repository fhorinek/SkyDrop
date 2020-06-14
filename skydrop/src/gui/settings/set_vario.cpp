#include "set_vario.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"

void gui_set_vario_init()
{
	gui_list_set(gui_set_vario_item, gui_set_vario_action, 5, GUI_SETTINGS);
}

void gui_set_vario_lift_cb(float val)
{
	gui_switch_task(GUI_SET_VARIO);
	
	int16_t tmp = val * 100;
	ee_update_word((uint16_t *)&config_ee.audio_profile.lift, tmp);
	config.audio_profile.lift = tmp;
}

void gui_set_vario_sink_cb(float val)
{
	gui_switch_task(GUI_SET_VARIO);
	
	int16_t tmp = val * 100;
	ee_update_word((uint16_t *)&config_ee.audio_profile.sink, tmp);
	config.audio_profile.sink = tmp;
}



void gui_set_vario_action(uint8_t index)
{
	switch(index)
	{
		case(0):
			gui_value_conf_P(PSTR("Lift threshold"), GUI_VAL_NUMBER, PSTR("%+0.1f m/s"), config.audio_profile.lift / 100.0, config.audio_profile.sink / 100.0, 2, 0.1, gui_set_vario_lift_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(1):
			gui_value_conf_P(PSTR("Sink threshold"), GUI_VAL_NUMBER, PSTR("%+0.1f m/s"), config.audio_profile.sink / 100.0, -10, config.audio_profile.lift / 100.0, 0.1, gui_set_vario_sink_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(2):
			config.vario.flags ^= VARIO_UNITS_I;
			
			ee_update_byte(&config_ee.vario.flags, config.vario.flags);
		break;

		case(3):
			config.vario.flags ^= VARIO_USE_ACC;
			
			ee_update_byte(&config_ee.vario.flags, config.vario.flags);
		break;

		case(4):
			gui_switch_task(GUI_SET_VARIO_ADVANCED);
		break;

	}
}

void gui_set_vario_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			strcpy_P(text, PSTR("Lift threshold"));
			sprintf_P(sub_text, PSTR("%+0.1f m/s"), config.audio_profile.lift / 100.0);
			*flags = GUI_LIST_SUB_TEXT;
		break;

		case (1):
			strcpy_P(text, PSTR("Sink threshold"));
			sprintf_P(sub_text, PSTR("%+0.1f m/s"), config.audio_profile.sink / 100.0);
			*flags = GUI_LIST_SUB_TEXT;
		break;

		case (2):
			strcpy_P(text, PSTR("Units"));
			*flags = GUI_LIST_SUB_TEXT;
			if (config.vario.flags & VARIO_UNITS_I)
				strcpy_P(sub_text, PSTR("imperial"));
			else
				strcpy_P(sub_text, PSTR("metric"));
		break;

		case (3):
			strcpy_P(text, PSTR("Use accel"));
			if (config.vario.flags & VARIO_USE_ACC)
				*flags = GUI_LIST_CHECK_ON;
			else
				*flags = GUI_LIST_CHECK_OFF;
		break;

		case (4):
			strcpy_P(text, PSTR("Advanced"));
		break;


	}
}

