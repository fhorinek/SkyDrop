#include "layouts.h"
#include "../gui_list.h"

void gui_layouts_init()
{
	gui_list_set(gui_layouts_item, gui_layouts_action, 2, GUI_PAGES);
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
		gui_switch_task(GUI_PAGES);
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
	}
}

