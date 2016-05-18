#include "logger.h"

#include "../fc.h"

#include "igc.h"
#include "kml.h"
#include "raw.h"

FIL * log_fil;
uint32_t logger_next = 0;
uint8_t logger_flight_number;
uint32_t logger_flight_day;

void logger_init()
{
	log_fil = new FIL;
	fc.logger_state = LOGGER_IDLE;

	uint8_t sec, min, hour, day, wday, month;
	uint16_t year;
	uint32_t today;


	datetime_from_epoch(time_get_local(), &sec, &min, &hour, &day, &wday, &month, &year);
	today = datetime_to_epoch(0, 0, 0, day, month, year);

	eeprom_busy_wait();
	eeprom_read_block((void *)&logger_flight_day, &config_ro.flight_date, sizeof(logger_flight_day));
	logger_flight_number = eeprom_read_byte(&config_ro.flight_number);

	if (logger_flight_day != today)
	{
		logger_flight_number = 0;
		logger_flight_day = today;

		eeprom_busy_wait();
		eeprom_update_block((void *)&logger_flight_day, &config_ro.flight_date, sizeof(logger_flight_day));
		eeprom_update_byte(&config_ro.flight_number, logger_flight_number);
	}

	DEBUG("date is: ");
	print_datetime(today);
	DEBUG("flight number is: %d\n", logger_flight_number);

}

void logger_next_flight()
{
	uint8_t sec, min, hour, day, wday, month;
	uint16_t year;
	uint32_t today;


	datetime_from_epoch(time_get_local(), &sec, &min, &hour, &day, &wday, &month, &year);
	today = datetime_to_epoch(0, 0, 0, day, month, year);
	
	if (today == logger_flight_day)
	{
		logger_flight_number++;

		eeprom_busy_wait();
		eeprom_update_byte(&config_ro.flight_number, logger_flight_number);
	}
	else
	{
		logger_flight_number = 0;
		logger_flight_day = today;

		eeprom_busy_wait();
		eeprom_update_block((void *)&logger_flight_day, &config_ro.flight_date, sizeof(logger_flight_day));
		eeprom_update_byte(&config_ro.flight_number, logger_flight_number);
	}

	DEBUG("date is: ");
	print_datetime(today);
	DEBUG("flight number is: %d\n", logger_flight_number);
}

void logger_step()
{
	if (!logger_active())
		return;

	if (logger_next > task_get_ms_tick())
		return;

	if (!fc.baro_valid)
		return;

	//RAW is running as fast as it can!
	if (config.logger.format != LOGGER_RAW)
	{
		if (fc.gps_data.new_sample)
		{
			logger_next = task_get_ms_tick() + 1000;
			fc.gps_data.new_sample = false;
		}
	}

	switch (config.logger.format)
	{
		case(LOGGER_IGC):
			igc_step();
		break;

		case(LOGGER_KML):
			kml_step();
		break;

		case(LOGGER_RAW):
			raw_step();
		break;
	}
}

void logger_comment(char * text)
{
	if (!logger_active())
		return;

	switch (config.logger.format)
	{
		case(LOGGER_IGC):
			igc_comment(text);
		break;

		case(LOGGER_KML):
			kml_comment(text);
		break;

		case(LOGGER_RAW):
			DEBUG("%s\n", text);
		break;
	}
}

void logger_start()
{
	if (!config.logger.enabled)
		return;

	logger_next_flight();

	if (!storage_ready())
	{
		gui_showmessage_P(PSTR("SD card error!"));

		fc.logger_state = LOGGER_ERROR;

		return;
	}

	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t day;
	uint8_t wday;
	uint8_t month;
	uint16_t year;

	datetime_from_epoch(time_get_utc(), &sec, &min, &hour, &day, &wday, &month, &year);

	char path[128];

	//base dir
	sprintf_P(path, PSTR("%S"), LOG_DIR);
	f_mkdir(path);
	//year
	sprintf_P(path, PSTR("%S/%04d"), LOG_DIR, year);
	f_mkdir(path);
	//month
	sprintf_P(path, PSTR("%S/%04d/%02d"), LOG_DIR, year, month);
	f_mkdir(path);
	//day
	sprintf_P(path, PSTR("%S/%04d/%02d/%02d"), LOG_DIR, year, month, day);
	f_mkdir(path);

	switch (config.logger.format)
	{
		case(LOGGER_IGC):
			fc.logger_state = igc_start(path);
		break;

		case(LOGGER_KML):
			fc.logger_state = kml_start(path);
		break;

		case(LOGGER_RAW):
			fc.logger_state = raw_start(path);
		break;
	}
}

//logger is active or it is waiting for gps
bool logger_active()
{
	return (fc.logger_state == LOGGER_ACTIVE || fc.logger_state == LOGGER_WAIT_FOR_GPS);
}

bool logger_error()
{
	return fc.logger_state == LOGGER_ERROR;
}

void logger_stop()
{
	if (!logger_active())
		return;

	if (fc.logger_state == LOGGER_WAIT_FOR_GPS)
	{
		char text[32];
		strcpy_P(text, PSTR("No GPS fix during flight!"));

		logger_comment(text);
	}

	fc.logger_state = LOGGER_IDLE;

	switch (config.logger.format)
	{
		case(LOGGER_IGC):
			igc_stop();
		break;

		case(LOGGER_KML):
			kml_stop();
		break;

		case(LOGGER_RAW):
			raw_step();
		break;
	}
}
