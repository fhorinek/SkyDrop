#include "set_altimeters.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"

#include "set_altimeter.h"

void gui_set_altimeters_init()
{
	gui_list_set(gui_set_altimeters_item, gui_set_altimeters_action, 7, GUI_SETTINGS);
}

void gui_set_altimeters_stop() {}

void gui_set_altimeters_loop()
{
	gui_list_draw();
}

void gui_set_altimeters_qnh1_cb(float val)
{
	val *= 100.0;

	gui_switch_task(GUI_SET_ALTIMETERS);
	eeprom_busy_wait();
	eeprom_write_float(&config_ee.altitude.QNH1, val);

	config.altitude.QNH1 = val;

	float new_alt = fc_press_to_alt(fc.vario.pressure, config.altitude.QNH1);
	fc_manual_alt0_change(new_alt);
}

void gui_set_altimeters_qnh2_cb(float val)
{
	val *= 100.0;

	gui_switch_task(GUI_SET_ALTIMETERS);
	eeprom_busy_wait();
	eeprom_write_float(&config_ee.altitude.QNH2, val);

	config.altitude.QNH2 = val;
}

void gui_set_altimeters_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_set_altimeters_action(uint8_t index)
{
	switch(index)
	{
		case (0): case (1): case (2): case (3): case (4):
			gui_set_altimeter_index(index);
			gui_switch_task(GUI_SET_ALTIMETER);
		break;

		case(5):
			gui_value_conf_P(PSTR("QNH1"), GUI_VAL_NUMBER, PSTR("%0.2f hPa"), config.altitude.QNH1 / 100.0, 0, 1500, 0.25, gui_set_altimeters_qnh1_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(6):
			gui_value_conf_P(PSTR("QNH2"), GUI_VAL_NUMBER, PSTR("%0.2f hPa"), config.altitude.QNH2 / 100.0, 0, 1500, 0.25, gui_set_altimeters_qnh2_cb);
			gui_switch_task(GUI_SET_VAL);
		break;
	}
}

void gui_set_altimeters_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0): case (1): case (2): case (3): case (4):
			sprintf_P(text, PSTR("Altimeter %d"), index + 1);
			*flags |= GUI_LIST_FOLDER;
		break;

		case (5):
			sprintf_P(text, PSTR("QNH1 for Alt 1"));
			sprintf_P(sub_text, PSTR("%0.2f hPa"), config.altitude.QNH1 / 100.0);
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (6):
			sprintf_P(text, PSTR("QNH2"));
			sprintf_P(sub_text, PSTR("%0.2f hPa"), config.altitude.QNH2 / 100.0);
			*flags |= GUI_LIST_SUB_TEXT;
		break;
	}
}

