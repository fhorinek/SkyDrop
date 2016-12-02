/*
 * gui_accel_calib.cpp
 *
 *  Created on: Nov 21, 2016
 *      Author: Fiala
 */

#include "gui_accel_calib.h"
#include "../fc/acc.h"
#include "gui_dialog.h"
#include "../drivers/audio/sequencer.h"
#include "../drivers/lcd_disp.h"


#define LPF_beta 			0.05		//low pass filter setting
#define treshold_percent	90.0
#define treshold_lo  		0.75 * default1G
#define treshold_hi  		1.10 * default1G
#define default1G			1300
#define avg_weight_calc 	10		//averaging, weight of old samples - getting values for calculation of bias and sensitivity
#define avg_weight_flow 	5		//averaging, weight of old samples - decision which axis will be set
#define calib_samples		100		//n of samples collected for calculating accel calibration
#define max_err				40		//max variance between samples

#define audio_acc_calib_freq	300

accel_calib_t gui_acc_calib;

MK_SEQ(snd_confirm, ARR({750, 0, 750, 0}), ARR({150, 150, 150, 150}));
MK_SEQ(snd_complete, ARR({750, 0, 750, 0, 900, 0}), ARR({150, 150, 150, 150, 250, 150}));

void gui_acc_snd_confirm()
{
	seq_start(&snd_confirm, config.gui.alert_volume);
}
void gui_acc_snd_complete()
{
	seq_start(&snd_complete, config.gui.alert_volume);
}


void gui_accelerometer_calib_init()
{
	gui_acc_calib.valid.Xpos = false;
	gui_acc_calib.valid.Ypos = false;
	gui_acc_calib.valid.Zpos = false;
	gui_acc_calib.valid.Xneg = false;
	gui_acc_calib.valid.Yneg = false;
	gui_acc_calib.valid.Zneg = false;

	gui_acc_calib.calc.counter = 0;
	gui_acc_calib.new_calib = false;

	gui_acc_calib.sens.x = 0;
	gui_acc_calib.sens.y = 0;
	gui_acc_calib.sens.z = 0;
	gui_acc_calib.bias.x = 0;
	gui_acc_calib.bias.y = 0;
	gui_acc_calib.bias.z = 0;

	gui_acc_calib.filtered_total_new = 1;
	gui_acc_calib.filtered_total_old = 1;
}


void gui_accelerometer_calib_stop() {}

int16_t acc_calib_get_value(int16_t value)
{
	uint16_t err = abs(value - gui_acc_calib.calc.old_value);
	gui_acc_calib.calc.old_value = value;

	if(err <= max_err)
	{
		if(gui_acc_calib.calc.counter == 0)
			gui_acc_calib.calc.avg_value = value;
		else
			gui_acc_calib.calc.avg_value = ((gui_acc_calib.calc.avg_value * avg_weight_calc - gui_acc_calib.calc.avg_value + value) / avg_weight_calc);
		gui_acc_calib.calc.counter += 1;
	}
	else
	{
		gui_acc_calib.calc.counter = 0;
		gui_acc_calib.is_sampling = false;
		return 0;
	}
	if(gui_acc_calib.calc.counter < calib_samples)
	{
		if(gui_acc_calib.calc.counter * 4 > calib_samples)
			gui_acc_calib.is_sampling = true;
		return 0;
	}
	else
	{
		//DEBUG("exit 3: %d\n", gui_acc_calib.calc.avg_value);
		gui_acc_calib.calc.counter = 0;
		gui_acc_calib.is_sampling = false;
		return gui_acc_calib.calc.avg_value;
	}
}


