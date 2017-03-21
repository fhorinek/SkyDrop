#include "set_gps.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"
#include "../../drivers/sensors/gps_l80.h"

void gui_set_gps_init()
{
	gui_list_set(gui_set_gps_item, gui_set_gps_action, 6, GUI_SETTINGS);

	if (config.connectivity.use_gps)
		gps_detail();
}

void gui_set_gps_stop()
{
	if (config.connectivity.use_gps)
		gps_normal();
}

void gui_set_gps_action(uint8_t index)
{
	uint8_t tmp;

	switch(index)
	{
	case(0):
		config.connectivity.use_gps = !config.connectivity.use_gps;
		eeprom_busy_wait();
		eeprom_update_byte(&config_ee.connectivity.use_gps, config.connectivity.use_gps);
		if (config.connectivity.use_gps)
			gps_start();
		else
			gps_stop();
	break;

	case(1):
		if (!config.connectivity.use_gps)
			gui_showmessage_P(PSTR("Enable GPS first"));
		else
			gui_switch_task(GUI_SET_GPS_DETAIL);
	break;

	case(2):
		if (config.connectivity.use_gps)
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
		if (config.connectivity.use_gps)
		{
			if (fc.gps_data.fix_cnt > GPS_FIX_TIME_SYNC)
			{
				fc_sync_gps_time();
			}
			else
			{
				gui_showmessage_P(PSTR("Wait for GPS"));
			}
		}
		else
			gui_showmessage_P(PSTR("Enable GPS first"));
	break;

	case(4):
		tmp = (config.connectivity.gps_format_flags & GPS_SPD_MASK) >> 0;
		tmp = (tmp + 1) % 4;
		config.connectivity.gps_format_flags = (config.connectivity.gps_format_flags & ~GPS_SPD_MASK) | (tmp << 0);
		eeprom_busy_wait();
		eeprom_update_byte(&config_ee.connectivity.gps_format_flags, config.connectivity.gps_format_flags);
	break;

	case(5):
		tmp = (config.connectivity.gps_format_flags & GPS_FORMAT_MASK) >> 2;
		tmp = (tmp + 1) % 3;
		config.connectivity.gps_format_flags = (config.connectivity.gps_format_flags & ~GPS_FORMAT_MASK) | (tmp << 2);
		eeprom_busy_wait();
		eeprom_update_byte(&config_ee.connectivity.gps_format_flags, config.connectivity.gps_format_flags);
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
			strcpy_P(text, PSTR("Enable GPS"));
			if (config.connectivity.use_gps)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;

		break;

		case (1):
			strcpy_P(text, PSTR("Status"));
			if (config.connectivity.use_gps)
			{
				switch (fc.gps_data.fix)
				{
					case(2):
						strcpy_P(tmp, PSTR("2D"));
					break;
					case(3):
						strcpy_P(tmp, PSTR("3D"));
					break;
					default:
						strcpy_P(tmp, PSTR("No"));
					break;
				}
				sprintf_P(sub_text, PSTR("%d/%d %s Fix"), fc.gps_data.sat_used, fc.gps_data.sat_total, tmp);
			}
			else
			{
				strcpy_P(sub_text, PSTR("GPS disabled"));
			}

			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (2):
			strcpy_P(text, PSTR("GPS time"));
			if (fc.gps_data.valid)
				sprintf_P(sub_text, PSTR("%02d:%02d.%02d"), hour, min, sec);
			else
			{
				if (config.connectivity.use_gps)
					strcpy_P(sub_text, PSTR("Waiting for fix"));
				else
					strcpy_P(sub_text, PSTR("GPS disabled"));
			}

			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (3):
			strcpy_P(text, PSTR("GPS date"));
			if (fc.gps_data.valid)
				sprintf_P(sub_text, PSTR("%02d/%02d/%04d"), day, month, year);
			else
			{
				if (config.connectivity.use_gps)
					strcpy_P(sub_text, PSTR("Waiting for fix"));
				else
					strcpy_P(sub_text, PSTR("GPS disabled"));
			}

			*flags |= GUI_LIST_SUB_TEXT;
		break;

		case (4):
			strcpy_P(text, PSTR("Speed units"));
			*flags |= GUI_LIST_SUB_TEXT;
			switch (config.connectivity.gps_format_flags & GPS_SPD_MASK)
			{
				case(GPS_SPD_KNOT):
					strcpy_P(sub_text, PSTR("Knots"));
				break;
				case(GPS_SPD_KPH):
					strcpy_P(sub_text, PSTR("Km/h"));
				break;
				case(GPS_SPD_MS):
					strcpy_P(sub_text, PSTR("m/s"));
				break;
				case(GPS_SPD_MPH):
					strcpy_P(sub_text, PSTR("MPH"));
				break;
			}
		break;

		case (5):
			strcpy_P(text, PSTR("Format"));
			*flags |= GUI_LIST_SUB_TEXT;
			switch (config.connectivity.gps_format_flags & GPS_FORMAT_MASK)
			{
				case(GPS_DDMMSS):
					strcpy_P(sub_text, PSTR("DD*MM'SS\""));
				break;
				case(GPS_DDMMmmm):
					strcpy_P(sub_text, PSTR("DD*MM.mmm'"));
				break;
				case(GPS_DDdddddd):
					strcpy_P(sub_text, PSTR("DD.dddddd"));
				break;
			}
		break;

	}
}

