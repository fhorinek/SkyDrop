#include "set_audio.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"

void gui_set_audio_init()
{
	gui_list_set(gui_set_audio_item, gui_set_audio_action, 5);

	eeprom_busy_wait();

}

void gui_set_audio_stop()
{
}

void gui_set_audio_loop()
{
	gui_list_draw();
}

void gui_set_audio_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_set_audio_lift_cb(float val)
{
	gui_switch_task(GUI_SET_AUDIO);
	eeprom_busy_wait();
	int16_t tmp = val * 100;
	eeprom_update_word((uint16_t *)&config.audio_profile.lift, tmp);
	fc.audio_lift = tmp;
}

void gui_set_audio_sink_cb(float val)
{
	gui_switch_task(GUI_SET_AUDIO);
	eeprom_busy_wait();
	int16_t tmp = val * 100;
	eeprom_update_word((uint16_t *)&config.audio_profile.sink, tmp);
	fc.audio_sink = tmp;
}

void gui_set_audio_vol_cb(float val)
{
	gui_switch_task(GUI_SET_AUDIO);
	eeprom_busy_wait();
	uint8_t tmp = val;
	eeprom_update_byte(&config.audio_profile.volume, tmp);
	fc.audio_volume = tmp;
}

void gui_set_audio_action(uint8_t index)
{
	switch(index)
	{
	case(0):
		gui_value_conf_P(PSTR("Lift threshold"), GUI_VAL_NUMBER, PSTR("%0.1f m/s"), fc.audio_lift / 100.0, fc.audio_sink / 100.0, 2, 0.1, gui_set_audio_lift_cb);
		gui_switch_task(GUI_SET_VAL);
	break;

	case(1):
		gui_value_conf_P(PSTR("Sink threshold"), GUI_VAL_NUMBER, PSTR("%0.1f m/s"), fc.audio_sink / 100.0, -10, fc.audio_lift / 100.0, 0.1, gui_set_audio_sink_cb);
		gui_switch_task(GUI_SET_VAL);
	break;

	case(2):
		fc.audio_fluid = !fc.audio_fluid;
		eeprom_busy_wait();
		eeprom_update_byte(&config.audio_profile.fluid, fc.audio_fluid);
	break;

	case(3):
		gui_value_conf_P(PSTR("General volume"), GUI_VAL_VOLUME, PSTR("%0.0f %%"), fc.audio_volume, 0, 100, 5, gui_set_audio_vol_cb);
		gui_switch_task(GUI_SET_VAL);
	break;

	case(4):
		gui_switch_task(GUI_SETTINGS);
	break;
	}
}

void gui_set_audio_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
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
			sprintf_P(text, PSTR("Fluid audio"));
			if (fc.audio_fluid)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;

		break;
		case (3):
			sprintf_P(text, PSTR("General volume"));
			sprintf_P(sub_text, PSTR("%d %%"), fc.audio_volume);
			*flags |= GUI_LIST_SUB_TEXT;
		break;
		case (4):
			sprintf_P(text, PSTR("back"));
			*flags |= GUI_LIST_BACK;
		break;

	}
}

