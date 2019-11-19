#include <gui/settings/set_vario_advanced.h>
#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"
#include "../../fc/vario.h"

void gui_set_vario_advanced_init()
{
	gui_list_set(gui_set_vario_advanced_item, gui_set_vario_advanced_action, 9, GUI_SET_VARIO);
}

void gui_set_vario_advanced_weak_cb(float val)
{
	uint16_t tmp = val * 100;

	config.audio_profile.weak = config.audio_profile.lift - tmp;
	eeprom_busy_wait();
	eeprom_update_block((void *)&config.audio_profile.weak, &config_ee.audio_profile.weak, sizeof(config.audio_profile.weak));
	gui_switch_task(GUI_SET_VARIO_ADVANCED);
}

void gui_set_vario_advanced_poffset_cb(float val)
{
	uint16_t tmp = val;

	config.audio_profile.prebeep_offset = tmp;
	eeprom_busy_wait();
	eeprom_update_byte(&config_ee.audio_profile.prebeep_offset, config.audio_profile.prebeep_offset);
	gui_switch_task(GUI_SET_VARIO_ADVANCED);
}

void gui_set_vario_advanced_plength_cb(float val)
{
	uint16_t tmp = val;

	config.audio_profile.prebeep_length = tmp;
	eeprom_busy_wait();
	eeprom_update_byte(&config_ee.audio_profile.prebeep_length, config.audio_profile.prebeep_length);
	gui_switch_task(GUI_SET_VARIO_ADVANCED);
}

void gui_set_vario_advanced_dig_int_cb(float val)
{
	config.vario.digital_vario_dampening = sec_to_mul(val);
	eeprom_busy_wait();
	eeprom_update_float(&config_ee.vario.digital_vario_dampening, config.vario.digital_vario_dampening);

	gui_switch_task(GUI_SET_VARIO_ADVANCED);
}

void gui_set_vario_advanced_avg_int_cb(float val)
{
	config.vario.avg_vario_dampening = sec_to_mul(val);
	vario_update_history_delay();
	eeprom_busy_wait();
	eeprom_update_float(&config_ee.vario.avg_vario_dampening, config.vario.avg_vario_dampening);

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
	switch (index)
	{
		case(0):
			config.audio_profile.flags ^= AUDIO_FLUID;
			eeprom_busy_wait();
			eeprom_update_byte(&config_ee.audio_profile.flags, config.audio_profile.flags);
		break;

		case(1):
			config.audio_profile.flags ^= AUDIO_WEAK;
			eeprom_busy_wait();
			eeprom_update_byte(&config_ee.audio_profile.flags, config.audio_profile.flags);
		break;

		case(2):
			gui_value_conf_P(PSTR("Weak threshold"), GUI_VAL_NUMBER, PSTR("%+0.1f"),  (float)(config.audio_profile.lift - config.audio_profile.weak) / 100.0, config.audio_profile.sink / 100.0, config.audio_profile.lift / 100.0, 0.1, gui_set_vario_advanced_weak_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(3):
			config.audio_profile.flags ^= AUDIO_BEEP_SINK;
			eeprom_busy_wait();
			eeprom_update_byte(&config_ee.audio_profile.flags, config.audio_profile.flags);
		break;

		case(4):
			gui_value_conf_P(PSTR("Weak/Beep offset"), GUI_VAL_NUMBER, PSTR("%1.0f Hz"), config.audio_profile.prebeep_offset, 5, 250, 1, gui_set_vario_advanced_poffset_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(5):
			gui_value_conf_P(PSTR("Weak/Beep duration"), GUI_VAL_NUMBER, PSTR("%1.0f ms"), config.audio_profile.prebeep_length, 10, 250, 1, gui_set_vario_advanced_plength_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(6):
			gui_value_conf_P(PSTR("Digital vario int."), GUI_VAL_NUMBER, PSTR("%0.1f sec"), mul_to_sec(config.vario.digital_vario_dampening), 0, 30, 0.1, gui_set_vario_advanced_dig_int_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(7):
			gui_value_conf_P(PSTR("Average vario int."), GUI_VAL_NUMBER, PSTR("%0.1f sec"), mul_to_sec(config.vario.avg_vario_dampening), 1, 90, 0.1, gui_set_vario_advanced_avg_int_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(8):
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
			strcpy_P(text, PSTR("Weak lift"));
			if (config.audio_profile.flags & AUDIO_WEAK)
				*flags = GUI_LIST_CHECK_ON;
			else
				*flags = GUI_LIST_CHECK_OFF;
		break;

		case (2):
			strcpy_P(text, PSTR("Weak lift threshold"));
			sprintf_P(sub_text, PSTR("%+0.1f m/s"), (float)(config.audio_profile.lift - config.audio_profile.weak) / 100.0);
			*flags = GUI_LIST_SUB_TEXT;
		break;

		case (3):
			strcpy_P(text, PSTR("Beep sink"));
			if (config.audio_profile.flags & AUDIO_BEEP_SINK)
				*flags = GUI_LIST_CHECK_ON;
			else
				*flags = GUI_LIST_CHECK_OFF;
		break;

		case (4):
			strcpy_P(text, PSTR("Weak/Beep offset"));
			sprintf_P(sub_text, PSTR("%u Hz"), config.audio_profile.prebeep_offset);
			*flags = GUI_LIST_SUB_TEXT;
		break;

		case (5):
			strcpy_P(text, PSTR("Weak/Beep duration"));
			sprintf_P(sub_text, PSTR("%u ms"), config.audio_profile.prebeep_length);
			*flags = GUI_LIST_SUB_TEXT;
		break;

		case (6):
			strcpy_P(text, PSTR("Digital vario int."));
			sprintf_P(sub_text, PSTR("%0.1f s"), mul_to_sec(config.vario.digital_vario_dampening));
			*flags = GUI_LIST_SUB_TEXT;
		break;

		case (7):
			strcpy_P(text, PSTR("Average vario int."));
			sprintf_P(sub_text, PSTR("%0.1f s"), mul_to_sec(config.vario.avg_vario_dampening));
			*flags = GUI_LIST_SUB_TEXT;
		break;

		case (8):
			strcpy_P(text, PSTR("Vario demo"));
		break;
	}
}
