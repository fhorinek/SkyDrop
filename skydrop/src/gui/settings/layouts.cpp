#include "layouts.h"

#include "../gui_list.h"
#include "../gui_value.h"

void gui_layouts_init()
{
	gui_list_set(gui_layouts_item, gui_layouts_action, 6, GUI_PAGES);
}


void gui_set_layouts_pages_cb(float val)
{
	uint8_t tmp = val;

	config.gui.number_of_pages = tmp;
	
	ee_update_byte(&config_ee.gui.number_of_pages, tmp);

	if (active_page >= tmp)
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

	case(3):
		config.gui.silent ^= (1 << active_page);
		
		ee_update_byte(&config_ee.gui.silent, config.gui.silent);
	break;

	case(4):
		config.gui.hide_label ^= (1 << active_page);
		
		ee_update_byte(&config_ee.gui.hide_label, config.gui.hide_label);
	break;

    case(5):
        gui_switch_task(GUI_SET_AUTOSET);
    break;
	}
}

void gui_layouts_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			strcpy_P(text, PSTR("Change widgets"));
		break;

		case (1):
			strcpy_P(text, PSTR("Change layout"));
		break;

		case (2):
			strcpy_P(text, PSTR("Pages count"));
			sprintf_P(sub_text, PSTR("%d"), config.gui.number_of_pages);
			*flags =  GUI_LIST_SUB_TEXT;
		break;

		case (3):
			strcpy_P(text, PSTR("Silent page"));
			if (config.gui.silent & (1 << active_page))
				*flags =  GUI_LIST_CHECK_ON;
			else
				*flags =  GUI_LIST_CHECK_OFF;
		break;

		case (4):
			strcpy_P(text, PSTR("Hide labels"));
			if (config.gui.hide_label & (1 << active_page))
				*flags =  GUI_LIST_CHECK_ON;
			else
				*flags =  GUI_LIST_CHECK_OFF;
		break;

        case (5):
            strcpy_P(text, PSTR("Autoset"));
        break;
	}
}

