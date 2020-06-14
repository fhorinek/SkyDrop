#include "battery.h"
#include "../tasks/tasks.h"

//#include "../debug_on.h"

#define BATTERY_STATE_IDLE		0
#define BATTERY_STATE_PREPARE	1
#define BATTERY_STATE_START		2
#define BATTERY_STATE_RESULT	3

#define BATTERY_MEAS_AVG 		16
#define BATTERY_MEAS_PERIOD 	10000
#define BATTERY_STABILISE 		100

#define BAT_ADC_MIN				(3200)

uint32_t battery_next_meas = 0;
uint8_t  battery_meas_state = BATTERY_STATE_PREPARE;

uint16_t battery_meas_acc = 0;
uint8_t  battery_meas_cnt = 0;

uint16_t battery_adc_raw = 0;
int8_t battery_per = 0;

uint16_t bat_adc_max;
uint16_t bat_adc_dif;

#define BAT_CAL_REMIND_INTERVAL 10          // Interval in seconds to remind user to discharge
#define BAT_CAL_INTERVAL 60                 // Interval in seconds to save calibration data

/** This is the ADC value where the next lower percent value starts. */
uint16_t bat_next_level = 0;

/** Do we have calibration data during normal operation. */
bool bat_cal_available = false;

/**
 * This is a state machine for battery calibration:
 *
 * BATTERY_CAL_NONE: No calibration is running, normal operation.
 * BATTERY_CAL_START: A battery calibration is starting. Inform user of what is going on. Then go to
 * BATTERY_CAL_DISCHARGE: wait until battery is fully discharged, store calibration data every BAT_CAL_INTERVAL.
 */
uint8_t battery_calibrating_state = BATTERY_CAL_NONE;

/**
 * Return the number of minutes, that the battery runs, if full.
 *
 * @return number of minutes for full capacity
 */
uint16_t battery_runtime_minutes()
{
	uint16_t value;

	
	ee_read_word(&config_ro.bat_runtime_minutes, value);
	//DEBUG("config_ro.bat_runtime_minutes=%u\n", value);
	return value;
}

/**
 * Check, if we have calibration data for the battery in EEPROM.
 *
 * @return true if data is available, false otherwise.
 */
bool battery_calibrated()
{
	return ( battery_runtime_minutes() != BATTERY_CAL_INVALID );
}

/**
 * Read EEPROM and search for given "battery_adc_raw". Return corresponding percent value.
 * It also sets "bat_next_level" for the next lower level.
 *
 * @param battery_adc_raw the ADC value to search for
 *
 * @return the corresponding percent value or 101 for error.
 */
uint8_t read_battery_per_from_calibration(uint16_t battery_adc_raw)
{
	uint8_t percent;
	uint16_t battery;

	bat_next_level = 0;

	for (percent = 100; percent > 0; percent--)
	{
		
		ee_read_word(&config_ro.bat_calibration[100 - percent], battery);
		if ( battery_adc_raw > battery )
		{
			bat_next_level = battery;
			break;
		}
	}

	return percent;
}

/**
 * Get corresponding percent value for given "battery_adc_raw". This will use "bat_next_value" to
 * only read EEPROM if necessary.
 *
 * @param battery_adc_raw the ADC value to search for
 *
 * @return the corresponding percent value
 */
int8_t get_battery_per_from_calibration(uint16_t battery_adc_raw)
{
	uint8_t percent;

	/* Check, if we reached the next level. If not return identical percent */
	if (bat_next_level != 0 && battery_adc_raw > bat_next_level)
		return battery_per;

	percent = read_battery_per_from_calibration(battery_adc_raw);
	DEBUG("get_battery_per_from_calibration(%d)=%d\n", battery_adc_raw, percent);

	return percent;
}

void battery_reset_calibration()
{
	if (bat_adc_max == BAT_ADC_MIN)
		return;

	bat_adc_max = BAT_ADC_MIN;

	DEBUG("Resetting max ADC value\n");

	
	ee_update_word(&config_ro.bat_adc_max, bat_adc_max);
}

void battery_init()
{
	
	ee_read_word(&config_ro.bat_adc_max, bat_adc_max);

	if (bat_adc_max > 4095)
		battery_reset_calibration();

	bat_adc_dif = bat_adc_max - BAT_ADC_MIN;

	if (battery_calibrated())
		bat_cal_available = true;

	BATTERY_ADC_PWR_ON;
	BATTERY_ADC_ENABLE;

	AdcAInit(adc_int1V);
	AdcASetMode(adc_unsigned);

	AdcPipeSetSource(pipea0, BAT_SNS_ADC);

	GpioSetDirection(BAT_EN, OUTPUT);
	bat_en_low(BAT_EN_ADC);

	GpioSetPull(CHARGING, gpio_pull_up);
}

void battery_force_update()
{
	battery_next_meas = 0;
	battery_meas_state = BATTERY_STATE_PREPARE;
	battery_meas_acc = 0;
	battery_meas_cnt = 0;
}

uint16_t bat_charge_discharge_ratio = 0;

