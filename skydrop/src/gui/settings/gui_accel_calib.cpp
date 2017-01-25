/*
 * gui_accel_calib.cpp
 *
 *  Created on: Nov 21, 2016
 *      Author: Fiala
 */

#include "gui_accel_calib.h"
#include "../../fc/acc.h"
#include "../gui_dialog.h"
#include "../../drivers/audio/sequencer.h"
#include "../../drivers/lcd_disp.h"


struct acc_calib_calc_t
{
	int16_t old_value;
	uint8_t counter;
	int32_t avg_value;
	vector_i16_t pos_val;
	vector_i16_t neg_val;
};

#define VALID_XP	0b00000001
#define VALID_XN	0b00000010
#define VALID_YP	0b00000100
#define VALID_YN	0b00001000
#define VALID_ZP	0b00010000
#define VALID_ZN	0b00100000

#define VALID_CALIB 0b10000000

struct acc_calib_t
{
	acc_calib_calc_t calc;
	uint8_t valid;

	bool new_calib;
	uint8_t sampling;

	vector_float_t bias;
	vector_float_t sens;
	float filtered_total_new;
	float filtered_total_old;
};


//#define treshold_percent	90.0
#define treshold_lo  		0.75 * default1G
//#define treshold_hi  		1.10 * default1G
#define default1G			1300

#define avg_weight_calc 	10		//averaging, weight of old samples - getting values for calculation of bias and sensitivity
#define avg_weight_flow 	5		//averaging, weight of old samples - decision which axis will be set
#define LPF_beta 			0.05	//low pass filter setting, for values shown at point of decision

#define calib_samples		100		//n of samples collected for calculating accel calibration
#define max_err				40		//max variance between samples

acc_calib_t gui_acc_calib;

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
	gui_acc_calib.valid = 0;

	gui_acc_calib.calc.counter = 0;

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

int16_t acc_calib_get_value(uint8_t flag, int16_t in_value)
{
	uint16_t err = abs(in_value - gui_acc_calib.calc.old_value);
	gui_acc_calib.calc.old_value = in_value;
	//*out_value = 0;

	if (err <= max_err)
	{
		if(gui_acc_calib.calc.counter == 0)
			gui_acc_calib.calc.avg_value = in_value;
		else
			gui_acc_calib.calc.avg_value = ((gui_acc_calib.calc.avg_value * avg_weight_calc - gui_acc_calib.calc.avg_value + in_value) / avg_weight_calc);
		gui_acc_calib.calc.counter += 1;
	}
	else
	{
		gui_acc_calib.calc.counter = 0;

		//not stable
		return 0;
	}

	if (gui_acc_calib.calc.counter < calib_samples)
	{
		if(gui_acc_calib.calc.counter * 4 > calib_samples)
		{
			gui_acc_calib.sampling = flag;
		}
		//collecting samples
		return 0;
	}
	else
	{
		gui_acc_calib.calc.counter = 0;

		gui_acc_calib.valid |= flag;
		gui_acc_snd_confirm();

		//calibrating done
		return gui_acc_calib.calc.avg_value;
	}
}


