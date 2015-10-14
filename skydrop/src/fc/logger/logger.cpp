#include "logger.h"

#include "../fc.h"
#include "igc.h"

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

	f_mkdir(LOG_DIR);
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

	logger_next = task_get_ms_tick() + 2000;

	if (fc.flight_state == FLIGHT_FLIGHT)
	{
		//XXX: only igc so far
		igc_step();
	}
}

void logger_start()
{
	logger_next_flight();

	if (igc_start())
		fc.logger_state = LOGGER_ACTIVE;

}

void logger_stop()
{
	fc.logger_state = LOGGER_IDLE;
	igc_stop();
}
