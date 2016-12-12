/*
 * set_calib.cpp
 *
 *  Created on: Dec 5, 2016
 *      Author: fiala
 */

#include "set_calib.h"
#include "../gui_list.h"


void gui_set_calib_init()
{
	gui_list_set(gui_set_calib_item, gui_set_calib_action, 1, GUI_PAGES);
}

void gui_set_calib_stop()
{
}

void gui_set_calib_loop()
{
	gui_list_draw();
}

void gui_set_calib_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_set_calib_action(uint8_t index)
{
	switch(index)
	{
	case(0):
		gui_switch_task(GUI_SET_CALIB_ACC);
	break;

	case(1):
		gui_switch_task(GUI_SET_CALIB_MAG);
	break;

	}
}

void gui_set_calib_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			sprintf_P(text, PSTR("Accelerometer"));
			*flags |= GUI_LIST_FOLDER;
		break;

		case (1):
			sprintf_P(text, PSTR("Magnetometer"));
			*flags |= GUI_LIST_FOLDER;
		break;

	}
}