void gui_accelerometer_calib_loop()
{

	gui_acc_calib.is_sampling = false;
	gui_acc_calib.sampling.Xpos = false;
	gui_acc_calib.sampling.Xneg = false;
	gui_acc_calib.sampling.Ypos = false;
	gui_acc_calib.sampling.Yneg = false;
	gui_acc_calib.sampling.Zpos = false;
	gui_acc_calib.sampling.Zneg = false;

	gui_acc_calib.filtered_raw.x = ((gui_acc_calib.filtered_raw.x * avg_weight_flow - gui_acc_calib.filtered_raw.x + fc.acc_data.x) / avg_weight_flow);
	gui_acc_calib.filtered_raw.y = ((gui_acc_calib.filtered_raw.y * avg_weight_flow - gui_acc_calib.filtered_raw.y + fc.acc_data.y) / avg_weight_flow);
	gui_acc_calib.filtered_raw.z = ((gui_acc_calib.filtered_raw.z * avg_weight_flow - gui_acc_calib.filtered_raw.z + fc.acc_data.z) / avg_weight_flow);

	//DEBUG("%d    %d    %d    %d\n", fc.acc_data.x, fc.acc_data.y, fc.acc_data.z, accel_calib.filtered.x);

	if(abs(gui_acc_calib.filtered_raw.x) > (treshold_lo) )
	//if(abs(gui_acc_calib.filtered_raw.x) > (fc.acc_tot * treshold_percent / 100.0) )
	{
		if(fc.acc_data.x > 0 and gui_acc_calib.valid.Xpos == false)
		{
			gui_acc_calib.calc.pos_val.x = acc_calib_get_value(fc.acc_data.x);
			if(gui_acc_calib.is_sampling)
				gui_acc_calib.sampling.Xpos = true;
			if(gui_acc_calib.calc.pos_val.x)
			{
				gui_acc_calib.valid.Xpos = true;
				gui_acc_snd_confirm();
			}
		}
		if(fc.acc_data.x < 0 and gui_acc_calib.valid.Xneg == false)
		{
			gui_acc_calib.calc.neg_val.x = acc_calib_get_value(fc.acc_data.x);
			if(gui_acc_calib.is_sampling)
				gui_acc_calib.sampling.Xneg = true;
			if(gui_acc_calib.calc.neg_val.x)
			{
				gui_acc_calib.valid.Xneg = true;
				gui_acc_snd_confirm();
			}
		}
	}

	if(abs(gui_acc_calib.filtered_raw.y) > (treshold_lo) )
	//if(abs(gui_acc_calib.filtered_raw.y) > (fc.acc_tot * treshold_percent / 100.0) )
	{
		if(fc.acc_data.y > 0 and gui_acc_calib.valid.Ypos == false)
		{
			gui_acc_calib.calc.pos_val.y = acc_calib_get_value(fc.acc_data.y);
			if(gui_acc_calib.is_sampling)
				gui_acc_calib.sampling.Ypos = true;
			if(gui_acc_calib.calc.pos_val.y)
			{
				gui_acc_calib.valid.Ypos = true;
				gui_acc_snd_confirm();
			}
		}
		if(fc.acc_data.y < 0 and gui_acc_calib.valid.Yneg == false)
		{
			gui_acc_calib.calc.neg_val.y = acc_calib_get_value(fc.acc_data.y);
			if(gui_acc_calib.is_sampling)
				gui_acc_calib.sampling.Yneg = true;
			if(gui_acc_calib.calc.neg_val.y)
			{
				gui_acc_calib.valid.Yneg = true;
				gui_acc_snd_confirm();
			}
		}
	}

	if(abs(gui_acc_calib.filtered_raw.z) > (treshold_lo) )
	//if(abs(gui_acc_calib.filtered_raw.z) > (fc.acc_tot * treshold_percent / 100.0) )
	{
		if(fc.acc_data.z > 0 and gui_acc_calib.valid.Zpos == false)
		{
			gui_acc_calib.calc.pos_val.z = acc_calib_get_value(fc.acc_data.z);
			if(gui_acc_calib.is_sampling)
				gui_acc_calib.sampling.Zpos = true;
			if(gui_acc_calib.calc.pos_val.z)
			{
				gui_acc_calib.valid.Zpos = true;
				gui_acc_snd_confirm();
			}
		}
		if(fc.acc_data.z < 0 and gui_acc_calib.valid.Zneg == false)
		{
			gui_acc_calib.calc.neg_val.z = acc_calib_get_value(fc.acc_data.z);
			if(gui_acc_calib.is_sampling)
				gui_acc_calib.sampling.Zneg = true;
			if(gui_acc_calib.calc.neg_val.z)
			{
				gui_acc_calib.valid.Zneg = true;
				gui_acc_snd_confirm();
			}
		}
	}

	disp.LoadFont(F_TEXT_M);
	uint8_t h_t = disp.GetTextHeight();

	gui_dialog_P(PSTR("Accel calib"));

	char tmp[16];
	//screen before calib calculation
	if(gui_acc_calib.new_calib == false)
	{
		sprintf_P(tmp, PSTR("X+"));
		gui_caligh_text(tmp, GUI_DISP_WIDTH / 2 - 20 , GUI_DIALOG_TOP + 2 + h_t);
		sprintf_P(tmp, PSTR("X - "));
		gui_caligh_text(tmp, GUI_DISP_WIDTH / 2 - 20 , GUI_DIALOG_TOP + 2 + h_t + h_t);

		sprintf_P(tmp, PSTR("Y+"));
		gui_caligh_text(tmp, GUI_DISP_WIDTH / 2 +1, GUI_DIALOG_TOP + 2 + h_t);
		sprintf_P(tmp, PSTR("Y - "));
		gui_caligh_text(tmp, GUI_DISP_WIDTH / 2 +1, GUI_DIALOG_TOP + 2 + h_t + h_t);

		sprintf_P(tmp, PSTR("Z+"));
		gui_caligh_text(tmp, GUI_DISP_WIDTH / 2 + 20 +1, GUI_DIALOG_TOP + 2 + h_t);
		sprintf_P(tmp, PSTR("Z -"));
		gui_caligh_text(tmp, GUI_DISP_WIDTH / 2 + 20 +1, GUI_DIALOG_TOP + 2 + h_t + h_t);

		if(gui_acc_calib.valid.Xpos)
		{
			disp.Invert(GUI_DISP_WIDTH / 2 -20 -8, GUI_DIALOG_TOP + 2 + h_t -1, GUI_DISP_WIDTH / 2 -20 +6, GUI_DIALOG_TOP + 2 + h_t + h_t -2);
		}
		if(gui_acc_calib.valid.Xneg)
		{
			disp.Invert(GUI_DISP_WIDTH / 2 -20 -8, GUI_DIALOG_TOP + 2 + h_t + h_t - 1, GUI_DISP_WIDTH / 2 -20 +6, GUI_DIALOG_TOP + 2 + h_t + h_t + h_t -2);
		}

		if(gui_acc_calib.valid.Ypos)
		{
			disp.Invert(GUI_DISP_WIDTH / 2 -7, GUI_DIALOG_TOP + 2 + h_t -1, GUI_DISP_WIDTH / 2 +7 , GUI_DIALOG_TOP + 2 + h_t + h_t -2);
		}
		if(gui_acc_calib.valid.Yneg)
		{
			disp.Invert(GUI_DISP_WIDTH / 2 -7, GUI_DIALOG_TOP + 2 + h_t + h_t - 1, GUI_DISP_WIDTH / 2 +7, GUI_DIALOG_TOP + 2 + h_t + h_t + h_t -2);
		}

		if(gui_acc_calib.valid.Zpos)
		{
			disp.Invert(GUI_DISP_WIDTH / 2 +20 -6, GUI_DIALOG_TOP + 2 + h_t -1, GUI_DISP_WIDTH / 2 +20 +7, GUI_DIALOG_TOP + 2 + h_t + h_t -2);
		}
		if(gui_acc_calib.valid.Zneg)
		{
			disp.Invert(GUI_DISP_WIDTH / 2 +20 -6, GUI_DIALOG_TOP + 2 + h_t + h_t - 1, GUI_DISP_WIDTH / 2 +20 +7, GUI_DIALOG_TOP + 2 + h_t + h_t + h_t -2);
		}


		disp.LoadFont(F_TEXT_M);
		if(gui_acc_calib.sampling.Xpos)
		{
			sprintf_P(tmp, PSTR("Calibrating X+"));	//status area
			if(GUI_BLINK_TGL(500))					//blink what is being measured
				disp.Invert(GUI_DISP_WIDTH / 2 -20 -8, GUI_DIALOG_TOP + 2 + h_t -1, GUI_DISP_WIDTH / 2 -20 +6, GUI_DIALOG_TOP + 2 + h_t + h_t -2);
		}
		else if(gui_acc_calib.sampling.Xneg)
		{
			sprintf_P(tmp, PSTR("Calibrating X-"));
			if(GUI_BLINK_TGL(500))
				disp.Invert(GUI_DISP_WIDTH / 2 -20 -8, GUI_DIALOG_TOP + 2 + h_t + h_t - 1, GUI_DISP_WIDTH / 2 -20 +6, GUI_DIALOG_TOP + 2 + h_t + h_t + h_t -2);
		}
		else if(gui_acc_calib.sampling.Ypos)
		{
			sprintf_P(tmp, PSTR("Calibrating Y+"));
			if(GUI_BLINK_TGL(500))
				disp.Invert(GUI_DISP_WIDTH / 2 -7, GUI_DIALOG_TOP + 2 + h_t -1, GUI_DISP_WIDTH / 2 +7, GUI_DIALOG_TOP + 2 + h_t + h_t -2);
		}
		else if(gui_acc_calib.sampling.Yneg)
		{
			sprintf_P(tmp, PSTR("Calibrating Y-"));
			if(GUI_BLINK_TGL(500))
				disp.Invert(GUI_DISP_WIDTH / 2 -7, GUI_DIALOG_TOP + 2 + h_t + h_t - 1, GUI_DISP_WIDTH / 2 +7, GUI_DIALOG_TOP + 2 + h_t + h_t + h_t -2);
		}
		else if(gui_acc_calib.sampling.Zpos)
		{
			sprintf_P(tmp, PSTR("Calibrating Z+"));
			if(GUI_BLINK_TGL(500))
				disp.Invert(GUI_DISP_WIDTH / 2 +20 -6, GUI_DIALOG_TOP + 2 + h_t -1, GUI_DISP_WIDTH / 2 +20 +7, GUI_DIALOG_TOP + 2 + h_t + h_t -2);
		}
		else if(gui_acc_calib.sampling.Zneg)
		{
			sprintf_P(tmp, PSTR("Calibrating Z-"));
			if(GUI_BLINK_TGL(500))
				disp.Invert(GUI_DISP_WIDTH / 2 +20 -6, GUI_DIALOG_TOP + 2 + h_t + h_t - 1, GUI_DISP_WIDTH / 2 +20 +7, GUI_DIALOG_TOP + 2 + h_t + h_t + h_t -2);
		}
		else
		{
			sprintf_P(tmp, PSTR("Waiting..."));
		}
		gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, GUI_DIALOG_TOP + 1 );

	}
	//menu
	disp.LoadFont(F_TEXT_S);
	if(gui_acc_calib.valid.Xpos and gui_acc_calib.valid.Ypos and gui_acc_calib.valid.Zpos
			and gui_acc_calib.valid.Xneg and gui_acc_calib.valid.Yneg and gui_acc_calib.valid.Zneg)
	{
		disp.GotoXY(GUI_DIALOG_LEFT + 2, GUI_DIALOG_BOTTOM - 4 );
		fprintf_P(lcd_out, PSTR("save"));
	}
	gui_caligh_text_P(PSTR("back"), GUI_DISP_WIDTH / 2, GUI_DIALOG_BOTTOM - 4);
	//gui_raligh_text_P(PSTR("default"), GUI_DIALOG_RIGHT + 1 , GUI_DIALOG_BOTTOM - 4);


	//after sample collection
	if(gui_acc_calib.valid.Xpos and gui_acc_calib.valid.Ypos and gui_acc_calib.valid.Zpos
			and gui_acc_calib.valid.Xneg and gui_acc_calib.valid.Yneg and gui_acc_calib.valid.Zneg)
	{
		if( gui_acc_calib.new_calib == false )
		{
		//calculate gain, offset
			//DEBUG("%d %d %d %d %d %d\n", gui_acc_calib.calc.pos_val.x, gui_acc_calib.calc.neg_val.x, gui_acc_calib.calc.pos_val.y,gui_acc_calib.calc.neg_val.y, gui_acc_calib.calc.pos_val.z, gui_acc_calib.calc.neg_val.z);
			gui_acc_calib.sens.x = (float(gui_acc_calib.calc.pos_val.x) - float(gui_acc_calib.calc.neg_val.x) ) / 2 ;
			gui_acc_calib.bias.x = (float(gui_acc_calib.calc.pos_val.x) - gui_acc_calib.sens.x);
			gui_acc_calib.sens.y = (float(gui_acc_calib.calc.pos_val.y) - float(gui_acc_calib.calc.neg_val.y) ) / 2 ;
			gui_acc_calib.bias.y = (float(gui_acc_calib.calc.pos_val.y) - gui_acc_calib.sens.y);
			gui_acc_calib.sens.z = (float(gui_acc_calib.calc.pos_val.z) - float(gui_acc_calib.calc.neg_val.z) ) / 2 ;
			gui_acc_calib.bias.z = (float(gui_acc_calib.calc.pos_val.z) - gui_acc_calib.sens.z);

			//DEBUG("sens: %f %f %f bias: %f %f %f\n", gui_acc_calib.sens.x, gui_acc_calib.sens.y, gui_acc_calib.sens.z, gui_acc_calib.bias.x, gui_acc_calib.bias.y, gui_acc_calib.bias.z);
			gui_acc_snd_complete();

			gui_acc_calib.new_calib = true;
			return;

		}
		else //show acceleration with old and new calibration values
		{
		vector_float_t new_acc;
		new_acc.x = (float(fc.acc_data.x) - gui_acc_calib.bias.x) / gui_acc_calib.sens.x;
		new_acc.y = (float(fc.acc_data.y) - gui_acc_calib.bias.y) / gui_acc_calib.sens.y;
		new_acc.z = (float(fc.acc_data.z) - gui_acc_calib.bias.z) / gui_acc_calib.sens.z;

		float new_acc_tot = sqrt(new_acc.x * new_acc.x + new_acc.y * new_acc.y + new_acc.z * new_acc.z);

		gui_acc_calib.filtered_total_new = (gui_acc_calib.filtered_total_new - (LPF_beta * (gui_acc_calib.filtered_total_new - new_acc_tot)));
		gui_acc_calib.filtered_total_old = (gui_acc_calib.filtered_total_old - (LPF_beta * (gui_acc_calib.filtered_total_old - fc.acc_tot)));


		disp.LoadFont(F_TEXT_M);

		//status area
		sprintf_P(tmp, PSTR("Done!"));
		gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, GUI_DIALOG_TOP + 1 );
		//

		disp.LoadFont(F_TEXT_S);
		sprintf_P(tmp, PSTR("old"));
		gui_caligh_text(tmp, GUI_DISP_WIDTH / 2 - 20 , GUI_DIALOG_TOP +  h_t );
		sprintf_P(tmp, PSTR("new"));
		gui_caligh_text(tmp, GUI_DISP_WIDTH / 2 + 20 , GUI_DIALOG_TOP +  h_t );

		disp.LoadFont(F_TEXT_L);
		//show current acceleration value with old calibration values
		sprintf_P(tmp, PSTR("%0.2f"), gui_acc_calib.filtered_total_old);
		gui_caligh_text(tmp, GUI_DISP_WIDTH / 2 - 20, GUI_DIALOG_TOP  + h_t + h_t -2);

		//show current acceleration value with new calibration values
		sprintf_P(tmp, PSTR("%0.2f"), gui_acc_calib.filtered_total_new);
		gui_caligh_text(tmp, GUI_DISP_WIDTH / 2 + 20, GUI_DIALOG_TOP  + h_t + h_t -2);
		}
	}
}


void gui_accelerometer_calib_irqh(uint8_t type, uint8_t * buff)
{
	if (*buff == BE_CLICK && type == B_MIDDLE)
		gui_switch_task(GUI_SET_ADVANCED);

	if (*buff == BE_LONG && type == B_MIDDLE)
		gui_switch_task(GUI_SET_ADVANCED);

	if (*buff == BE_CLICK && type == B_LEFT && gui_acc_calib.new_calib)
	{
		//apply new settings
		acc_data.calibration.sens = gui_acc_calib.sens;
		acc_data.calibration.bias = gui_acc_calib.bias;

		//DEBUG("saved values: bias %f %f %f sens %f %f %f\n", gui_acc_calib.bias.x, gui_acc_calib.bias.y, gui_acc_calib.bias.z, gui_acc_calib.sens.x, gui_acc_calib.sens.y, gui_acc_calib.sens.z);

		//save settings to eeprom
		accel_save_calibration(gui_acc_calib.sens, gui_acc_calib.bias);

		accel_load_calibration(&gui_acc_calib.sens, &gui_acc_calib.bias);
		//return to menu
		gui_switch_task(GUI_SET_ADVANCED);
	}
}