void gui_accelerometer_calib_loop()
{


	gui_acc_calib.sampling = 0;

	//DEBUG("%d    %d    %d\n", fc.acc_raw.x, fc.acc_raw.y, fc.acc_raw.z);

	if(abs(fc.acc.raw.x) > (treshold_lo) )
	//if(abs(fc.acc_raw.x) > (fc.acc_tot * treshold_percent / 100.0) )
	{
		if(fc.acc.raw.x > 0 and !(gui_acc_calib.valid & VALID_XP))
			gui_acc_calib.calc.pos_val.x = acc_calib_get_value(VALID_XP, fc.acc.raw.x);

		if(fc.acc.raw.x < 0 and !(gui_acc_calib.valid & VALID_XN))
			gui_acc_calib.calc.neg_val.x = acc_calib_get_value(VALID_XN, fc.acc.raw.x);
	}

	if(abs(fc.acc.raw.y) > (treshold_lo) )
	//if(abs(fc.acc_raw.y) > (fc.acc_tot * treshold_percent / 100.0) )
	{
		if(fc.acc.raw.y > 0 and !(gui_acc_calib.valid & VALID_YP))
			gui_acc_calib.calc.pos_val.y = acc_calib_get_value(VALID_YP, fc.acc.raw.y);

		if(fc.acc.raw.y < 0 and !(gui_acc_calib.valid & VALID_YN))
			gui_acc_calib.calc.neg_val.y = acc_calib_get_value(VALID_YN, fc.acc.raw.y);
	}

	if(abs(fc.acc.raw.z) > (treshold_lo) )
	//if(abs(fc.acc_raw.z) > (fc.acc_tot * treshold_percent / 100.0) )
	{
		if(fc.acc.raw.z > 0 and !(gui_acc_calib.valid & VALID_ZP))
			gui_acc_calib.calc.pos_val.z = acc_calib_get_value(VALID_ZP, fc.acc.raw.z);

		if(fc.acc.raw.z < 0 and !(gui_acc_calib.valid & VALID_ZN))
			gui_acc_calib.calc.neg_val.z = acc_calib_get_value(VALID_ZN, fc.acc.raw.z);
	}

	disp.LoadFont(F_TEXT_M);
	uint8_t h_t = disp.GetTextHeight();

	gui_dialog_P(PSTR("Accelerometer"));

	char tmp[16];
	//screen before calib calculation
	if((gui_acc_calib.valid & VALID_CALIB) == false)
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

		if(gui_acc_calib.valid & VALID_XP)
			disp.Invert(GUI_DISP_WIDTH / 2 -20 -8, GUI_DIALOG_TOP + 2 + h_t -1, GUI_DISP_WIDTH / 2 -20 +6, GUI_DIALOG_TOP + 2 + h_t + h_t -2);

		if(gui_acc_calib.valid & VALID_XN)
			disp.Invert(GUI_DISP_WIDTH / 2 -20 -8, GUI_DIALOG_TOP + 2 + h_t + h_t - 1, GUI_DISP_WIDTH / 2 -20 +6, GUI_DIALOG_TOP + 2 + h_t + h_t + h_t -2);

		if(gui_acc_calib.valid & VALID_YP)
			disp.Invert(GUI_DISP_WIDTH / 2 -7, GUI_DIALOG_TOP + 2 + h_t -1, GUI_DISP_WIDTH / 2 +7 , GUI_DIALOG_TOP + 2 + h_t + h_t -2);

		if(gui_acc_calib.valid & VALID_YN)
			disp.Invert(GUI_DISP_WIDTH / 2 -7, GUI_DIALOG_TOP + 2 + h_t + h_t - 1, GUI_DISP_WIDTH / 2 +7, GUI_DIALOG_TOP + 2 + h_t + h_t + h_t -2);

		if(gui_acc_calib.valid & VALID_ZP)
			disp.Invert(GUI_DISP_WIDTH / 2 +20 -6, GUI_DIALOG_TOP + 2 + h_t -1, GUI_DISP_WIDTH / 2 +20 +7, GUI_DIALOG_TOP + 2 + h_t + h_t -2);

		if(gui_acc_calib.valid & VALID_ZN)
			disp.Invert(GUI_DISP_WIDTH / 2 +20 -6, GUI_DIALOG_TOP + 2 + h_t + h_t - 1, GUI_DISP_WIDTH / 2 +20 +7, GUI_DIALOG_TOP + 2 + h_t + h_t + h_t -2);


		disp.LoadFont(F_TEXT_M);
		if(gui_acc_calib.sampling == VALID_XP)
		{
			sprintf_P(tmp, PSTR("Calibrating X+"));	//status area
			if(GUI_BLINK_TGL(500))					//blink what is being measured
				disp.Invert(GUI_DISP_WIDTH / 2 -20 -8, GUI_DIALOG_TOP + 2 + h_t -1, GUI_DISP_WIDTH / 2 -20 +6, GUI_DIALOG_TOP + 2 + h_t + h_t -2);
		}
		else if(gui_acc_calib.sampling == VALID_XN)
		{
			sprintf_P(tmp, PSTR("Calibrating X-"));
			if(GUI_BLINK_TGL(500))
				disp.Invert(GUI_DISP_WIDTH / 2 -20 -8, GUI_DIALOG_TOP + 2 + h_t + h_t - 1, GUI_DISP_WIDTH / 2 -20 +6, GUI_DIALOG_TOP + 2 + h_t + h_t + h_t -2);
		}
		else if(gui_acc_calib.sampling == VALID_YP)
		{
			sprintf_P(tmp, PSTR("Calibrating Y+"));
			if(GUI_BLINK_TGL(500))
				disp.Invert(GUI_DISP_WIDTH / 2 -7, GUI_DIALOG_TOP + 2 + h_t -1, GUI_DISP_WIDTH / 2 +7, GUI_DIALOG_TOP + 2 + h_t + h_t -2);
		}
		else if(gui_acc_calib.sampling == VALID_YN)
		{
			sprintf_P(tmp, PSTR("Calibrating Y-"));
			if(GUI_BLINK_TGL(500))
				disp.Invert(GUI_DISP_WIDTH / 2 -7, GUI_DIALOG_TOP + 2 + h_t + h_t - 1, GUI_DISP_WIDTH / 2 +7, GUI_DIALOG_TOP + 2 + h_t + h_t + h_t -2);
		}
		else if(gui_acc_calib.sampling == VALID_ZP)
		{
			sprintf_P(tmp, PSTR("Calibrating Z+"));
			if(GUI_BLINK_TGL(500))
				disp.Invert(GUI_DISP_WIDTH / 2 +20 -6, GUI_DIALOG_TOP + 2 + h_t -1, GUI_DISP_WIDTH / 2 +20 +7, GUI_DIALOG_TOP + 2 + h_t + h_t -2);
		}
		else if(gui_acc_calib.sampling == VALID_ZN)
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
	gui_caligh_text_P(PSTR("back"), GUI_DISP_WIDTH / 2, GUI_DIALOG_BOTTOM - 4);

	if(gui_acc_calib.valid == ( VALID_XP | VALID_XN | VALID_YP | VALID_YN | VALID_ZP | VALID_ZN ) )
	{
		//calculate gain, offset

		gui_acc_calib.sens.x = (float(gui_acc_calib.calc.pos_val.x) - float(gui_acc_calib.calc.neg_val.x) ) / 2 ;
		gui_acc_calib.sens.y = (float(gui_acc_calib.calc.pos_val.y) - float(gui_acc_calib.calc.neg_val.y) ) / 2 ;
		gui_acc_calib.sens.z = (float(gui_acc_calib.calc.pos_val.z) - float(gui_acc_calib.calc.neg_val.z) ) / 2 ;
		gui_acc_calib.bias.x = (float(gui_acc_calib.calc.pos_val.x) - gui_acc_calib.sens.x);
		gui_acc_calib.bias.y = (float(gui_acc_calib.calc.pos_val.y) - gui_acc_calib.sens.y);
		gui_acc_calib.bias.z = (float(gui_acc_calib.calc.pos_val.z) - gui_acc_calib.sens.z);

		//DEBUG("%d %d %d %d %d %d\n", gui_acc_calib.calc.pos_val.x, gui_acc_calib.calc.neg_val.x, gui_acc_calib.calc.pos_val.y,gui_acc_calib.calc.neg_val.y, gui_acc_calib.calc.pos_val.z, gui_acc_calib.calc.neg_val.z);
		//DEBUG("sens: %f %f %f bias: %f %f %f\n", gui_acc_calib.sens.x, gui_acc_calib.sens.y, gui_acc_calib.sens.z, gui_acc_calib.bias.x, gui_acc_calib.bias.y, gui_acc_calib.bias.z);
		gui_acc_snd_complete();
		gui_acc_calib.valid |= VALID_CALIB;
	}

	if(gui_acc_calib.valid == ( VALID_XP | VALID_XN | VALID_YP | VALID_YN | VALID_ZP | VALID_ZN | VALID_CALIB) )
	{
		//calibration values were calculated

		vector_float_t new_acc;
		new_acc.x = (float(fc.acc.raw.x) - gui_acc_calib.bias.x) / gui_acc_calib.sens.x;
		new_acc.y = (float(fc.acc.raw.y) - gui_acc_calib.bias.y) / gui_acc_calib.sens.y;
		new_acc.z = (float(fc.acc.raw.z) - gui_acc_calib.bias.z) / gui_acc_calib.sens.z;

		float new_acc_tot = sqrt(new_acc.x * new_acc.x + new_acc.y * new_acc.y + new_acc.z * new_acc.z);

		gui_acc_calib.filtered_total_new = (gui_acc_calib.filtered_total_new - (LPF_beta * (gui_acc_calib.filtered_total_new - new_acc_tot)));
		gui_acc_calib.filtered_total_old = (gui_acc_calib.filtered_total_old - (LPF_beta * (gui_acc_calib.filtered_total_old - fc.acc.total)));


		disp.LoadFont(F_TEXT_M);

		//status area
		sprintf_P(tmp, PSTR("Done!"));
		gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, GUI_DIALOG_TOP + 1 );

		//main area labels
		disp.LoadFont(F_TEXT_S);
		sprintf_P(tmp, PSTR("old"));
		gui_caligh_text(tmp, GUI_DISP_WIDTH / 2 - 20 , GUI_DIALOG_TOP +  h_t );
		sprintf_P(tmp, PSTR("new"));
		gui_caligh_text(tmp, GUI_DISP_WIDTH / 2 + 20 , GUI_DIALOG_TOP +  h_t );

		//add menu option
		disp.GotoXY(GUI_DIALOG_LEFT + 2, GUI_DIALOG_BOTTOM - 4 );
		fprintf_P(lcd_out, PSTR("save"));

		//main area values
		disp.LoadFont(F_TEXT_L);
		//show current acceleration value with old calibration values
		sprintf_P(tmp, PSTR("%0.2f"), gui_acc_calib.filtered_total_old);
		gui_caligh_text(tmp, GUI_DISP_WIDTH / 2 - 20, GUI_DIALOG_TOP  + h_t + h_t -2);

		//show current acceleration value with new calibration values
		sprintf_P(tmp, PSTR("%0.2f"), gui_acc_calib.filtered_total_new);
		gui_caligh_text(tmp, GUI_DISP_WIDTH / 2 + 20, GUI_DIALOG_TOP  + h_t + h_t -2);
	}
}


void gui_accelerometer_calib_irqh(uint8_t type, uint8_t * buff)
{
	if (*buff == BE_CLICK && type == B_MIDDLE)
		gui_switch_task(GUI_SET_CALIB);

	if (*buff == BE_LONG && type == B_MIDDLE)
		gui_switch_task(GUI_SET_CALIB);

	if (*buff == BE_CLICK && type == B_LEFT && ( gui_acc_calib.valid & VALID_CALIB ))
	{	//if calibration is done

		//apply new settings
		memcpy((void *)&fc.acc.sens, &gui_acc_calib.sens, sizeof(vector_float_t));
		memcpy((void *)&fc.acc.bias, &gui_acc_calib.bias, sizeof(vector_float_t));

		//save settings to eeprom
		acc_save_calibration(gui_acc_calib.sens, gui_acc_calib.bias);

		//return to menu
		gui_switch_task(GUI_SET_CALIB);
	}
}

