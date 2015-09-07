#include "layouts.h"

#include "../gui_list.h"
#include "../gui_value.h"

void gui_layouts_init()
{
	gui_list_set(gui_layouts_item, gui_layouts_action, 3, GUI_PAGES);
}

void gui_layouts_stop()
{
}

void gui_layouts_loop()
{
	gui_list_draw();
}

void gui_layouts_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_set_layouts_pages_cb(float val)
{
	uint8_t tmp = val;

	config.gui.number_of_pages = tmp;
	eeprom_busy_wait();
	eeprom_update_byte(&config_ee.gui.number_of_pages, tmp);

	if (active_page >= tmp);
		active_page = 0;

	gui_switch_task(GUI_LAYOUTS);
}


void gui_layouts_action(uint8_t index)
{
	switch(index)
	{
	case(0):
		gui_switch_task(GUI_SET_WIDGETS);
	break;

	case(1):
		gui_switch_task(GUI_SET_LAYOUT);
	break;

	case(2):
		gui_value_conf_P(PSTR("Pages count"), GUI_VAL_NUMBER, PSTR("%1.0f"), config.gui.number_of_pages, 1, MAX_NUMBER_OF_PAGES, 1, gui_set_layouts_pages_cb);
		gui_switch_task(GUI_SET_VAL);
	break;
	}
}

void gui_layouts_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			sprintf_P(text, PSTR("Change widgets"));
			*flags |= GUI_LIST_FOLDER;
		break;
		case (1):
			sprintf_P(text, PSTR("Change layout"));
			*flags |= GUI_LIST_FOLDER;
		break;
		case (2):
			sprintf_P(text, PSTR("Pages count"));
			sprintf_P(sub_text, PSTR("%d"), config.gui.number_of_pages);
			*flags |= GUI_LIST_SUB_TEXT;
		break;
	}
}

