#include "gui_alarm.h"

#include "../../fc/conf.h"
#include "../../fc/fc.h"
#include "../../drivers/audio/sequencer.h"

// #include "../../debug_on.h"

#define GUI_ALARM1_PERIOD	1000
#define GUI_ALARM2_PERIOD	500
#define GUI_ALARM_H1_PERIOD	1000
#define GUI_ALARM_TIMEOUT	5000

uint32_t gui_alarm_switch_back = 0;
uint32_t gui_alarm_started = 0;         // at what time did the alarm start? 0 means, no alarm

MK_SEQ(alarm1, ARR({1000, 750}), ARR({500, 500}));
MK_SEQ(alarm2, ARR({1000, 750}), ARR({250, 250}));
MK_SEQ(alarm_h1, ARR({1000, 750}), ARR({500, 500}));

void suppress_alarm()
{
	gui_switch_task(GUI_PAGES);

	//Suppress alarm
	switch (fc_alarm_running(fc_active_alarm()))
	{
		case (1):
			fc.altitude_alarm_status |= FC_ALT_ALARM1_SUPPRESS;
			break;
		case (2):
			fc.altitude_alarm_status |= FC_ALT_ALARM1_SUPPRESS | FC_ALT_ALARM2_SUPPRESS;
			break;
		case (3):
			fc.altitude_alarm_status |= FC_ALT_ALARM_H1_SUPPRESS;
			break;
	}
}

void gui_alarm_init()
{
	gui_alarm_switch_back = 0;
	gui_alarm_started = 0;

	switch (fc_alarm_running(fc_active_alarm()))
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

void gui_alarm_stop()
{
	seq_stop();
}

void gui_alarm_loop()
{
	char tmp[12];

	uint8_t n = fc_alarm_running(fc_active_alarm());
	DEBUG("fc_active_alarm()=%d, fc.altitude_alarm_status=0x%2x\n", n, fc.altitude_alarm_status);

	switch (n)
	{
		case(0):
			// Alarm is now off. We wait for GUI_ALARM_TIMEOUT millisecs until we return to GUI_PAGES:
			if (gui_alarm_switch_back == 0)
			{
				gui_alarm_switch_back = task_get_ms_tick() + GUI_ALARM_TIMEOUT;
				seq_stop();
			}
			else
			{
				if (gui_alarm_switch_back < task_get_ms_tick())
					gui_switch_task(GUI_PAGES);
			}
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
	{
		gui_alarm_started = 0;
		sprintf_P(tmp, PSTR("Altitude"));
	}
	else
	{
		if (gui_alarm_started == 0)
		{
			// The alarm is new and started right now. Save the time.
			gui_alarm_started = task_get_ms_tick();
		}
		else
		{
			// This is an already running alarm.
			if (config.altitude.alarm_confirm_secs != 0)
			{
				if (task_get_ms_tick() > gui_alarm_started + 1000 * config.altitude.alarm_confirm_secs)
				{
					suppress_alarm();
				}
			}
		}
		sprintf_P(tmp, PSTR("Alarm %u"), n);
	}

	disp.LoadFont(F_TEXT_L);
	gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, 1);

	disp.LoadFont(F_VALUES_XL);
	float val = fc.altitude1 * ((config.altitude.alt1_flags & ALT_UNIT_I) ? FC_METER_TO_FEET : 1);
	sprintf_P(tmp, PSTR("%0.0f"), val);
	gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, 13);

	disp.LoadFont(F_TEXT_S);
	uint8_t f_h = disp.GetAHeight();
	gui_caligh_text_P(PSTR("Confirm"), GUI_DISP_WIDTH / 2, GUI_DISP_HEIGHT - f_h -1);

	if (config.altitude.alarm_confirm_secs != 0)
	{
		sprintf_P(tmp, PSTR("%u"), config.altitude.alarm_confirm_secs - ((task_get_ms_tick() - gui_alarm_started) / 1000));
		gui_raligh_text(tmp, GUI_DISP_WIDTH, GUI_DISP_HEIGHT - f_h -1);
	}

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

void gui_alarm_irqh(uint8_t type, uint8_t * buff)
{
	if (type == TASK_IRQ_BUTTON_M)
	{
		suppress_alarm();
	}
}




