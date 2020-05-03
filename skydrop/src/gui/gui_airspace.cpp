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

	//Draw the widget
	widget_airspace_info_draw(0, 0, GUI_DISP_WIDTH, 40);

	disp.LoadFont(F_TEXT_S);
	uint8_t f_h = disp.GetAHeight();


	//timeout
	if (config.airspaces.alarm_confirm_secs != 0 && gui_airspace_started != DISABLE_COUNTER)
	{
		char text[8];

		sprintf_P(text, PSTR("%u"), config.airspaces.alarm_confirm_secs - ((task_get_ms_tick() - gui_airspace_started) / 1000));
		gui_raligh_text(text, GUI_DISP_WIDTH, GUI_DISP_HEIGHT - f_h -1);
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

		//invalidate the cache
		fc.airspace.cache_index = AIR_INDEX_INVALID;

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




