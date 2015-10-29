#include "logger.h"

#include "../fc.h"

#include "igc.h"
#include "kml.h"

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


	datetime_from_epoch(time_get_actual(), &sec, &min, &hour, &day, &wday, &month, &year);
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


	datetime_from_epoch(time_get_actual(), &sec, &min, &hour, &day, &wday, &month, &year);
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
	if (fc.logger_state != LOGGER_ACTIVE)
		return;

	if (logger_next > task_get_ms_tick())
		return;

	logger_next = task_get_ms_tick() + 1000;

	if (fc.flight_state == FLIGHT_FLIGHT)
	{
		switch (config.logger.format)
		{
			case(LOGGER_IGC):
				igc_step();
			break;
			case(LOGGER_KML):
				kml_step();
			break;
		}
	}
}

void logger_start()
{
	logger_next_flight();

	if (!storage_selftest())
		gui_showmessage_P(PSTR("SD card error!"));

	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t day;
	uint8_t wday;
	uint8_t month;
	uint16_t year;

	datetime_from_epoch(fc.gps_data.utc_time, &sec, &min, &hour, &day, &wday, &month, &year);

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

	bool ret = false;

	switch (config.logger.format)
	{
		case(LOGGER_IGC):
			ret = igc_start(path);
		break;
		case(LOGGER_KML):
			ret = kml_start(path);
		break;
	}

	if (ret)
		fc.logger_state = LOGGER_ACTIVE;

}

void logger_stop()
{
	if (fc.logger_state != LOGGER_ACTIVE)
		return;

	fc.logger_state = LOGGER_IDLE;

	switch (config.logger.format)
	{
		case(LOGGER_IGC):
			igc_stop();
		break;
		case(LOGGER_KML):
			kml_stop();
		break;
	}
}