/**
 * Save the given ADC value into BAT_CAL_FILE_RAW.
 *
 * @param battery_adc_raw the value to append at the end of the file.
 */
void save_bat_cal_value(uint16_t battery_adc_raw)
{
	uint8_t res;
	FIL cal_file;
	uint16_t wt;

	if (!storage_ready())
		return;

	// open or create new
	res = f_open(&cal_file, BAT_CAL_FILE_RAW, FA_WRITE | FA_OPEN_ALWAYS);
	if (res != FR_OK)
		return;

	//seek to end
	res = f_lseek(&cal_file, f_size(&cal_file));
	if (res != FR_OK)
	{
		f_close(&cal_file);
		return;
	}

	f_write(&cal_file, &battery_adc_raw, sizeof(battery_adc_raw), &wt);
	f_close(&cal_file);

	DEBUG("Battery cal: wrote %d\n", battery_adc_raw);
}

/**
 * This function reads calibration from BAT_CAL_FILE_RAW and creates EEPROM used during normal
 * operations. To do it creates 100 values which correspond to 100% until 0%. Each value is computed by using
 * the average to multiple available ADC values.
 */
void battery_finish_calibration()
{
	uint8_t res;
	FIL cal_file_raw;
	uint16_t wt;
	uint16_t battery;
	bool error = false;
	uint16_t num;

	res = f_open(&cal_file_raw, BAT_CAL_FILE_RAW, FA_READ);
	if (res != FR_OK)
		return;

	DEBUG("resort_bat_cal: f_size(raw): %ld\n", f_size(&cal_file_raw));
	DEBUG("---Start---\n");
	/* This can be saved to eeprom to store the number of minutes, the battery lasts. */
	num = f_size(&cal_file_raw) / sizeof(uint16_t);

	for (uint8_t i = 0; i < 100; i++)
	{
		uint16_t start = (num * i) / 100;
		res = f_lseek(&cal_file_raw, (uint32_t)start * sizeof(uint16_t));

		if (res != FR_OK)
		{
			DEBUG("resort_bat_cal: f_lseek failed\n");
			error = true;
			break;
		}

		uint32_t sum = 0;
		uint16_t num_values = 0;

		for (uint16_t j = 0; j <= num / 100; j++)
		{
			res = f_read(&cal_file_raw, &battery, sizeof(battery), &wt);
			if (res == FR_OK && wt == sizeof(battery))
			{
				sum += battery;
				num_values++;
				//DEBUG("Battery cal: num_values=%u, val=%u sum=%lu\n", num_values, battery, sum);
			}
		}

		if (num_values == 0)
		{
			error = true;
			break;
		}

		battery = sum / num_values;

		DEBUG("%d,%u\n", i, battery);
		
		ee_update_word(&config_ro.bat_calibration[i], battery);
	}

	f_close(&cal_file_raw);
	f_unlink(BAT_CAL_FILE_RAW);

	DEBUG("---End---\n");
	DEBUG("resort_bat_cal error: %d\n", (int)error);

	if (!error)
	{
        gui_showmessage_P(PSTR("Battery\nCalibration\nFinished."));
    	
    	ee_update_word(&config_ro.bat_runtime_minutes, num);

		bat_cal_available = true;
	}
	else
	{
        gui_showmessage_P(PSTR("Battery\nCalibration\nError."));
    	
        uint16_t tmp = BATTERY_CAL_INVALID;
    	ee_update_word(&config_ro.bat_runtime_minutes, tmp);
	}
}

void battery_abort_calibration()
{
	f_unlink(BAT_CAL_FILE_RAW);
	battery_calibrating_state = BATTERY_CAL_NONE;
	gui_showmessage_P(PSTR("Battery\nCalibration\nAborted."));
	DEBUG("Battery Cal.\nAborted!");
}

/**
 * This is the main function for battery calibration. If uses battery_calibrating_state to keep
 * track of where the operation is.
 */
