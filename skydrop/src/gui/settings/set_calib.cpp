/*
 * set_calib.cpp
 *
 *  Created on: Dec 5, 2016
 *      Author: fiala
 */

#include "set_calib.h"
#include "../gui_list.h"
#include "../gui_dialog.h"
#include "../../drivers/battery.h"

void gui_set_calib_init()
{
	gui_list_set(gui_set_calib_item, gui_set_calib_action, 5, GUI_SET_ADVANCED);
}

void gui_set_calib_stop() {}

void gui_set_calib_loop()
{
	gui_list_draw();
}

void gui_set_calib_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_set_calib_action_acc(uint8_t ret)
{
	gui_switch_task(GUI_SET_CALIB_ACC);
}

void gui_set_calib_action_mag(uint8_t ret)
{
	gui_switch_task(GUI_SET_CALIB_MAG);
}

void gui_set_calib_action_gyro(uint8_t ret)
{
	gui_switch_task(GUI_SET_CALIB_GYRO);
}


void gui_set_calib_action(uint8_t index)
{
	switch(index)
	{
	case(0):
		gui_dialog_set_P(PSTR("Place device on"), PSTR("each edge and\nwait for confirm.\nRefer to manual."), GUI_STYLE_OK | GUI_STYLE_NO_TITLE, gui_set_calib_action_acc);
		gui_switch_task(GUI_DIALOG);
	break;

	case(1):
		gui_dialog_set_P(PSTR("Rotate device"), PSTR("around each axis.\nRefer to manual."), GUI_STYLE_OK | GUI_STYLE_NO_TITLE, gui_set_calib_action_mag);
		gui_switch_task(GUI_DIALOG);
	break;

	case(2):
		gui_dialog_set_P(PSTR("Leave device"), PSTR("at rest, wait\nuntil calibration\nis done."), GUI_STYLE_OK | GUI_STYLE_NO_TITLE, gui_set_calib_action_gyro);
		gui_switch_task(GUI_DIALOG);
	break;

	case(3):
		gui_switch_task(GUI_SET_COMPASS);
	break;

	case(4):
		if (battery_calibrating_state == BATTERY_CAL_NONE)
			battery_calibrating_state = BATTERY_CAL_START;
		else
			battery_calibrating_state = BATTERY_CAL_STOP;
	break;
	}
}

void gui_set_calib_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			strcpy_P(text, PSTR("Accelerometer"));
			*flags |= GUI_LIST_FOLDER;
		break;

		case (1):
			strcpy_P(text, PSTR("Magnetometer"));
			*flags |= GUI_LIST_FOLDER;
		break;

		case (2):
			strcpy_P(text, PSTR("Gyroscope"));
			*flags |= GUI_LIST_FOLDER;
		break;

		case (3):
			strcpy_P(text, PSTR("Compass"));
			*flags |= GUI_LIST_FOLDER;
		break;

		case (4):
			strcpy_P(text, PSTR("Battery"));
			if (battery_calibrating_state != BATTERY_CAL_NONE)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;

		break;
	}
}





