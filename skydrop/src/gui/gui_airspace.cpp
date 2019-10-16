#include "gui_airspace.h"

#include "../../fc/conf.h"
#include "../../fc/fc.h"
#include "../../fc/airspace.h"

#include "../../drivers/audio/sequencer.h"
#include "widgets/widgets.h"
#include "gui_dialog.h"

 #include "../../debug_on.h"

uint32_t gui_airspace_started = 0;         // at what time did the airspace start? 0 means, no airspace

MK_SEQ(airspace_warn, ARR({1000}), ARR({250}));
MK_SEQ(airspace_alert, ARR({1000, 0, 1000}), ARR({250, 250, 255}));

#define DISABLE_COUNTER 0xFFFFFFFF

void gui_airspace_init()
{
	gui_airspace_started = 0;

	if (fc.airspace.inside)
		seq_start(&airspace_alert, config.gui.alert_volume);
	else
		seq_start(&airspace_warn, config.gui.alert_volume);
}

void gui_airspace_hide()
{
	fc.airspace.hidden = fc.airspace.airspace_index | (fc.airspace.inside ? AIR_HIDE_INSIDE : 0x00);
	gui_switch_task(GUI_PAGES);
}

void gui_airspace_loop()
{
	if (gui_airspace_started == 0)
		{
			gui_airspace_started = task_get_ms_tick();
		}
		else
		{
			// This is an already running alarm.
			if (config.airspaces.alarm_confirm_secs != 0 && gui_airspace_started != DISABLE_COUNTER)
			{
				if (task_get_ms_tick() > gui_airspace_started + 1000 * config.airspaces.alarm_confirm_secs)
				{
					gui_airspace_hide();
				}
			}
		}

	char text1[12];
	char text2[12];

	//Name
	widget_value_scroll((char *)fc.airspace.airspace_name, 0, 0, GUI_DISP_WIDTH, 12);
	//Class
	airspace_class_to_text(text1, fc.airspace.airspace_class);

	disp.LoadFont(F_LABEL);
	uint8_t f_h = disp.GetAHeight();
	gui_raligh_text(text1, GUI_DISP_WIDTH - 2, 10);

	disp.GotoXY(10, 12);
	if (fc.airspace.inside)
	{
		strcpy_P(text1, PSTR("INSIDE!"));
		fprintf(lcd_out, text1);

		if (GUI_BLINK_TGL(500))
		{
			uint8_t f_w = disp.GetTextWidth(text1);
			disp.Invert(8, 10, 10 + f_w, 13 + f_h);
		}
	}
	else
	{
		strcpy_P(text1, PSTR("Outside"));
		fprintf(lcd_out, text1);
	}

	//Direction
	int16_t relative_direction;
	relative_direction = fc.airspace.angle - fc.compass.azimuth_filtered;
	widget_arrow(relative_direction, 0, 18, 22, 20);

	//Distance
	sprintf_distance(text1, fc.airspace.distance_m / 1000.0);
	if (config.connectivity.gps_format_flags & GPS_DIST_UNIT_I)
		strcpy_P(text2, PSTR("mi"));
	else
		strcpy_P(text2, PSTR("km"));

	widget_value_int_sub(text1, text2, 22, 18, 20, 20);

	//Floor & ceil
	disp.LoadFont(F_TEXT_S);
	f_h = disp.GetAHeight();

	const char * msl = PSTR("Msl");
	const char * agl = PSTR("Agl");

	if (config.altitude.alt1_flags & ALT_UNIT_I)
	{
		sprintf_P(text1, PSTR("%u %S"), airspace_convert_alt_ft(fc.airspace.ceiling), (fc.airspace.ceiling & AIR_AGL_FLAG) ? agl : msl);
		sprintf_P(text2, PSTR("%u %S"), airspace_convert_alt_ft(fc.airspace.floor), (fc.airspace.floor & AIR_AGL_FLAG) ? agl : msl);
	}
	else
	{
		sprintf_P(text1, PSTR("%u %S"), airspace_convert_alt_m(fc.airspace.ceiling), (fc.airspace.ceiling & AIR_AGL_FLAG) ? agl : msl);
		sprintf_P(text2, PSTR("%u %S"), airspace_convert_alt_m(fc.airspace.floor), (fc.airspace.floor & AIR_AGL_FLAG) ? agl : msl);
	}

	disp.GotoXY(49, 17);
	fprintf_P(lcd_out, PSTR("Ceiling"));
	disp.GotoXY(53, 23);
	fprintf(lcd_out, text1);

	disp.GotoXY(49, 29);
	fprintf_P(lcd_out, PSTR("Floor"));
	disp.GotoXY(53, 35);
	fprintf(lcd_out, text2);

	//timeout
	if (config.airspaces.alarm_confirm_secs != 0 && gui_airspace_started != DISABLE_COUNTER)
	{
		sprintf_P(text1, PSTR("%u"), config.airspaces.alarm_confirm_secs - ((task_get_ms_tick() - gui_airspace_started) / 1000));
		gui_raligh_text(text1, GUI_DISP_WIDTH, GUI_DISP_HEIGHT - f_h -1);
	}

	//Actions
	gui_caligh_text_P(PSTR("Hide"), GUI_DISP_WIDTH / 2, GUI_DISP_HEIGHT - f_h -1);

	disp.GotoXY(2, GUI_DISP_HEIGHT - f_h -1);
	fprintf_P(lcd_out, PSTR("Ignore"));
}

