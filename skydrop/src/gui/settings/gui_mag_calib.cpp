/*
 * gui_mag_calib.cpp
 *
 *  Created on: Dec 5, 2016
 *      Author: fiala
 */

#include "gui_mag_calib.h"
#include "../gui_dialog.h"
#include "../../drivers/audio/sequencer.h"
#include "../../fc/mag.h"

gui_mag_calib_t gui_mag_calib;
extern mag_calc_data_t mag_calc_data;


#define BAR_PADDING			4
#define BAR_TEXT_SPACE 		10
#define BAR_WIDTH  			GUI_DISP_WIDTH - 2 * BAR_PADDING - BAR_TEXT_SPACE
#define BAR_X1_POS			BAR_PADDING + BAR_TEXT_SPACE
#define BAR_X2_POS			BAR_X1_POS + BAR_WIDTH
#define BAR_CENTER_POS		(BAR_X1_POS + BAR_X2_POS) / 2
#define BAR_MAX_LENGHT		(BAR_WIDTH - 2) / 2
#define BAR_HEIGHT			6


MK_SEQ(snd_confirm, ARR({750}), ARR({150}));

void gui_mag_snd_update()
{
	if(not seq_enabled)
		seq_start(&snd_confirm, config.gui.alert_volume);
}
int8_t gui_mag_calc_bar(int16_t value, int16_t min, int16_t max)
{
	if(value >= 0)
		return int8_t(int( BAR_MAX_LENGHT * (float(value) / float(max)) + 0.5));
	else
		return int8_t(int(-BAR_MAX_LENGHT * (float(value) / float(min)) - 0.5));
}

void gui_mag_calib_get_extrem(int16_t in_value, int16_t * out_min_value, int16_t * out_max_value)
{
	if(in_value > *out_max_value)
		*out_max_value = in_value;
	if(in_value < *out_min_value)
		*out_min_value = in_value;
	else
		return;
	gui_mag_snd_update();
}

void gui_mag_calib_init()
{
	gui_mag_calib.samples.max.x = 0;
	gui_mag_calib.samples.max.y = 0;
	gui_mag_calib.samples.max.z = 0;
	gui_mag_calib.samples.min.x = 0;
	gui_mag_calib.samples.min.y = 0;
	gui_mag_calib.samples.min.z = 0;
}

void gui_mag_calib_stop() {}

void gui_mag_calib_loop()
{
	//get min, max values to each direction
	gui_mag_calib_get_extrem( fc.mag_data_raw.x, &gui_mag_calib.samples.min.x, &gui_mag_calib.samples.max.x );
	gui_mag_calib_get_extrem( fc.mag_data_raw.y, &gui_mag_calib.samples.min.y, &gui_mag_calib.samples.max.y );
	gui_mag_calib_get_extrem( fc.mag_data_raw.z, &gui_mag_calib.samples.min.z, &gui_mag_calib.samples.max.z );

	gui_dialog_P(PSTR("Magnetometer"));
	disp.LoadFont(F_TEXT_M);
	uint8_t h_t = disp.GetTextHeight();

	disp.GotoXY(GUI_DIALOG_LEFT + 2 , GUI_DIALOG_TOP + 1);
	fprintf_P(lcd_out, PSTR("X"));
	disp.GotoXY(GUI_DIALOG_LEFT + 2, GUI_DIALOG_TOP + 1 + h_t);
	fprintf_P(lcd_out, PSTR("Y"));
	disp.GotoXY(GUI_DIALOG_LEFT + 2, GUI_DIALOG_TOP + 1 + h_t + h_t);
	fprintf_P(lcd_out, PSTR("Z"));

	//draw frames
	disp.DrawRectangle(BAR_X1_POS , GUI_DIALOG_TOP + 1 , BAR_X2_POS, GUI_DIALOG_TOP + 1 + BAR_HEIGHT, 1, 0);
	disp.DrawRectangle(BAR_X1_POS , GUI_DIALOG_TOP + 1 + h_t, BAR_X2_POS, GUI_DIALOG_TOP + 1 + h_t + BAR_HEIGHT, 1, 0);
	disp.DrawRectangle(BAR_X1_POS , GUI_DIALOG_TOP + 1 + h_t + h_t, BAR_X2_POS, GUI_DIALOG_TOP + 1 + h_t + h_t + BAR_HEIGHT, 1, 0);

	//draw value bars
	int8_t value;
	value = gui_mag_calc_bar(fc.mag_data_raw.x, gui_mag_calib.samples.min.x, gui_mag_calib.samples.max.x);
	disp.DrawRectangle(BAR_CENTER_POS , GUI_DIALOG_TOP + 2 , BAR_CENTER_POS + value, GUI_DIALOG_TOP  + BAR_HEIGHT, 1, 1);
	value = gui_mag_calc_bar(fc.mag_data_raw.y, gui_mag_calib.samples.min.y, gui_mag_calib.samples.max.y);
	disp.DrawRectangle(BAR_CENTER_POS , GUI_DIALOG_TOP + 2 + h_t , BAR_CENTER_POS + value, GUI_DIALOG_TOP + h_t + BAR_HEIGHT, 1, 1);
	value = gui_mag_calc_bar(fc.mag_data_raw.z, gui_mag_calib.samples.min.z, gui_mag_calib.samples.max.z);
	disp.DrawRectangle(BAR_CENTER_POS , GUI_DIALOG_TOP + 2 + h_t + h_t , BAR_CENTER_POS + value, GUI_DIALOG_TOP + h_t + h_t + BAR_HEIGHT, 1, 1);

	//float size = sqrt(fc.mag_data.x * fc.mag_data.x + fc.mag_data.y * fc.mag_data.y + fc.mag_data.z * fc.mag_data.z);
	//DEBUG("mag vector: %f   %f   %f      %f\n", fc.mag_data.x, fc.mag_data.y, fc.mag_data.z, size);

	disp.LoadFont(F_TEXT_S);
	gui_caligh_text_P(PSTR("back"), GUI_DISP_WIDTH / 2, GUI_DIALOG_BOTTOM - 4);
	disp.GotoXY(GUI_DIALOG_LEFT + 2, GUI_DIALOG_BOTTOM - 4 );
	fprintf_P(lcd_out, PSTR("save"));
}

void gui_mag_calib_irqh(uint8_t type, uint8_t * buff)
{
	if (*buff == BE_CLICK && type == B_MIDDLE)
		gui_switch_task(GUI_SET_CALIB);

	if (*buff == BE_LONG && type == B_MIDDLE)
		gui_switch_task(GUI_SET_CALIB);

	if (*buff == BE_CLICK && type == B_LEFT)
	{
		mag_calc_data.calibration.bias.x = float(gui_mag_calib.samples.max.x + gui_mag_calib.samples.min.x) / 2;
		mag_calc_data.calibration.bias.y = float(gui_mag_calib.samples.max.y + gui_mag_calib.samples.min.y) / 2;
		mag_calc_data.calibration.bias.z = float(gui_mag_calib.samples.max.z + gui_mag_calib.samples.min.z) / 2;

		mag_calc_data.calibration.sens.x = float(gui_mag_calib.samples.max.x - gui_mag_calib.samples.min.x) / 2;
		mag_calc_data.calibration.sens.y = float(gui_mag_calib.samples.max.y - gui_mag_calib.samples.min.y) / 2;
		mag_calc_data.calibration.sens.z = float(gui_mag_calib.samples.max.z - gui_mag_calib.samples.min.z) / 2;

		mag_save_calibration(mag_calc_data.calibration.sens, mag_calc_data.calibration.bias);

		gui_switch_task(GUI_SET_CALIB);
	}
}




