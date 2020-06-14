#include <gui/settings/set_vario_advanced.h>
#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"
#include "../../fc/vario.h"

void gui_set_vario_advanced_init()
{
	gui_list_set(gui_set_vario_advanced_item, gui_set_vario_advanced_action, 11, GUI_SET_VARIO);
}

void gui_set_vario_advanced_weak_cb(float val)
{
	uint16_t tmp = val * 100;

	config.audio_profile.weak = config.audio_profile.lift - tmp;
	
	ee_update_block((void *)&config.audio_profile.weak, &config_ee.audio_profile.weak, sizeof(config.audio_profile.weak));
	gui_switch_task(GUI_SET_VARIO_ADVANCED);
}

void gui_set_vario_advanced_poffset_cb(float val)
{
	uint16_t tmp = val;

	config.audio_profile.prebeep_offset = tmp;
	
	ee_update_word(&config_ee.audio_profile.prebeep_offset, config.audio_profile.prebeep_offset);
	gui_switch_task(GUI_SET_VARIO_ADVANCED);
}

void gui_set_vario_advanced_low_weak_cb(float val)
{
	uint16_t tmp = val;

	config.audio_profile.weak_low_freq = tmp;
	
	ee_update_word(&config_ee.audio_profile.weak_low_freq, config.audio_profile.weak_low_freq);
	gui_switch_task(GUI_SET_VARIO_ADVANCED);
}

void gui_set_vario_advanced_high_weak_cb(float val)
{
	uint16_t tmp = val;

	config.audio_profile.weak_high_freq = tmp;

	
	ee_update_word(&config_ee.audio_profile.weak_high_freq, config.audio_profile.weak_high_freq);
	gui_switch_task(GUI_SET_VARIO_ADVANCED);
}

void gui_set_vario_advanced_plength_cb(float val)
{
	uint16_t tmp = val;

	config.audio_profile.prebeep_length = tmp;
	
	ee_update_byte(&config_ee.audio_profile.prebeep_length, config.audio_profile.prebeep_length);
	gui_switch_task(GUI_SET_VARIO_ADVANCED);
}

void gui_set_vario_advanced_dig_int_cb(float val)
{
	config.vario.digital_vario_dampening = sec_to_mul(val);
	
	ee_update_float(&config_ee.vario.digital_vario_dampening, config.vario.digital_vario_dampening);

	gui_switch_task(GUI_SET_VARIO_ADVANCED);
}

void gui_set_vario_advanced_avg_int_cb(float val)
{
	config.vario.avg_vario_dampening = sec_to_mul(val);
	vario_update_history_delay();
	
	ee_update_float(&config_ee.vario.avg_vario_dampening, config.vario.avg_vario_dampening);

	gui_switch_task(GUI_SET_VARIO_ADVANCED);
}

void gui_set_vario_advanced_demo_cb(float val)
{
	gui_buttons_override = false;
	audio_demo = false;
	audio_demo_val = 0;
	audio_off();

	gui_switch_task(GUI_SET_VARIO_ADVANCED);
}