//to prevent racing condition
uint8_t gui_airspace_ignore_index;
char gui_airspace_ignore_filename[10];

void gui_airspace_ignore_cb(uint8_t ret)
{
	uint8_t hard = 2;

	if (ret == GUI_DIALOG_NOW)
	{
		hard = false;
	}

	if (ret == GUI_DIALOG_FOREVER)
	{
		hard = true;
	}

	if (hard != 2)
	{
		FIL f;

		uint8_t buff[sizeof(fc.airspace.ignore)];
		memset(buff, 0, sizeof(buff));

	    airspace_read_ignore_file(&f, hard, buff, gui_airspace_ignore_filename);

		uint8_t ignore_index = gui_airspace_ignore_index / 8;
		uint8_t ignore_bit = 1 << (gui_airspace_ignore_index % 8);
		buff[ignore_index] |= ignore_bit;

		//add to RAM ignore if the filename was not changed
		if (strcmp(gui_airspace_ignore_filename, (char *)fc.airspace.filename) == 0)
			fc.airspace.ignore[ignore_index] |= ignore_bit;

		//write it to sd card ignore
		airspace_write_ignore_file(&f, hard, buff, gui_airspace_ignore_filename);

		gui_switch_task(GUI_PAGES);
	}
	else
	{
		gui_switch_task(GUI_AIRSPACE_ALARM);
	}
}

void gui_airspace_irqh(uint8_t type, uint8_t * buff)
{
	if (type == TASK_IRQ_BUTTON_L)
	{
		char text1[7];
		char text2[64];
		char name[16];

		strcpy_P(text1, PSTR("Ignore"));
		memcpy(name, (void *)fc.airspace.airspace_name, 15);
		name[min(strlen((char *)fc.airspace.airspace_name), 15)] = 0;
		sprintf_P(text2, PSTR("Ignore airspace\n%s\nfor %s"), name, fc.airspace.filename);

		//to prevent changing airspace index or filename during dialog
		gui_airspace_ignore_index = fc.airspace.airspace_index;
		strcpy(gui_airspace_ignore_filename, (char *)fc.airspace.filename);

		gui_dialog_set(text1, text2, GUI_STYLE_AIRSPACE, gui_airspace_ignore_cb);
		gui_switch_task(GUI_DIALOG);
	}

	if (type == TASK_IRQ_BUTTON_M)
	{
		gui_airspace_hide();
	}

	if (type == TASK_IRQ_BUTTON_R)
	{
		gui_airspace_started = DISABLE_COUNTER;
	}

}




