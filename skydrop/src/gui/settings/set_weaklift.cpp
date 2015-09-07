#include "set_weaklift.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"

void gui_set_weaklift_init()
{
	gui_list_set(gui_set_weaklift_item, gui_set_weaklift_action, 3, GUI_SET_VARIO);
}

void gui_set_weaklift_stop() {}

void gui_set_weaklift_loop()
{
	gui_list_draw();
}

void gui_set_weaklift_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_set_weaklift_val_cb(float val)
{
	uint16_t tmp = val * 10;

	config.vario.weak_lift = tmp;
	eeprom_busy_wait();
	eeprom_update_byte(&config_ee.vario.weak_lift, config.vario.weak_lift);
	gui_switch_task(GUI_SET_WEEKLIFT);
}

void gui_set_weaklift_freq_cb(float val)
{
	uint16_t tmp = val;

	config.audio_profile.weak_lift_freq = tmp;

	eeprom_busy_wait();
	eeprom_update_word(&config_ee.audio_profile.weak_lift_freq, config.audio_profile.weak_lift_freq);
	gui_switch_task(GUI_SET_WEEKLIFT);
}

void gui_set_weaklift_action(uint8_t index)
{
	switch (index)
	{
		case(0):
			config.vario.weak_lift_enabled = !config.vario.weak_lift_enabled;
			eeprom_busy_wait();
			eeprom_update_byte(&config_ee.vario.weak_lift_enabled, config.vario.weak_lift_enabled);
		break;

		case(1):
			gui_value_conf_P(PSTR("Threshold"), GUI_VAL_NUMBER, PSTR("%1.0f"), config.vario.weak_lift / 10, 1, 20, 1, gui_set_weaklift_val_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(2):
			gui_value_conf_P(PSTR("Buzzer frequency"), GUI_VAL_NUMBER, PSTR("%1.0f Hz"), config.audio_profile.weak_lift_freq, 10, 2000, 10, gui_set_weaklift_freq_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

	}
}

void gui_set_weaklift_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			sprintf_P(text, PSTR("Enabled"));
			if (config.vario.weak_lift_enabled)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;
		break;

		case (1):
			sprintf_P(text, PSTR("Weak lift setting"));
			sprintf_P(sub_text, PSTR("%d (%0.1f m/s)"), config.vario.weak_lift / 10, (float)(config.audio_profile.lift - config.vario.weak_lift) / 100.0);
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (2):
			sprintf_P(text, PSTR("Buzzer frequency"));
			sprintf_P(sub_text, PSTR("%d Hz"), config.audio_profile.weak_lift_freq);
			*flags |= GUI_LIST_SUB_TEXT;
		break;
	}
}
