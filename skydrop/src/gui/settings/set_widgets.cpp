#include "set_widgets.h"
#include "../widgets/widgets.h"
#include "../gui_list.h"

#define SET_WIDGETS_CHOOSE	0
#define SET_WIDGETS_LIST	1

uint8_t set_widget_mode = SET_WIDGETS_CHOOSE;
uint8_t set_widget_index;
uint8_t set_widget_cnt;

void gui_set_widgets_init()
{
	set_widget_mode = SET_WIDGETS_CHOOSE;
	set_widget_cnt = layout_get_number_of_widgets(config.gui.pages[active_page].type);
	set_widget_index = 0;
	gui_list_set(gui_set_widgets_item, gui_set_widgets_action, NUMBER_OF_SORTED_WIDGETS, GUI_LAYOUTS);
}

void gui_set_widgets_stop() {}

void gui_set_widgets_loop()
{
	switch (set_widget_mode)
	{
	case(SET_WIDGETS_CHOOSE):
		widgets_draw(active_page);

		if (GUI_BLINK_TGL(1000))
		{
			//Highlight selected widget
			uint8_t x, y, w, h;
			layout_get_widget_rect(config.gui.pages[active_page].type, set_widget_index, &x, &y, &w, &h);

			disp.Invert(x, y, x + w - 1, y + h - 1);
			disp.InvertPixel(x, y);
			disp.InvertPixel(x + w - 1, y);
			disp.InvertPixel(x, y + h - 1);
			disp.InvertPixel(x + w - 1, y + h - 1);
		}
	break;

	case(SET_WIDGETS_LIST):
		gui_list_draw();
	break;

	}
}

void gui_set_widgets_choose_irqh(uint8_t type, uint8_t * buff)
{

	switch(type)
	{
	case(TASK_IRQ_BUTTON_L):
		if (*buff == BE_CLICK)
		{
			if (set_widget_index > 0)
				set_widget_index -= 1;
			else
				set_widget_index = set_widget_cnt - 1;
		}
	break;

	case(TASK_IRQ_BUTTON_R):
		if (*buff == BE_CLICK)
		{
			set_widget_index = (set_widget_index + 1) % set_widget_cnt;
		}
	break;

	case(TASK_IRQ_BUTTON_M):
		if (*buff == BE_CLICK)
		{
			uint8_t sorted_index = 0;
			//get index in sorted list
			for (uint8_t i = 0; i < NUMBER_OF_SORTED_WIDGETS; i++)
				if (widget_sorted_get_index(i) == config.gui.pages[active_page].widgets[set_widget_index])
				{
					sorted_index = i;
					break;
				}

			gui_list_set_index(GUI_SET_WIDGETS, sorted_index);
			set_widget_mode = SET_WIDGETS_LIST;
		}
		if (*buff == BE_LONG)
		{
			gui_switch_task(GUI_LAYOUTS);
		}
	break;
	}
}

void gui_set_widgets_irqh(uint8_t type, uint8_t * buff)
{
	switch(set_widget_mode)
	{
	case(SET_WIDGETS_CHOOSE):
		gui_set_widgets_choose_irqh(type, buff);
	break;

	case(SET_WIDGETS_LIST):
		gui_list_irqh(type, buff);
	break;
	}
}

void gui_set_widgets_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	strcpy_P(text, widget_array[widget_sorted_get_index(index)].label);
}

void gui_set_widgets_action(uint8_t index)
{
	config.gui.pages[active_page].widgets[set_widget_index] = widget_sorted_get_index(index);
	eeprom_busy_wait();
	eeprom_update_byte(&config_ee.gui.pages[active_page].widgets[set_widget_index], config.gui.pages[active_page].widgets[set_widget_index]);
	set_widget_mode = SET_WIDGETS_CHOOSE;
}
