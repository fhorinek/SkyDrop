#include "set_vario.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"
#include "../../fc/vario.h"


void gui_set_vario_init()
{
	gui_list_set(gui_set_vario_item, gui_set_vario_action, 9, GUI_SETTINGS);
}

void gui_set_vario_stop() {}

void gui_set_vario_loop()
{
	gui_list_draw();
}

void gui_set_vario_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_set_vario_dig_int_cb(float val)
{
	gui_switch_task(GUI_SET_VARIO);

	config.vario.digital_vario_dampening = sec_to_mul(val);
	eeprom_busy_wait();
	eeprom_write_float(&config_ee.vario.digital_vario_dampening, config.vario.digital_vario_dampening);

}

void gui_set_vario_avg_int_cb(float val)
{
	gui_switch_task(GUI_SET_VARIO);

	config.vario.avg_vario_dampening = sec_to_mul(val);
	vario_update_history_delay();
	eeprom_busy_wait();
	eeprom_write_float(&config_ee.vario.avg_vario_dampening, config.vario.avg_vario_dampening);
}

void gui_set_vario_lift_cb(float val)
{
	gui_switch_task(GUI_SET_VARIO);
	eeprom_busy_wait();
	int16_t tmp = val * 100;
	eeprom_update_word((uint16_t *)&config_ee.audio_profile.lift, tmp);
	config.audio_profile.lift = tmp;
}

void gui_set_vario_sink_cb(float val)
{
	gui_switch_task(GUI_SET_VARIO);
	eeprom_busy_wait();
	int16_t tmp = val * 100;
	eeprom_update_word((uint16_t *)&config_ee.audio_profile.sink, tmp);
	config.audio_profile.sink = tmp;
}

void gui_set_vario_demo_cb(float val)
{
	gui_buttons_override = false;
	audio_demo = false;
	audio_demo_val = 0;
	audio_off();
	gui_switch_task(GUI_SET_VARIO);
}


void gui_set_vario_action(uint8_t index)
{
	switch(index)
	{
		case(0):
			gui_value_conf_P(PSTR("Lift threshold"), GUI_VAL_NUMBER, PSTR("%0.1f m/s"), config.audio_profile.lift / 100.0, config.audio_profile.sink / 100.0, 2, 0.1, gui_set_vario_lift_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(1):
			gui_value_conf_P(PSTR("Sink threshold"), GUI_VAL_NUMBER, PSTR("%0.1f m/s"), config.audio_profile.sink / 100.0, -10, config.audio_profile.lift / 100.0, 0.1, gui_set_vario_sink_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(2):
			gui_switch_task(GUI_SET_WEEKLIFT);
		break;

		case(3):
			config.audio_profile.fluid = !config.audio_profile.fluid;
			eeprom_busy_wait();
			eeprom_update_byte(&config_ee.audio_profile.fluid, config.audio_profile.fluid);
		break;

		case(4):
			gui_value_conf_P(PSTR("Digital vario int."), GUI_VAL_NUMBER, PSTR("%0.1f sec"), mul_to_sec(config.vario.digital_vario_dampening), 0, 30, 0.1, gui_set_vario_dig_int_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(5):
			gui_value_conf_P(PSTR("Average vario int."), GUI_VAL_NUMBER, PSTR("%0.1f sec"), mul_to_sec(config.vario.avg_vario_dampening), 1, 90, 0.1, gui_set_vario_avg_int_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(6):
			config.vario.flags ^= VARIO_UNITS_I;
			eeprom_busy_wait();
			eeprom_update_byte(&config_ee.vario.flags, config.vario.flags);
		break;

		case(7):
			config.vario.flags ^= VARIO_USE_ACC;
			eeprom_busy_wait();
			eeprom_update_byte(&config_ee.vario.flags, config.vario.flags);
		break;

		case(8):
			gui_value_conf_P(PSTR("Vario demo"), GUI_VAL_VARIO_TEST, PSTR("%+0.1f m/s"), 0.0, -10.0, +10.0, 0.1, gui_set_vario_demo_cb);
			gui_switch_task(GUI_SET_VAL);
			audio_demo_val = 0;
			audio_demo = true;
			gui_buttons_override = true;
		break;
	}
}

void gui_set_vario_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			strcpy_P(text, PSTR("Lift threshold"));
			sprintf_P(sub_text, PSTR("%0.1f m/s"), config.audio_profile.lift / 100.0);
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (1):
			strcpy_P(text, PSTR("Sink threshold"));
			sprintf_P(sub_text, PSTR("%0.1f m/s"), config.audio_profile.sink / 100.0);
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (2):
			strcpy_P(text, PSTR("Weak lift"));
			if (config.vario.weak_lift_enabled)
				sprintf_P(sub_text, PSTR("%0.1fm/s"), (float)(config.audio_profile.lift - config.vario.weak_lift) / 100.0);
			else
				strcpy_P(sub_text, PSTR("disabled"));
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (3):
			strcpy_P(text, PSTR("Fluid audio"));
			if (config.audio_profile.fluid)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;
		break;

		case (4):
			strcpy_P(text, PSTR("Digital vario int."));
			sprintf_P(sub_text, PSTR("%0.1f s"), mul_to_sec(config.vario.digital_vario_dampening));
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (5):
			strcpy_P(text, PSTR("Average vario int."));
			sprintf_P(sub_text, PSTR("%0.1f s"), mul_to_sec(config.vario.avg_vario_dampening));
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (6):
			strcpy_P(text, PSTR("Units"));
			*flags |= GUI_LIST_SUB_TEXT;
			if (config.vario.flags & VARIO_UNITS_I)
				strcpy_P(sub_text, PSTR("imperial"));
			else
				strcpy_P(sub_text, PSTR("metric"));
		break;

		case (7):
			strcpy_P(text, PSTR("Use accel"));
			if (config.vario.flags & VARIO_USE_ACC)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;
		break;

		case (8):
			strcpy_P(text, PSTR("Vario demo"));
			*flags |= GUI_LIST_FOLDER;
		break;

	}
}