void battery_calibration_step()
{
	#define BAT_CAL_MESSAGE_DURATION 5

	static uint32_t battery_calibrating_next_message = 0;
	static uint32_t battery_calibrating_next_cal = 0;

	switch (battery_calibrating_state)
	{
		case BATTERY_CAL_NONE:
			break;

		case BATTERY_CAL_START:
			DEBUG("Battery Cal: Start\n");

			if (battery_per == BATTERY_FULL)
			{
				f_unlink(BAT_CAL_FILE_RAW);

				battery_calibrating_state = BATTERY_CAL_DISCHARGE;
				battery_calibrating_next_message = 0;

				//force on gps, bt and full backlight to increase the battery drain
				//temporary settings are not stored to eeprom
				if (!bt_ready())
				{
					bt_module_init();
					config.connectivity.use_bt = true;
				}

				if (!gps_selftest())
				{
					gps_start();
					config.connectivity.use_gps = true;
				}

				config.gui.brightness = 100;
				config.gui.brightness_timeout = 30;
			}
			else
			{
				gui_showmessage_P(PSTR("Please plug\nUSB and wait\nfor full charge!"));
				battery_calibrating_state = BATTERY_CAL_NONE;
			}
			break;

		case BATTERY_CAL_DISCHARGE:
			switch (battery_per)
			{
				case BATTERY_CHARGING:
					// BAD: The user has plugged in charging in the middle of the discharge. Abort
					battery_calibrating_state = BATTERY_CAL_STOP;
					break;

				case BATTERY_FULL:
					battery_calibrating_next_cal = 0;

					if (task_get_ms_tick() > battery_calibrating_next_message)
					{
						battery_calibrating_next_message = task_get_ms_tick() + BAT_CAL_REMIND_INTERVAL * 1000L;
						gui_showmessage_P(PSTR("Please unplug\nUSB and wait\nfor full discharge!"));
						DEBUG("Battery Cal.\nUnplug USB!");
					}
					break;

				default:
					//do not turn off backlight during discharge
					gui_trigger_backlight();

					if (task_get_ms_tick() > battery_calibrating_next_cal)
					{
						battery_calibrating_next_cal = task_get_ms_tick() + BAT_CAL_INTERVAL * 1000L;
						save_bat_cal_value(battery_adc_raw);
					}
					break;
			}
			break;

		case BATTERY_CAL_STOP:
			battery_abort_calibration();
			break;
	}
}

void battery_stop()
{
	if (battery_calibrating_state != BATTERY_CAL_NONE)
		battery_abort_calibration();
}

bool battery_step()
{
	#ifdef FAKE_ENABLE
		return false;
	#endif

	battery_calibration_step();

	if (battery_next_meas > task_get_ms_tick())
		return false;

	if (USB_CONNECTED)
	{
		/* As we are charging, the bat_next_level is not valid anymore */
		bat_next_level = 0;

		if (BAT_CHARGING)
		{
			if (bat_charge_discharge_ratio > 1)
				bat_charge_discharge_ratio--;
		}
		else
		{
			if (bat_charge_discharge_ratio <= 500)
				bat_charge_discharge_ratio += 5;
		}

//		DEBUG("BAT c/d %u\n", bat_charge_discharge_ratio);

		if (bat_charge_discharge_ratio > 500)
		{
			battery_per = BATTERY_FULL;
			battery_reset_calibration();
		}

		if (bat_charge_discharge_ratio < 10)
		{
			battery_per = BATTERY_CHARGING;
		}

		return true;
	}

	if (bat_charge_discharge_ratio != 0)
	{
		bat_charge_discharge_ratio = 0;
		battery_force_update();
	}

	switch (battery_meas_state)
	{
	case(BATTERY_STATE_IDLE):
		//read adc value
		battery_adc_raw = battery_meas_acc / BATTERY_MEAS_AVG;

		if (bat_adc_max < battery_adc_raw)
		{
			bat_adc_max = battery_adc_raw;
			bat_adc_dif = bat_adc_max - BAT_ADC_MIN;

			DEBUG("Updating max ADC value to %u\n", bat_adc_max);

			
			ee_update_word(&config_ro.bat_adc_max, bat_adc_max);
		}

		if (bat_cal_available)
			//if callibration table is avalible
			battery_per = get_battery_per_from_calibration(battery_adc_raw);
		else
			//else use old linear method
			battery_per = (((int32_t)battery_adc_raw - (int32_t)BAT_ADC_MIN) * 105) / bat_adc_dif; //5% is planned overshoot

		if (battery_per > 100)
			battery_per = 100;
		if (battery_per < 0)
			battery_per = 0;

//		DEBUG("BAT val %lu %u %u\n", task_get_ms_tick(), battery_per, battery_adc_raw);

		battery_meas_state = BATTERY_STATE_PREPARE;
		battery_next_meas = task_get_ms_tick() + BATTERY_MEAS_PERIOD;
		battery_meas_acc = 0;
		battery_meas_cnt = 0;

		return true;
	break;

	case(BATTERY_STATE_PREPARE):
		battery_meas_state = BATTERY_STATE_START;

		BATTERY_ADC_PWR_ON;
		BATTERY_ADC_ENABLE;
		bat_en_high(BAT_EN_ADC);
		battery_next_meas = task_get_ms_tick() + BATTERY_STABILISE;
	break;

	case(BATTERY_STATE_START):
		AdcPipeStart(pipea0);

		battery_meas_state = BATTERY_STATE_RESULT;
	break;

	case(BATTERY_STATE_RESULT):
		if (!AdcPipeReady(pipea0))
		{
			DEBUG("adc not ready\n");
			return false;
		}

		uint16_t tmp = AdcPipeValue(pipea0);
		battery_meas_acc += tmp;

		battery_meas_cnt++;

		if (battery_meas_cnt >= BATTERY_MEAS_AVG)
		{
			battery_meas_state = BATTERY_STATE_IDLE;

			bat_en_low(BAT_EN_ADC);
			BATTERY_ADC_DISABLE;
			BATTERY_ADC_PWR_OFF;
		}
		else
			battery_meas_state = BATTERY_STATE_START;
	break;
	}

	return false;
}
