#include "set_display.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"

extern uint8_t lcd_contrast_min;
extern uint8_t lcd_contrast_max;

void gui_set_display_init()
{
	gui_list_set(gui_set_display_item, gui_set_display_action, 5, GUI_SET_SYSTEM);
}

void gui_set_display_stop() {}

void gui_set_display_loop()
{
	gui_list_draw();
}

void gui_set_display_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_set_display_contrast_cb(float val)
{
	gui_switch_task(GUI_SET_DISPLAY);
	config.gui.contrast = val;
	gui_change_disp_cfg();
}

uint8_t tmp_contrast;

void gui_set_display_contrast_min_cb(float val)
{
	gui_switch_task(GUI_SET_DISPLAY);
	lcd_contrast_min = val;

	eeprom_busy_wait();
	eeprom_update_byte(&config_ro.lcd_contrast_min, lcd_contrast_min);

	config.gui.contrast = tmp_contrast;
	gui_change_disp_cfg();
}

void gui_set_display_contrast_max_cb(float val)
{
	gui_switch_task(GUI_SET_DISPLAY);
	lcd_contrast_max = val;

	eeprom_busy_wait();
	eeprom_update_byte(&config_ro.lcd_contrast_max, lcd_contrast_max);

	config.gui.contrast = tmp_contrast;
	gui_change_disp_cfg();
}

void gui_set_display_action(uint8_t index)
{
	switch(index)
	{
	case(0):
		gui_value_conf_P(PSTR("Contrast"), GUI_VAL_CONTRAST, PSTR(""), config.gui.contrast, 0, GUI_CONTRAST_STEPS, 1, gui_set_display_contrast_cb);
		gui_switch_task(GUI_SET_VAL);
	break;

	case(1):
		tmp_contrast = config.gui.contrast;
		config.gui.contrast = 0;
		gui_value_conf_P(PSTR("Contrast min"), GUI_VAL_CONTRAST_MIN, PSTR(""), lcd_contrast_min, 0, 127, 1, gui_set_display_contrast_min_cb);
		gui_switch_task(GUI_SET_VAL);
	break;

	case(2):
		tmp_contrast = config.gui.contrast;
		config.gui.contrast = GUI_CONTRAST_STEPS;
		gui_value_conf_P(PSTR("Contrast max"), GUI_VAL_CONTRAST_MAX, PSTR(""), lcd_contrast_max, 0, 127, 1, gui_set_display_contrast_max_cb);
		gui_switch_task(GUI_SET_VAL);
	break;

	case(3):
		config.gui.disp_flags = config.gui.disp_flags ^ CFG_DISP_INVERT;
		gui_change_disp_cfg();
	break;

	case(4):
		config.gui.disp_flags = config.gui.disp_flags ^ CFG_DISP_FLIP;
		disp.SetFlip(config.gui.disp_flags & CFG_DISP_FLIP);
	break;
	}
}

void gui_set_display_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			strcpy_P(text, PSTR("Contrast"));
		break;

		case (1):
			strcpy_P(text, PSTR("Contrast min"));
			*flags |= GUI_LIST_SUB_TEXT;
			sprintf_P(sub_text, PSTR("%u"), lcd_contrast_min);
		break;

		case (2):
			strcpy_P(text, PSTR("Contrast max"));
			*flags |= GUI_LIST_SUB_TEXT;
			sprintf_P(sub_text, PSTR("%u"), lcd_contrast_max);
		break;

		case (3):
			strcpy_P(text, PSTR("Invert display"));
			if (config.gui.disp_flags & CFG_DISP_INVERT)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;
		break;
		case (4):
			strcpy_P(text, PSTR("Flip orientation"));
			if (config.gui.disp_flags & CFG_DISP_FLIP)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;
		break;
	}
}

