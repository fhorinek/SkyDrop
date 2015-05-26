#include "set_display.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"

void gui_set_display_init()
{
	gui_list_set(gui_set_display_item, gui_set_display_action, 4);
}

void gui_set_display_stop()
{
}

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
	eeprom_busy_wait();
	lcd_contrast = val;
	eeprom_update_byte(&config.gui.contrast, lcd_contrast);
	gui_set_contrast(lcd_contrast);
}

void gui_set_display_brightness_cb(float val)
{
	gui_switch_task(GUI_SET_DISPLAY);
	eeprom_busy_wait();
	lcd_brightness = val;
	eeprom_update_byte(&config.gui.brightness, lcd_brightness);
	gui_trigger_backlight();
}

void gui_set_display_brightness_timeout_cb(float val)
{
	gui_switch_task(GUI_SET_DISPLAY);
	eeprom_busy_wait();
	lcd_brightness_timeout = val;
	eeprom_update_byte(&config.gui.brightness_timeout, lcd_brightness_timeout);
}


void gui_set_display_action(uint8_t index)
{
	switch(index)
	{
	case(0):
		gui_value_conf("Contrast", GUI_VAL_NUMBER, "%1.0f %%", lcd_contrast, 0, 100, 1, gui_set_display_contrast_cb);
		gui_switch_task(GUI_SET_VAL);
	break;

	case(1):
		gui_value_conf("Backlight", GUI_VAL_NUMBER, "%1.0f %%", lcd_brightness, 0, 100, 20, gui_set_display_brightness_cb);
		gui_switch_task(GUI_SET_VAL);
	break;

	case(2):
		gui_value_conf("Backlight timeout", GUI_VAL_NUMBER, "%1.0f sec", lcd_brightness_timeout, 0, 30, 1, gui_set_display_brightness_timeout_cb);
		gui_switch_task(GUI_SET_VAL);
	break;

	case(3):
		gui_switch_task(GUI_SETTINGS);
	break;
	}
}

void gui_set_display_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			sprintf_P(text, PSTR("Contrast"));
			sprintf_P(sub_text, PSTR("%d %%"), lcd_contrast);
			*flags |= GUI_LIST_SUB_TEXT;
		break;
		case (1):
			sprintf_P(text, PSTR("Backlight"));
			sprintf_P(sub_text, PSTR("%d %%"), lcd_brightness);
			*flags |= GUI_LIST_SUB_TEXT;
		break;
		case (2):
			sprintf_P(text, PSTR("Backlight timeout"));
			sprintf_P(sub_text, PSTR("%d sec"), lcd_brightness_timeout);
			*flags |= GUI_LIST_SUB_TEXT;
		break;
		case (3):
			sprintf_P(text, PSTR("back"));
			*flags |= GUI_LIST_BACK;
		break;

	}
}

