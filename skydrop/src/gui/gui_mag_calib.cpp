/*
 * gui_mag_calib.cpp
 *
 *  Created on: Dec 5, 2016
 *      Author: fiala
 */

#include "gui_mag_calib.h"
#include "gui_dialog.h"



void gui_mag_calib_init()
{

}

void gui_mag_calib_stop() {}

void gui_mag_calib_loop()
{
	gui_dialog_P(PSTR("Magnetometer"));

	disp.LoadFont(F_TEXT_S);
	gui_caligh_text_P(PSTR("back"), GUI_DISP_WIDTH / 2, GUI_DIALOG_BOTTOM - 4);
}


void gui_mag_calib_irqh(uint8_t type, uint8_t * buff)
{
	if (*buff == BE_CLICK && type == B_MIDDLE)
		gui_switch_task(GUI_SET_CALIB);

	if (*buff == BE_LONG && type == B_MIDDLE)
		gui_switch_task(GUI_SET_CALIB);

	if (*buff == BE_CLICK && type == B_LEFT )
	{
		gui_switch_task(GUI_SET_CALIB);
	}
}




