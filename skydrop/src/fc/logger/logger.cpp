#include "logger.h"

#include "../fc.h"

#include "igc.h"
#include "kml.h"
#include "raw.h"
#include "aero.h"

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

	if (!fc.vario.valid)
		return;

	//RAW is running as fast as it can!
	if (config.logger.format != LOGGER_RAW && config.logger.format != LOGGER_AERO)
	{
		if (fc.gps_data.new_sample & FC_GPS_NEW_SAMPLE_LOGGER)
		{
			logger_next = task_get_ms_tick() + 1000;
			fc.gps_data.new_sample &= ~FC_GPS_NEW_SAMPLE_LOGGER;
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

		case(LOGGER_AERO):
			aero_step();
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

		case(LOGGER_RAW):
		case(LOGGER_AERO):
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
	sprintf_P(path, PSTR("%S"), LOG_DIR_P);
	f_mkdir(path);
	//day
	sprintf_P(path, PSTR("%S/%04d-%02d-%02d"), LOG_DIR_P, year, month, day);
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

		case(LOGGER_AERO):
			fc.logger_state = aero_start(path);
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

		case(LOGGER_RAW):
			raw_stop();
		break;

		case(LOGGER_AERO):
			aero_stop();
		break;
	}
}

/**
 * Traverse dirname and return the path entry number "count" in fname.
 * If the entry is found, then "0" is returned. Otherwise the remaining
 * value of count is returned. E.g. if the current directory holds 3
 * files and we look for count=5, then 2 is returned.
 *
 * @param dirname The name of the directory, where we start searching
 * @param count   The number of the entry, which we are searching, start with "1"
 * @param fname   A pointer to a memory area allocated by caller to take full filename
 *                (including directory) of the found filename
 * @param dirOnly Set to "true", if we only look for directories.
 *                "false" means, to look for files only.
 *
 * @return the remaining count.
 */
uint16_t logger_fileno(const char *dirname, uint16_t count, char *fname, bool dirOnly)
{
	FRESULT f_result;
	DIR dir;
	FILINFO fileinfo;
	char next_dirname[128];

	f_result = f_opendir(&dir, dirname);
	if (f_result != FR_OK )
		return count;

	while(1) {
		f_result = f_readdir (&dir, &fileinfo);
		if ( fileinfo.fname[0] == 0) break;           // end of directory

		if ( fileinfo.fattrib & AM_DIR ) {
			sprintf(next_dirname, "%s/%s", dirname, fileinfo.fname);
			count = logger_fileno(next_dirname, count, fname, dirOnly);
			if (count == 0) break;
		} else {
			count--;
			if (count == 0) {
				if (dirOnly) {
					sprintf(fname, "%s", dirname);
				} else {
					sprintf(fname, "%s/%s", dirname, fileinfo.fname);
				}
				break;
			}
			if (dirOnly) {
				break;
			}
		}
	}

	f_closedir(&dir);

	return count;
}

/**
 * Traverse LOG_DIR and return the path entry number "count" in fname.
 * If the entry is found, then "0" is returned. Otherwise the remaining
 * value of count is returned. E.g. if the current directory holds 3
 * files and we look for count=5, then 2 is returned.
 *
 * @param count   The number of the entry, which we are searching, start with "1"
 * @param fname   A pointer to a memory area allocated by caller to take full filename
 *                (including directory) of the found filename
 * @param dirOnly Set to "true", if we only look for directories.
 *                "false" means, to look for files only.
 *
 * @return the remaining count.
 */
uint16_t logger_fileno(uint16_t count, char *fname, bool dirOnly)
{
	return logger_fileno(LOG_DIR, count, fname, dirOnly);
}

/**
 * Return how many entries we have.
 *
 * @param dirname The name of the directory to search through all subdirs
 * @param dirOnly Set to "true", if we only look for directories.
 *                "false" means, to look for files only.
 *
 * @return the number of path entries found.
 */
uint16_t logger_count(const char *dirname, bool dirOnly)
{
	char fname[128];

	#define MAX_NUM_LOGS 1000

	return MAX_NUM_LOGS - logger_fileno(dirname, MAX_NUM_LOGS, fname, dirOnly);
}

/**
 * Return how many entries we have in the LOG_DIR directory.
 *
 * @param dirOnly Set to "true", if we only look for directories.
 *                "false" means, to look for files only.
 *
 * @return the number of path entries found.
 */
uint16_t logger_count(bool dirOnly)
{
	return logger_count(LOG_DIR, dirOnly);
}
