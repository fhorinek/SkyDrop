#include "set_display.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"

void gui_set_display_init()
{
	gui_list_set(gui_set_display_item, gui_set_display_action, 8, GUI_SET_SYSTEM);
}

void gui_set_display_contrast_cb(float val)
{
	gui_switch_task(GUI_SET_DISPLAY);
	
	config.gui.contrast = val;
	ee_update_byte(&config_ee.gui.contrast, config.gui.contrast);
	gui_change_disp_cfg();
}

void gui_set_display_brightness_cb(float val)
{
	gui_switch_task(GUI_SET_DISPLAY);
	
	config.gui.brightness = val;
	ee_update_byte(&config_ee.gui.brightness, config.gui.brightness);
	gui_trigger_backlight();
}

void gui_set_display_brightness_timeout_cb(float val)
{
	gui_switch_task(GUI_SET_DISPLAY);
	
	config.gui.brightness_timeout = val;
	ee_update_byte(&config_ee.gui.brightness_timeout, config.gui.brightness_timeout);
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
		gui_value_conf_P(PSTR("Backlight"), GUI_VAL_BRIGTHNES, PSTR(""), config.gui.brightness, 0, 100, 20, gui_set_display_brightness_cb);
		gui_switch_task(GUI_SET_VAL);
	break;

	case(2):
		gui_value_conf_P(PSTR("Backlight timeout"), GUI_VAL_NUMBER, PSTR("%1.0f sec"), config.gui.brightness_timeout, 0, 30, 1, gui_set_display_brightness_timeout_cb);
		gui_switch_task(GUI_SET_VAL);
	break;

	case(3):
		config.gui.disp_flags = config.gui.disp_flags ^ CFG_DISP_INVERT;
		
		ee_update_byte(&config_ee.gui.disp_flags, config.gui.disp_flags);
		gui_change_disp_cfg();
	break;

	case(4):
		config.gui.disp_flags = config.gui.disp_flags ^ CFG_DISP_FLIP;
		
		ee_update_byte(&config_ee.gui.disp_flags, config.gui.disp_flags);
		disp.SetFlip(config.gui.disp_flags & CFG_DISP_FLIP);
	break;

	case(5):
		config.gui.disp_flags = config.gui.disp_flags ^ CFG_DISP_ANIM;
		
		ee_update_byte(&config_ee.gui.disp_flags, config.gui.disp_flags);
	break;

	case(6):
		config.gui.disp_flags = config.gui.disp_flags ^ CFG_DISP_CYCLE;
		
		ee_update_byte(&config_ee.gui.disp_flags, config.gui.disp_flags);
	break;

	case(7):
		config.gui.disp_flags = config.gui.disp_flags ^ CFG_DISP_FAHRENHEIT;
		
		ee_update_byte(&config_ee.gui.disp_flags, config.gui.disp_flags);
	break;
}
}

void gui_set_display_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			strcpy_P(text, PSTR("Contrast"));
//			sprintf_P(sub_text, PSTR("%d %%"), lcd_contrast);
//			*flags = GUI_LIST_SUB_TEXT;
		break;
		case (1):
			strcpy_P(text, PSTR("Backlight"));
//			sprintf_P(sub_text, PSTR("%d %%"), lcd_brightness);
//			*flags = GUI_LIST_SUB_TEXT;
		break;
		case (2):
			strcpy_P(text, PSTR("Backlight timeout"));
			sprintf_P(sub_text, PSTR("%d sec"), config.gui.brightness_timeout);
			*flags = GUI_LIST_SUB_TEXT;
		break;
		case (3):
			strcpy_P(text, PSTR("Invert display"));
			if (config.gui.disp_flags & CFG_DISP_INVERT)
				*flags = GUI_LIST_CHECK_ON;
			else
				*flags = GUI_LIST_CHECK_OFF;
		break;
		case (4):
			strcpy_P(text, PSTR("Flip orientation"));
			if (config.gui.disp_flags & CFG_DISP_FLIP)
				*flags = GUI_LIST_CHECK_ON;
			else
				*flags = GUI_LIST_CHECK_OFF;
		break;
		case (5):
			strcpy_P(text, PSTR("Animation"));
			if (config.gui.disp_flags & CFG_DISP_ANIM)
				*flags = GUI_LIST_CHECK_ON;
			else
				*flags = GUI_LIST_CHECK_OFF;
		break;
		case (6):
			strcpy_P(text, PSTR("Cycle"));
			if (config.gui.disp_flags & CFG_DISP_CYCLE)
				*flags = GUI_LIST_CHECK_ON;
			else
				*flags = GUI_LIST_CHECK_OFF;
		break;
		case (7):
			strcpy_P(text, PSTR("Temperature"));
			*flags = GUI_LIST_SUB_TEXT;
			if (config.gui.disp_flags & CFG_DISP_FAHRENHEIT)
				strcpy_P(sub_text, PSTR("Fahrenheit"));
			else
				strcpy_P(sub_text, PSTR("Celsius"));
		break;
	}
}

