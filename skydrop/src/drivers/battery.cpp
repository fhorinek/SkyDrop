#include "battery.h"
#include "../tasks/tasks.h"

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

bool battery_step()
{
	#ifdef FAKE_ENABLE
		return false;
	#endif

	if (battery_next_meas > task_get_ms_tick())
		return false;

	if (USB_CONNECTED)
	{
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

		//5% is planned overshoot
		battery_per = (((int32_t)battery_adc_raw - (int32_t)BAT_ADC_MIN) * 105) / bat_adc_dif;
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
