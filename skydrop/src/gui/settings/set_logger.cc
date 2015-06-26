#include "set_logger.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"

void gui_set_logger_init()
{
	gui_list_set(gui_set_logger_item, gui_set_logger_action, 1, GUI_SETTINGS);
}

void gui_set_logger_stop() {}

void gui_set_logger_loop()
{
	gui_list_draw();
}

void gui_set_logger_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_set_logger_action(uint8_t index)
{
	switch(index)
	{
		case(0):
			gui_switch_task(GUI_SET_AUTOSTART);
		break;
	}
}

void gui_set_logger_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			sprintf_P(text, PSTR("Autostart"));
			*flags |= GUI_LIST_FOLDER;
		break;
	}
}

