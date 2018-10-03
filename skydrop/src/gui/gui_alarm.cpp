#include "gui_alarm.h"

#include "../../fc/conf.h"
#include "../../fc/fc.h"
#include "../../drivers/audio/sequencer.h"

#define GUI_ALARM1_PERIOD	1000
#define GUI_ALARM2_PERIOD	500
#define GUI_ALARM_H1_PERIOD	1000
#define GUI_ALARM_TIMEOUT	5000

uint32_t gui_alarm_switch_back = 0;

MK_SEQ(alarm1, ARR({1000, 750}), ARR({500, 500}));
MK_SEQ(alarm2, ARR({1000, 750}), ARR({250, 250}));
MK_SEQ(alarm_h1, ARR({1000, 750}), ARR({500, 500}));

void gui_set_alarm_init()
{
	gui_alarm_switch_back = 0;

	switch (fc_active_alarm())
	{
		case (1):
			seq_start(&alarm1, config.gui.alert_volume, true);
		break;
		case (2):
			seq_start(&alarm2, config.gui.alert_volume, true);
		break;
		case (3):
			seq_start(&alarm_h1, config.gui.alert_volume, true);
		break;
	}
}

void gui_set_alarm_stop()
{
	seq_stop();
}

void gui_set_alarm_loop()
{
	char tmp[12];

	disp.LoadFont(F_TEXT_L);

	uint8_t n = fc_active_alarm();

	switch (n)
	{
		case(0):
			if (gui_alarm_switch_back == 0)
			{
				gui_alarm_switch_back = task_get_ms_tick() + GUI_ALARM_TIMEOUT;
			}
			else
			{
				if (gui_alarm_switch_back < task_get_ms_tick())
					gui_switch_task(GUI_PAGES);
			}

			seq_stop();
		break;

		case(1):
			if (&alarm1 != seq_active())
			{
				seq_start(&alarm1, config.gui.alert_volume, true);
				gui_alarm_switch_back = 0;
			}
		break;

		case(2):
			if (&alarm2 != seq_active())
			{
				seq_start(&alarm2, config.gui.alert_volume, true);
				gui_alarm_switch_back = 0;
			}
		break;

		case(3):
			if (&alarm_h1 != seq_active())
			{
				seq_start(&alarm_h1, config.gui.alert_volume, true);
				gui_alarm_switch_back = 0;
			}
		break;

	}

	if (n == 0)
		sprintf_P(tmp, PSTR("Altitude"), n);
	else
		sprintf_P(tmp, PSTR("Alarm %u"), n);

	gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, 1);

	disp.LoadFont(F_VALUES_XL);
	float val = fc.altitude1 * ((config.altitude.alt1_flags & ALT_UNIT_I) ? FC_METER_TO_FEET : 1);
	sprintf_P(tmp, PSTR("%0.0f"), val);
	gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, 13);

	disp.LoadFont(F_TEXT_S);
	uint8_t f_h = disp.GetAHeight();
	gui_caligh_text_P(PSTR("Confirm"), GUI_DIALOG_LEFT + GUI_DIALOG_WIDTH / 2, GUI_DISP_HEIGHT - f_h -1);

	//blink if alarm is active
	if (n != 0)
		if (GUI_BLINK_TGL((n == 1) ? GUI_ALARM1_PERIOD : GUI_ALARM2_PERIOD))
		{
			disp.InvertPart(0, 0, GUI_DISP_WIDTH, 8);
			disp.PutPixel(0, 0, DISP_COLOR_WHITE);
			disp.PutPixel(0, GUI_DISP_HEIGHT - 1, DISP_COLOR_WHITE);
			disp.PutPixel(GUI_DISP_WIDTH - 1, 0, DISP_COLOR_WHITE);
			disp.PutPixel(GUI_DISP_WIDTH - 1, GUI_DISP_HEIGHT - 1, DISP_COLOR_WHITE);
		}
}

void gui_set_alarm_irqh(uint8_t type, uint8_t * buff)
{
	if (type == TASK_IRQ_BUTTON_M)
	{
		gui_switch_task(GUI_PAGES);

		//Suppress alarm
		switch (fc_active_alarm())
		{
			case (1):
				fc.altitude_alarm_status |= 0b00000001;
				break;
			case (2):
				fc.altitude_alarm_status |= 0b00000011;
				break;
			case (3):
				fc.altitude_alarm_status |= 0b00000100;
				break;
		}
	}
}




