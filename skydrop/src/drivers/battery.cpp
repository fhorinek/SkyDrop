#include "battery.h"
#include "../tasks/tasks.h"

// #include "../debug_on.h"

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

#define BAT_CAL_REMIND_INTERVAL 10          // Interval in seconds to remind user of wrong charging/discharging
#define BAT_CAL_INTERVAL 60                 // Interval in seconds to save calibration data

#define BAT_CAL_FILE_RAW "/BAT-CAL.RAW"     // file to store raw calibration data collected during callibration

/** This is the ADC value where the next lower percent value starts. */
uint16_t bat_next_level = 0;

/** Do we have calibration data during normal operation. */
bool bat_cal_available = false;

/**
 * This is a state machine for battery calibration:
 *
 * BATTERY_CAL_BOOT: System is booting, check for BAT_CAL_FILE_RAW and work with it.
 *                   Then go to BATTERY_CAL_NONE
 * BATTERY_CAL_NONE: No calibration is running, normal operation.
 * BATTERY_CAL_START: A battery calibration is starting. Inform user of what is going on. Then go to
 * BATTERY_CAL_CHARGE: wait until battery is fully charged.
 * BATTERY_CAL_DISCHARGE: wait until battery is fully discharged, store calibration data every BAT_CAL_INTERVAL.
 */
uint8_t battery_calibrating_state = BATTERY_CAL_BOOT;

/**
 * Check, if we have calibration data for the battery in EEPROM.
 *
 * @return true if data is available, false otherwise.
 */