void gui_set_vario_advanced_action(uint8_t index)
{
	uint8_t mode;

	switch (index)
	{
		case(0):
			config.audio_profile.flags ^= AUDIO_FLUID;
			
			ee_update_byte(&config_ee.audio_profile.flags, config.audio_profile.flags);
		break;

		case(1):
			mode = (config.audio_profile.flags & AUDIO_SINK_MASK);
			mode = (mode + 1) % AUDIO_SINK_MODES;
			config.audio_profile.flags = (config.audio_profile.flags & ~AUDIO_SINK_MASK) | mode;
			
			ee_update_byte(&config_ee.audio_profile.flags, config.audio_profile.flags);
		break;

		case(2):
			mode = (config.audio_profile.flags & AUDIO_WEAK_MASK) >> 2;
			mode = (mode + 1) % AUDIO_WEAK_MODES;
			config.audio_profile.flags = (config.audio_profile.flags & ~AUDIO_WEAK_MASK) | (mode << 2);
			
			ee_update_byte(&config_ee.audio_profile.flags, config.audio_profile.flags);
		break;

		case(3):
			gui_value_conf_P(PSTR("Weak threshold"), GUI_VAL_NUMBER, PSTR("%+0.1f"),  (float)(config.audio_profile.lift - config.audio_profile.weak) / 100.0, config.audio_profile.sink / 100.0, config.audio_profile.lift / 100.0, 0.1, gui_set_vario_advanced_weak_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(4):
			gui_value_conf_P(PSTR("Beep offset"), GUI_VAL_NUMBER, PSTR("%1.0f Hz"), config.audio_profile.prebeep_offset, 10, 3000, 10, gui_set_vario_advanced_poffset_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(5):
			gui_value_conf_P(PSTR("Beep duration"), GUI_VAL_NUMBER, PSTR("%1.0f ms"), config.audio_profile.prebeep_length, 10, 250, 1, gui_set_vario_advanced_plength_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(6):
			gui_value_conf_P(PSTR("Weak low freq."), GUI_VAL_NUMBER, PSTR("%1.0f Hz"), config.audio_profile.weak_low_freq, 10, 3000, 10, gui_set_vario_advanced_low_weak_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(7):
			gui_value_conf_P(PSTR("Weak high freq."), GUI_VAL_NUMBER, PSTR("%1.0f Hz"), config.audio_profile.weak_high_freq, 10, 3000, 10, gui_set_vario_advanced_high_weak_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(8):
			gui_value_conf_P(PSTR("Digital vario int."), GUI_VAL_NUMBER, PSTR("%0.1f sec"), mul_to_sec(config.vario.digital_vario_dampening), 0, 30, 0.1, gui_set_vario_advanced_dig_int_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(9):
			gui_value_conf_P(PSTR("Average vario int."), GUI_VAL_NUMBER, PSTR("%0.1f sec"), mul_to_sec(config.vario.avg_vario_dampening), 1, 90, 0.1, gui_set_vario_advanced_avg_int_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(10):
			gui_value_conf_P(PSTR("Vario demo"), GUI_VAL_VARIO_TEST, PSTR("%+0.1f m/s"), 0.0, -10.0, +10.0, 0.1, gui_set_vario_advanced_demo_cb);
			gui_switch_task(GUI_SET_VAL);
			audio_demo_val = 0;
			audio_demo = true;
			gui_buttons_override = true;
		break;
	}
}

void gui_set_vario_advanced_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			strcpy_P(text, PSTR("Fluid audio"));
			if (config.audio_profile.flags & AUDIO_FLUID)
				*flags = GUI_LIST_CHECK_ON;
			else
				*flags = GUI_LIST_CHECK_OFF;
		break;

		case (1):
			strcpy_P(text, PSTR("Sink mode"));
			*flags = GUI_LIST_SUB_TEXT;
			switch (config.audio_profile.flags & AUDIO_SINK_MASK)
			{
				case(AUDIO_SINK_OFF):
					strcpy_P(sub_text, PSTR("Disabled"));
					break;
				case(AUDIO_SINK_BEEP):
					strcpy_P(sub_text, PSTR("Beeps"));
					break;
				case(AUDIO_SINK_CONT):
					strcpy_P(sub_text, PSTR("Continuous"));
					break;
			}
		break;

		case (2):
			strcpy_P(text, PSTR("Weak lift mode"));
			*flags = GUI_LIST_SUB_TEXT;
			switch (config.audio_profile.flags & AUDIO_WEAK_MASK)
			{
				case(AUDIO_WEAK_OFF):
					strcpy_P(sub_text, PSTR("Disabled"));
					break;
				case(AUDIO_WEAK_BEEP):
					strcpy_P(sub_text, PSTR("Beeps"));
					break;
				case(AUDIO_WEAK_CONT):
					strcpy_P(sub_text, PSTR("Continuous"));
					break;
			}
		break;

		case (3):
			strcpy_P(text, PSTR("Weak lift threshold"));
			sprintf_P(sub_text, PSTR("%+0.1f m/s"), (float)(config.audio_profile.lift - config.audio_profile.weak) / 100.0);
			*flags = GUI_LIST_SUB_TEXT;
		break;

		case (4):
			strcpy_P(text, PSTR("Beep offset"));
			sprintf_P(sub_text, PSTR("%u Hz"), config.audio_profile.prebeep_offset);
			*flags = GUI_LIST_SUB_TEXT;
		break;

		case (5):
			strcpy_P(text, PSTR("Beep duration"));
			sprintf_P(sub_text, PSTR("%u ms"), config.audio_profile.prebeep_length);
			*flags = GUI_LIST_SUB_TEXT;
		break;

		case (6):
			strcpy_P(text, PSTR("Weak low freq."));
			sprintf_P(sub_text, PSTR("%u Hz"), config.audio_profile.weak_low_freq);
			*flags = GUI_LIST_SUB_TEXT;
		break;

		case (7):
			strcpy_P(text, PSTR("Weak high freq."));
			sprintf_P(sub_text, PSTR("%u Hz"), config.audio_profile.weak_high_freq);
			*flags = GUI_LIST_SUB_TEXT;
		break;

		case (8):
			strcpy_P(text, PSTR("Digital vario int."));
			sprintf_P(sub_text, PSTR("%0.1f s"), mul_to_sec(config.vario.digital_vario_dampening));
			*flags = GUI_LIST_SUB_TEXT;
		break;

		case (9):
			strcpy_P(text, PSTR("Average vario int."));
			sprintf_P(sub_text, PSTR("%0.1f s"), mul_to_sec(config.vario.avg_vario_dampening));
			*flags = GUI_LIST_SUB_TEXT;
		break;

		case (10):
			strcpy_P(text, PSTR("Vario demo"));
		break;
	}
}
