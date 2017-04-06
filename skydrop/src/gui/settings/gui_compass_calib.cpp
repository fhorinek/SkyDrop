/*
 * gui_compass_calib.cpp
 *
 *  Created on: Mar 29, 2017
 *      Author: fiala
 */


#include "gui_compass_calib.h"
#include "../../fc/acc.h"
#include "../gui_dialog.h"
#include "../../drivers/lcd_disp.h"

void gui_compass_calib_init()
{

}

void gui_compass_calib_stop() {}


void gui_compass_calib_loop()
{

	disp.LoadFont(F_TEXT_M);
	uint8_t h_t = disp.GetTextHeight();

	gui_dialog_P(PSTR("Compass"));

	char tmp[16];

}


void gui_compass_calib_irqh(uint8_t type, uint8_t * buff)
{
	if (*buff == BE_CLICK && type == B_MIDDLE)
		gui_switch_task(GUI_SET_CALIB);

	if (*buff == BE_LONG && type == B_MIDDLE)
		gui_switch_task(GUI_SET_CALIB);

	if (*buff == BE_CLICK && type == B_LEFT )
	{	//if calibration is done

		//apply new settings
		//memcpy((void *)&fc.acc.sens, &gui_acc_calib.sens, sizeof(vector_float_t));
		//memcpy((void *)&fc.acc.bias, &gui_acc_calib.bias, sizeof(vector_float_t));

		//save settings to eeprom
		//acc_save_calibration(gui_acc_calib.sens, gui_acc_calib.bias);

		//return to menu
		gui_switch_task(GUI_SET_CALIB);
	}
}





