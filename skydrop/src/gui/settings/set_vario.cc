#include "set_vario.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"

float set_vario_digital;
float set_vario_avg;

void gui_set_vario_init()
{
	gui_list_set(gui_set_vario_item, gui_set_vario_action, 7, GUI_SETTINGS);

	set_vario_digital = eeprom_read_float(&config.vario.digital_vario_dampening);
	set_vario_avg = eeprom_read_float(&config.vario.avg_vario_dampening);
}

void gui_set_vario_stop()
{
}

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
	eeprom_busy_wait();
	eeprom_write_float(&config.vario.digital_vario_dampening, val);

	if (val == 0)
		fc.digital_vario_dampening = 1;
	else
		fc.digital_vario_dampening = 1.0 / 100.0 / val;

}

void gui_set_vario_avg_int_cb(float val)
{
	gui_switch_task(GUI_SET_VARIO);
	eeprom_busy_wait();
	eeprom_write_float(&config.vario.avg_vario_dampening, val);

	if (val == 0)
		fc.avg_vario_dampening = 1;
	else
		fc.avg_vario_dampening = 1.0 / 100.0 / val;
}

void gui_set_vario_lift_cb(float val)
{
	gui_switch_task(GUI_SET_VARIO);
	eeprom_busy_wait();
	int16_t tmp = val * 100;
	eeprom_update_word((uint16_t *)&config.audio_profile.lift, tmp);
	fc.audio_lift = tmp;
}

void gui_set_vario_sink_cb(float val)
{
	gui_switch_task(GUI_SET_VARIO);
	eeprom_busy_wait();
	int16_t tmp = val * 100;
	eeprom_update_word((uint16_t *)&config.audio_profile.sink, tmp);
	fc.audio_sink = tmp;
}

void gui_set_vario_action(uint8_t index)
{
	switch(index)
	{
		case(0):
			gui_value_conf_P(PSTR("Lift threshold"), GUI_VAL_NUMBER, PSTR("%0.1f m/s"), fc.audio_lift / 100.0, fc.audio_sink / 100.0, 2, 0.1, gui_set_vario_lift_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(1):
			gui_value_conf_P(PSTR("Sink threshold"), GUI_VAL_NUMBER, PSTR("%0.1f m/s"), fc.audio_sink / 100.0, -10, fc.audio_lift / 100.0, 0.1, gui_set_vario_sink_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(2):
			gui_showmessage_P(PSTR("Not implemented"));
		break;

		case(3):
			fc.audio_fluid = !fc.audio_fluid;
			eeprom_busy_wait();
			eeprom_update_byte(&config.audio_profile.fluid, fc.audio_fluid);
		break;

		case(4):
			gui_value_conf_P(PSTR("Digital vario int."), GUI_VAL_NUMBER, PSTR("%0.1f sec"), set_vario_digital, 0, 30, 0.1, gui_set_vario_dig_int_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(5):
			gui_value_conf_P(PSTR("Average vario int."), GUI_VAL_NUMBER, PSTR("%0.1f sec"), set_vario_avg, 0, 90, 0.1, gui_set_vario_avg_int_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(6):
			fc.vario_flags ^= VARIO_UNITS_I;
			eeprom_busy_wait();
			eeprom_update_byte(&config.vario.flags, fc.vario_flags);
		break;

	}
}

void gui_set_vario_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			sprintf_P(text, PSTR("Lift threshold"));
			sprintf_P(sub_text, PSTR("%0.1f m/s"), fc.audio_lift / 100.0);
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (1):
			sprintf_P(text, PSTR("Sink threshold"));
			sprintf_P(sub_text, PSTR("%0.1f m/s"), fc.audio_sink / 100.0);
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (2):
			sprintf_P(text, PSTR("Weak lift"));
			sprintf_P(sub_text, PSTR("Not yet"));
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (3):
			sprintf_P(text, PSTR("Fluid audio"));
			if (fc.audio_fluid)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;
		break;

		case (4):
			sprintf_P(text, PSTR("Digital vario int."));
			sprintf_P(sub_text, PSTR("%0.1f s"), set_vario_digital);
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (5):
			sprintf_P(text, PSTR("Average vario int."));
			sprintf_P(sub_text, PSTR("%0.1f s"), set_vario_avg);
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (6):
			sprintf_P(text, PSTR("Units"));
			*flags |= GUI_LIST_SUB_TEXT;
			if (fc.vario_flags & VARIO_UNITS_I)
				sprintf_P(sub_text, PSTR("imperial"));
			else
				sprintf_P(sub_text, PSTR("metric"));
		break;
	}
}

