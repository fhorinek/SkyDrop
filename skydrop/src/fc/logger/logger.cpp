#include "logger.h"

#include "../fc.h"

#include "igc.h"
#include "kml.h"

FIL log_file;
uint32_t logger_next = 0;
uint8_t logger_flight_number;
uint32_t logger_flight_day;

void logger_init()
{
	fc.logger_state = LOGGER_IDLE;

	uint8_t sec, min, hour, day, wday, month;
	uint16_t year;
	uint32_t today;


	datetime_from_epoch(time_get_local(), &sec, &min, &hour, &day, &wday, &month, &year);
	today = datetime_to_epoch(0, 0, 0, day, month, year);

	
	ee_read_block((void *)&logger_flight_day, &config_ro.flight_date, sizeof(logger_flight_day));
	ee_read_byte(&config_ro.flight_number, logger_flight_number);

	if (logger_flight_day != today)
	{
		logger_flight_number = 0;
		logger_flight_day = today;

		
		ee_update_block((void *)&logger_flight_day, &config_ro.flight_date, sizeof(logger_flight_day));
		ee_update_byte(&config_ro.flight_number, logger_flight_number);
	}

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

		
		ee_update_byte(&config_ro.flight_number, logger_flight_number);
	}
	else
	{
		logger_flight_number = 0;
		logger_flight_day = today;

		
		ee_update_block((void *)&logger_flight_day, &config_ro.flight_date, sizeof(logger_flight_day));
		ee_update_byte(&config_ro.flight_number, logger_flight_number);
	}

	DEBUG("flight number is: %d\n", logger_flight_number);
}

void logger_step()
{
	if (!fc.vario.valid)
		return;

	if (logger_next > task_get_ms_tick())
		return;

	if (fc.gps_data.new_sample & FC_GPS_NEW_SAMPLE_LOGGER)
	{
		logger_next = task_get_ms_tick() + 1000;
		fc.gps_data.new_sample &= ~FC_GPS_NEW_SAMPLE_LOGGER;
	}

	if (!logger_active())
	{
		if (config.logger.format == LOGGER_IGC)
			igc_pre_step();

		return;
	}

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

/**
 * printf-like function to send output to the GPS log .
 *
 * \param format a printf-like format string that must reside in PROGMEM.
 *
 */
void logger_comment(const char *format, ...)
{
	va_list arp;
	char text[80];

	if (!logger_active())
		return;

	va_start(arp, format);
	vsnprintf_P(text, sizeof(text), format, arp);
	va_end(arp);

	switch (config.logger.format)
	{
		case(LOGGER_IGC):
			igc_comment(text);
		break;

		case(LOGGER_KML):
			kml_comment(text);
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

	datetime_from_epoch(time_get_local(), &sec, &min, &hour, &day, &wday, &month, &year);

	char path[128];

	//base dir
	sprintf_P(path, PSTR("%S"), LOG_DIR_P);
	f_mkdir(path);
	//year
	sprintf_P(path, PSTR("%S/%04u"), LOG_DIR_P, year, month, day);
	f_mkdir(path);
	//month-day
	sprintf_P(path, PSTR("%S/%04u/%02u-%02u"), LOG_DIR_P, year, month, day);
	f_mkdir(path);

	if (fc.flight.state == FLIGHT_FLIGHT)
		sprintf_P(path, PSTR("/%S/%04u/%02u-%02u/%02d-%02d%02d."), LOG_DIR_P, year, month, day, logger_flight_number, hour, min);
	else
		sprintf_P(path, PSTR("/%S/%04u/%02u-%02u/%02d-%02d%02dH."), LOG_DIR_P, year, month, day, logger_flight_number, hour, min);

	//	f_mkdir(path);

	switch (config.logger.format)
	{
		case(LOGGER_IGC):
			fc.logger_state = igc_start(path);
		break;

		case(LOGGER_KML):
			fc.logger_state = kml_start(path);
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
		logger_comment(PSTR("No GPS fix during flight!"));
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
	}
}

