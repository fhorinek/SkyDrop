#include "set_vario.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"

float set_vario_digital;
float set_vario_avg;

void gui_set_vario_init()
{
	gui_list_set(gui_set_vario_item, gui_set_vario_action, 5);

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

void gui_set_vario_qnh1_cb(float val)
{
	val *= 100.0;

	gui_switch_task(GUI_SET_VARIO);
	eeprom_busy_wait();
	eeprom_write_float(&config.altitude.QNH1, val);

	fc.QNH1 = val;
}

void gui_set_vario_qnh2_cb(float val)
{
	val *= 100.0;

	gui_switch_task(GUI_SET_VARIO);
	eeprom_busy_wait();
	eeprom_write_float(&config.altitude.QNH2, val);

	fc.QNH2 = val;
}

void gui_set_vario_action(uint8_t index)
{
	switch(index)
	{
	case(0):
		gui_value_conf("Digital vario int.", GUI_VAL_NUMBER, "%0.1f sec", set_vario_digital, 0, 30, 0.1, gui_set_vario_dig_int_cb);
		gui_switch_task(GUI_SET_VAL);
	break;

	case(1):
		gui_value_conf("Average vario int.", GUI_VAL_NUMBER, "%0.1f sec", set_vario_avg, 0, 90, 0.1, gui_set_vario_avg_int_cb);
		gui_switch_task(GUI_SET_VAL);
	break;

	case(2):
		gui_value_conf("QNH1", GUI_VAL_NUMBER, "%0.2f hPa", fc.QNH1 / 100.0, 0, 1500, 0.25, gui_set_vario_qnh1_cb);
		gui_switch_task(GUI_SET_VAL);
	break;

	case(3):
		gui_value_conf("QNH2", GUI_VAL_NUMBER, "%0.2f hPa", fc.QNH2 / 100.0, 0, 1500, 0.25, gui_set_vario_qnh2_cb);
		gui_switch_task(GUI_SET_VAL);
	break;


	case(4):
		gui_switch_task(GUI_SETTINGS);
	break;
	}
}

void gui_set_vario_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			sprintf_P(text, PSTR("Digital vario int."));
			sprintf_P(sub_text, PSTR("%0.1f s"), set_vario_digital);
			*flags |= GUI_LIST_SUB_TEXT;
		break;
		case (1):
			sprintf_P(text, PSTR("Average vario int."));
			sprintf_P(sub_text, PSTR("%0.1f s"), set_vario_avg);
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (2):
			sprintf_P(text, PSTR("QNH1 for Alt 1,2,3"));
			sprintf_P(sub_text, PSTR("%0.2f hPa"), fc.QNH1 / 100.0);
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (3):
			sprintf_P(text, PSTR("QNH2 for Alt 4"));
			sprintf_P(sub_text, PSTR("%0.2f hPa"), fc.QNH2 / 100.0);
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (4):
			sprintf_P(text, PSTR("back"));
			*flags |= GUI_LIST_BACK;
		break;

	}
}

