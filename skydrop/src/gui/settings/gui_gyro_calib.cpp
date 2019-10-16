/*
 * gui_gyro_calib.cpp
 *
 *  Created on: Mar 13, 2017
 *      Author: fiala
 */

#include "gui_gyro_calib.h"
#include "../../fc/gyro.h"
#include "../gui_dialog.h"
#include "../../drivers/audio/sequencer.h"
#include "../../drivers/lcd_disp.h"


struct gyro_calib_t
{
	int16_t 		wait_cnt;
	vector_float_t 	bias;
};

#define WAIT_TIME 100
#define N_OF_SAMPLES 200

#define BAR_PADDING			4
#define BAR_WIDTH  			GUI_DISP_WIDTH - 2 * BAR_PADDING
#define BAR_X1_POS			BAR_PADDING
#define BAR_X2_POS			BAR_X1_POS + BAR_WIDTH
#define BAR_HEIGHT			6
#define BAR_MAX_LENGHT		(BAR_WIDTH - 2)

gyro_calib_t gui_gyro_calib;

void gui_gyro_calib_init()
{
	gui_gyro_calib.wait_cnt = 0 ;

}

void gui_gyro_calib_loop()
{
	uint8_t value;
	char tmp[20];

	gui_dialog_P(PSTR("Gyroscope"));
	disp.LoadFont(F_TEXT_M);
	uint8_t h_t = disp.GetTextHeight();

	if(gui_gyro_calib.wait_cnt < WAIT_TIME)
	{
		//print message
		strcpy_P(tmp, PSTR("Keep device calm!"));
		gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, GUI_DIALOG_TOP + 1 );
		gui_gyro_calib.wait_cnt += 1;
		return;
	}
	else if(gui_gyro_calib.wait_cnt <= (WAIT_TIME + N_OF_SAMPLES))
	{
		strcpy_P(tmp, PSTR("Calibrating ..."));
		gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, GUI_DIALOG_TOP + 1 );

		gui_gyro_calib.bias.x += (float) fc.gyro.raw.x;
		gui_gyro_calib.bias.y += (float) fc.gyro.raw.y;
		gui_gyro_calib.bias.z += (float) fc.gyro.raw.z;
		gui_gyro_calib.wait_cnt += 1;
	}
	else if(gui_gyro_calib.wait_cnt == (WAIT_TIME + N_OF_SAMPLES + 1))
	{
		gui_gyro_calib.bias.x /= (float) N_OF_SAMPLES;
		gui_gyro_calib.bias.y /= (float) N_OF_SAMPLES;
		gui_gyro_calib.bias.z /= (float) N_OF_SAMPLES;
		gui_gyro_calib.wait_cnt += 1;
	}

	//draw frame
	disp.DrawRectangle(BAR_X1_POS , GUI_DIALOG_TOP + 1 + h_t, BAR_X2_POS, GUI_DIALOG_TOP + 1 + h_t + BAR_HEIGHT, 1, 0);

	//draw bar
	value = uint8_t( BAR_MAX_LENGHT * ( float( gui_gyro_calib.wait_cnt - WAIT_TIME ) / N_OF_SAMPLES ) + 1.5);
	disp.DrawRectangle(BAR_X1_POS , GUI_DIALOG_TOP + 2 + h_t , BAR_X1_POS + value, GUI_DIALOG_TOP + h_t + BAR_HEIGHT, 1, 1);



	//menu
	disp.LoadFont(F_TEXT_S);
	gui_raligh_text_P(PSTR("back"), GUI_DISP_WIDTH - 2, GUI_DIALOG_BOTTOM - 4);

	if(gui_gyro_calib.wait_cnt == (WAIT_TIME + N_OF_SAMPLES + 2))
	{	//if complete

		//add menu option
		gui_caligh_text_P(PSTR("save"), GUI_DISP_WIDTH / 2, GUI_DIALOG_BOTTOM - 4);

		//print message
		disp.LoadFont(F_TEXT_M);
		strcpy_P(tmp, PSTR("Done!"));
		gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, GUI_DIALOG_TOP +1 );
	}
}


void gui_gyro_calib_irqh(uint8_t type, uint8_t * buff)
{
	if (*buff == BE_CLICK && type == B_RIGHT)
		gui_switch_task(GUI_SET_CALIB);

	if (*buff == BE_LONG && type == B_MIDDLE)
		gui_switch_task(GUI_SET_CALIB);

	if (*buff == BE_CLICK && type == B_MIDDLE && (gui_gyro_calib.wait_cnt == (WAIT_TIME + N_OF_SAMPLES + 2)))
	{	//if calibration is done

		//apply new settings
		fc.gyro.bias.x = gui_gyro_calib.bias.x;
		fc.gyro.bias.y = gui_gyro_calib.bias.y;
		fc.gyro.bias.z = gui_gyro_calib.bias.z;


		//save settings to eeprom
		gyro_save_calibration(&fc.gyro.bias);

		vector_float_t bias;
		gyro_load_calibration(&bias);

		//return to menu
		gui_switch_task(GUI_SET_CALIB);
	}
}





