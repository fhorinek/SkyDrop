#include "set_system.h"

#include "../gui_list.h"
#include "../gui_value.h"
#include "../gui_dialog.h"
#include "../gui_storage.h"

#include "../../fc/conf.h"
#include "../../drivers/storage/storage.h"

void gui_set_system_init()
{
	gui_list_set(gui_set_system_item, gui_set_system_action, 3, GUI_SETTINGS);
}

void gui_set_system_stop() {}

void gui_set_system_loop()
{
	gui_list_draw();
}

void gui_set_system_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_set_system_action(uint8_t index)
{
	switch(index)
	{
		case(0):
			gui_switch_task(GUI_SET_DISPLAY);
		break;

		case(1):
			gui_switch_task(GUI_SET_AUDIO);
		break;

		case(2):
			gui_switch_task(GUI_SET_ADVANCED);
		break;
	}
}

void gui_set_system_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			strcpy_P(text, PSTR("Display"));
			*flags |= GUI_LIST_FOLDER;
		break;

		case (1):
			strcpy_P(text, PSTR("Audio"));
			*flags |= GUI_LIST_FOLDER;
		break;

		case (2):
			strcpy_P(text, PSTR("Advanced"));
		break;
	}
}

