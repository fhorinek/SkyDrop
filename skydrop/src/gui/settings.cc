#include "settings.h"
#include "gui_list.h"

void gui_settings_init()
{
	gui_list_set(gui_settings_item, 40);
}

void gui_settings_stop()
{
}

void gui_settings_loop()
{
	DEBUG("LIST DRAW\n");
	gui_list_draw();
}

void gui_settings_irqh(uint8_t type, uint8_t * buff)
{
	switch (type)
	{
	case(TASK_IRQ_BUTTON_L):
		if (*buff == BE_CLICK)
			gui_list_moveup();
		if (*buff == BE_DBL_CLICK)
			gui_list_moveup();
	break;

	case(TASK_IRQ_BUTTON_R):
		if (*buff == BE_CLICK)
			gui_list_movedown();
		if (*buff == BE_DBL_CLICK)
			gui_list_movedown();
	break;


	}
}


void gui_settings_item(uint8_t index, char * text, uint8_t * disabled)
{
	*disabled = false;
	sprintf(text, "item %d", index);
	DEBUG("gen %d, %s\n", index, text);
}