bool bat_calibrated()
{
	uint16_t value;

	eeprom_busy_wait();
	value = eeprom_read_word(&config_ro.bat_runtime_minutes);
	DEBUG("config_ro.bat_runtime_minutes=%u\n", value);
	return ( value != 0xffff );
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

	if ( !bat_calibrated() )
		return 101;                  // No calibration available

	for (percent = 100; percent > 0; percent-- )
	{
		eeprom_busy_wait();
		battery = eeprom_read_word(&config_ro.bat_calibration[100-percent]);
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
 * @return the corresponding percent value or 101 for error
 */
int8_t get_battery_per_from_calibration(uint16_t battery_adc_raw)
{
	uint8_t percent;

	/* Check, if we have a calibration available: */
	if ( bat_cal_available )
	{
		/* Check, if we reached the next level. If not return identical percent */
		if ( bat_next_level != 0 && battery_adc_raw > bat_next_level )
			return battery_per;

		percent = read_battery_per_from_calibration(battery_adc_raw);
		DEBUG("get_battery_per_from_calibration(%d)=%d ", battery_adc_raw, percent);
		print_datetime(time_get_local());
	}
	else
	{
		percent = 101;
	}

	return percent;
}

void battery_reset_calibration()
{
	if (bat_adc_max == BAT_ADC_MIN)
		return;

	bat_adc_max = BAT_ADC_MIN;

	DEBUG("Resetting max ADC value\n");

	eeprom_busy_wait();
	eeprom_update_word(&config_ro.bat_adc_max, bat_adc_max);
}

void battery_init()
{
	eeprom_busy_wait();
	bat_adc_max = eeprom_read_word(&config_ro.bat_adc_max);

	if (bat_adc_max > 4095)
		battery_reset_calibration();

	bat_adc_dif = bat_adc_max - BAT_ADC_MIN;

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
void resort_bat_cal()
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
	/* This can be saved in eeprom to store the number of minutes, the battery lasts. */
	num = f_size(&cal_file_raw) / sizeof(uint16_t);

	for ( int i = 0; i < 100; i++ )
	{
		int32_t start = (int32_t)num * i / 100;
		res = f_lseek(&cal_file_raw, (DWORD)start * sizeof(uint16_t));
		if (res != FR_OK)
		{
			DEBUG("resort_bat_cal: f_lseek failed\n");
			break;
		}
		int32_t sum = 0;
		int16_t num_values = 0;
		for (unsigned int j = 0; j < f_size(&cal_file_raw) / 100; j++ )
		{
			res = f_read(&cal_file_raw, &battery, sizeof(battery), &wt);
			if ( res == FR_OK && wt == sizeof(battery) )
			{
				sum += battery;
				num_values++;
				// DEBUG("Battery cal: num_values=%d, sum=%ld\n", num_values, sum);
			}
		}
		if ( num_values == 0 )
		{
			error = true;
			break;
		}
		battery = sum / num_values;

		DEBUG("Battery resort_bat_cal: bat_calibration[%d]=%u\n", i, battery);
		eeprom_busy_wait();
		eeprom_write_word(&config_ro.bat_calibration[i], battery);
	}

	eeprom_busy_wait();
	eeprom_write_word(&config_ro.bat_runtime_minutes, num);

	f_close(&cal_file_raw);

	DEBUG("resort_bat_cal error: %d\n", (int)error);

	if ( !error)
	{
		f_unlink(BAT_CAL_FILE_RAW);
        gui_showmessage_P(PSTR("Battery Cal:\nFinished."));
	}
	else
	{
        gui_showmessage_P(PSTR("Battery Cal:\nError!\nPlease retry."));
	}
}

/**
 * This is the main function for battery calibration. If uses battery_calibrating_state to keep
 * track of where the operation is.
 */
void battery_cal()
{
	static uint8_t charge_counter = 0;
	static uint32_t battery_calibrating_next_message = 0;
	static uint32_t battery_calibrating_next_cal = 0;
	FILINFO fno;

#define BAT_CAL_MESSAGE_DURATION 5

	switch (battery_calibrating_state)
	{
	case BATTERY_CAL_BOOT:
		if (!storage_ready() || gui_task != GUI_PAGES)
			break;                 // wait for system to operate normally
		if (f_stat(BAT_CAL_FILE_RAW, &fno) == FR_OK)
			resort_bat_cal();      // check, if calibration raw is available
		if ( bat_calibrated() )
			bat_cal_available = true;

		battery_calibrating_state = BATTERY_CAL_NONE;
		break;
	case BATTERY_CAL_NONE:
		break;

	case BATTERY_CAL_START:
		DEBUG("Battery Cal: Start\n");
		charge_counter = 0;
		gui_showmessage_P(PSTR("Battery Cal:\nCharge to full,\nthen discharge."));
		gui_messageduration(BAT_CAL_MESSAGE_DURATION);
		battery_calibrating_next_message = task_get_ms_tick() + BAT_CAL_MESSAGE_DURATION * 1000L;
		battery_calibrating_state = BATTERY_CAL_CHARGE;
		break;

	case BATTERY_CAL_CHARGE:
		switch (battery_per) {
		case ( BATTERY_FULL ):
			// Battery is now full, go to DISCHARGE state
			battery_calibrating_state = BATTERY_CAL_DISCHARGE;
			DEBUG("Battery Cal: BATTERY FULL.\n");
			break;
		case BATTERY_CHARGING:
			charge_counter = 0;
			break;
		default:
			// We are discharging, which is wrong here. We first want to charge until BATTERY_FULL
			if ( task_get_ms_tick() > battery_calibrating_next_message)
			{
				battery_calibrating_next_message = task_get_ms_tick() + BAT_CAL_REMIND_INTERVAL * 1000L;
				charge_counter++;
				if ( charge_counter >= 4 )
				{
					gui_showmessage_P(PSTR("Battery Cal:\nCharge missing.\nNow discharge."));
					battery_calibrating_state = BATTERY_CAL_DISCHARGE;
					DEBUG("Battery Cal.\nunplugged...\n");
				}
				else
				{
					DEBUG("Battery Cal.\nPlease charge!\n");
					gui_showmessage_P(PSTR("Battery Cal:\nPlease charge!"));
				}
			}
			break;
		}
		break;

	case BATTERY_CAL_DISCHARGE:
		switch (battery_per) {
		case (BATTERY_CHARGING):
			// BAD: The user has plugged in charging in the middle of the discharge. Go back to full charge.
			f_unlink(BAT_CAL_FILE_RAW);
			battery_calibrating_state = BATTERY_CAL_CHARGE;
			DEBUG("Battery cal: recharged.\n");
			break;
		case BATTERY_FULL:
			battery_calibrating_next_cal = 0;
			f_unlink(BAT_CAL_FILE_RAW);
			if ( task_get_ms_tick() > battery_calibrating_next_message)
			{
				battery_calibrating_next_message = task_get_ms_tick() + BAT_CAL_REMIND_INTERVAL * 1000L;
				gui_showmessage_P(PSTR("Battery Cal.\nPlease\ndischarge!"));
				DEBUG("Battery Cal.\nPlease discharge!");
			}
			break;
		default:
			if ( task_get_ms_tick() > battery_calibrating_next_cal)
			{
				battery_calibrating_next_cal = task_get_ms_tick() + BAT_CAL_INTERVAL * 1000L;
				save_bat_cal_value(battery_adc_raw);
			}
			break;
		}
		break;
	case BATTERY_CAL_STOP:
		f_unlink(BAT_CAL_FILE_RAW);
		battery_calibrating_state = BATTERY_CAL_NONE;
		gui_showmessage_P(PSTR("Battery Cal.\nAborted."));
		DEBUG("Battery Cal.\nAborted!");
		break;
	}
}

bool battery_step()
{
	#ifdef FAKE_ENABLE
		return false;
	#endif

	battery_cal();

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

			eeprom_busy_wait();
			eeprom_update_word(&config_ro.bat_adc_max, bat_adc_max);
		}

		battery_per = get_battery_per_from_calibration(battery_adc_raw);

		if ( battery_per > 100 )
		{
			//5% is planned overshoot
			battery_per = (((int32_t)battery_adc_raw - (int32_t)BAT_ADC_MIN) * 105) / bat_adc_dif;
		}
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
