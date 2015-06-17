#include "set_gps.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"
#include "../../drivers/sensors/gps_l80.h"

void gui_set_gps_init()
{
	gui_list_set(gui_set_gps_item, gui_set_gps_action, 6);

	if (fc.use_gps)
		gps_detail();
}

void gui_set_gps_stop()
{

}

void gui_set_gps_loop()
{
	gui_list_draw();
}

void gui_set_gps_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_set_gps_action(uint8_t index)
{
	switch(index)
	{
	case(0):
		fc.use_gps = !fc.use_gps;
		eeprom_busy_wait();
		eeprom_update_byte(&config.system.use_gps, fc.use_gps);
		if (fc.use_gps)
			gps_start();
		else
			gps_stop();
	break;

	case(1):
		if (!fc.use_gps)
			gui_showmessage_P(PSTR("Enable GPS first"));
		else
			gui_switch_task(GUI_SET_GPS_DETAIL);
	break;

	case(2):
		if (fc.use_gps)
		{
			if (fc.gps_data.fix_cnt > GPS_FIX_TIME_SYNC)
				fc_sync_gps_time();
			else
				gui_showmessage_P(PSTR("Wait for GPS"));
		}
		else
			gui_showmessage_P(PSTR("Enable GPS first"));
	break;

	case(3):
		if (fc.use_gps)
		{
			if (fc.gps_data.fix_cnt > GPS_FIX_TIME_SYNC)
				fc_sync_gps_time();
			else
				gui_showmessage_P(PSTR("Wait for GPS"));
		}
		else
			gui_showmessage_P(PSTR("Enable GPS first"));
	break;

	case(4):
		fc.sync_gps_time = !fc.sync_gps_time;
		eeprom_busy_wait();
		eeprom_update_byte(&config.system.sync_gps_time, fc.sync_gps_time);
	break;

	case(5):
		gui_switch_task(GUI_SETTINGS);
		if (fc.use_gps)
			gps_normal();
	break;
	}
}

void gui_set_gps_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	char tmp[10];

	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t day;
	uint8_t wday;
	uint8_t month;
	uint16_t year;

	if (fc.gps_data.valid)
	{
		datetime_from_epoch(fc.gps_data.utc_time, &sec, &min, &hour, &day, &wday, &month, &year);
	}

	switch (index)
	{
		case (0):
			sprintf_P(text, PSTR("Enable GPS"));
			if (fc.use_gps)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;

		break;

		case (1):
			sprintf_P(text, PSTR("Status"));
			if (fc.use_gps)
			{
				switch (fc.gps_data.fix)
				{
					case(2):
						sprintf_P(tmp, PSTR("2D"));
					break;
					case(3):
						sprintf_P(tmp, PSTR("3D"));
					break;
					default:
						sprintf_P(tmp, PSTR("No"));
					break;
				}
				sprintf_P(sub_text, PSTR("%d/%d %s Fix"), fc.gps_data.sat_used, fc.gps_data.sat_total, tmp);
			}
			else
			{
				sprintf_P(sub_text, PSTR("GPS disabled"));
			}

			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (2):
			sprintf_P(text, PSTR("GPS time"));
			if (fc.gps_data.valid)
				sprintf_P(sub_text, PSTR("%02d:%02d.%02d"), hour, min, sec);
			else
			{
				if (fc.use_gps)
					sprintf_P(sub_text, PSTR("Waiting for fix"));
				else
					sprintf_P(sub_text, PSTR("GPS disabled"));
			}

			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (3):
			sprintf_P(text, PSTR("GPS date"));
			if (fc.gps_data.valid)
				sprintf_P(sub_text, PSTR("%02d/%02d/%04d"), day, month, year);
			else
			{
				if (fc.use_gps)
					sprintf_P(sub_text, PSTR("Waiting for fix"));
				else
					sprintf_P(sub_text, PSTR("GPS disabled"));
			}

			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (4):
			sprintf_P(text, PSTR("Sync GPS time"));
			if (fc.sync_gps_time)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;
		break;

		case (5):
			sprintf_P(text, PSTR("back"));
			*flags |= GUI_LIST_BACK;
		break;

	}
}

