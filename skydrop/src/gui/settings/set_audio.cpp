#include "set_audio.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"
#include "../../drivers/audio/audio.h"


void gui_set_audio_init()
{
	gui_list_set(gui_set_audio_item, gui_set_audio_action, 4, GUI_SET_SYSTEM);
}

void gui_set_audio_stop() {}

void gui_set_audio_loop()
{
	gui_list_draw();
}

void gui_set_audio_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_set_audio_vario_vol_cb(float val)
{
	gui_switch_task(GUI_SET_AUDIO);
	eeprom_busy_wait();
	uint8_t tmp = val;
	eeprom_update_byte(&config_ee.gui.vario_volume, tmp);
	config.gui.vario_volume = tmp;
}

void gui_set_audio_alert_vol_cb(float val)
{
	gui_switch_task(GUI_SET_AUDIO);
	eeprom_busy_wait();
	uint8_t tmp = val;
	eeprom_update_byte(&config_ee.gui.alert_volume, tmp);
	config.gui.alert_volume = tmp;
}

void gui_set_audio_menu_vol_cb(float val)
{
	gui_switch_task(GUI_SET_AUDIO);
	eeprom_busy_wait();
	uint8_t tmp = val;
	eeprom_update_byte(&config_ee.gui.menu_volume, tmp);
	config.gui.menu_volume = tmp;
}

void gui_set_audio_action(uint8_t index)
{
	switch(index)
	{
		case(0):
			gui_value_conf_P(PSTR("Vario volume"), GUI_VAL_VOLUME, PSTR(""), config.gui.vario_volume, 0, 100, 25, gui_set_audio_vario_vol_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(1):
			gui_value_conf_P(PSTR("Alert volume"), GUI_VAL_VOLUME, PSTR(""), config.gui.alert_volume, 0, 100, 25, gui_set_audio_alert_vol_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(2):
			gui_value_conf_P(PSTR("Menu volume"), GUI_VAL_VOLUME, PSTR(""), config.gui.menu_volume, 0, 100, 25, gui_set_audio_menu_vol_cb);
			gui_switch_task(GUI_SET_VAL);
		break;

		case(3):
			gui_switch_task(GUI_SET_AUDIO_MENU);
		break;
	}
}

void gui_set_audio_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			sprintf_P(text, PSTR("Vario volume"));
			sprintf_P(sub_text, PSTR("%d %%"), config.gui.vario_volume);
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (1):
			sprintf_P(text, PSTR("Alert volume"));
			sprintf_P(sub_text, PSTR("%d %%"), config.gui.alert_volume);
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (2):
			sprintf_P(text, PSTR("Menu volume"));
			sprintf_P(sub_text, PSTR("%d %%"), config.gui.menu_volume);
			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (3):
			sprintf_P(text, PSTR("Menu sounds"));
			*flags |= GUI_LIST_FOLDER;
		break;
	}
}

