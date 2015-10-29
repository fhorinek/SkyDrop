#include "battery.h"
#include "../tasks/tasks.h"


void battery_init()
{
	BATTERY_ADC_PWR_ON;
	BATTERY_ADC_ENABLE;

	AdcAInit(adc_int1V);
	AdcASetMode(adc_unsigned);

	AdcPipeSetSource(pipea0, BAT_SNS_ADC);

	GpioSetDirection(BAT_EN, OUTPUT);
	bat_en_low(BAT_EN_ADC);
}


#define BATT_THOLD	VDC2ADC(2.2)

#define BATTERY_STATE_IDLE		0
#define BATTERY_STATE_PREPARE	1
#define BATTERY_STATE_START		2
#define BATTERY_STATE_RESULT	3

#define BATTERY_MEAS_AVG 		16
#define BATTERY_MEAS_PERIOD 	10000
//#define BATTERY_MEAS_PERIOD 	1000
#define BATTERY_STABILISE 		100


uint32_t battery_next_meas = 0;
uint8_t  battery_meas_state = BATTERY_STATE_PREPARE;

uint16_t battery_meas_acc = 0;
uint8_t  battery_meas_cnt = 0;

int16_t battery_adc_raw = 0;
int8_t battery_per = 0;

//#define BATT_COEF_A	(0.291950711)
//#define BATT_COEF_B  (-672.1273455619)


#define BATT_COEF_A	(0.2147782473)
#define BATT_COEF_B  (-681.4132446547)

uint8_t battery_get_per()
{
	return battery_per;
}

bool battery_step()
{
	if (battery_next_meas > task_get_ms_tick())
		return false;

	switch (battery_meas_state)
	{
	case(BATTERY_STATE_IDLE):
		battery_adc_raw = battery_meas_acc / BATTERY_MEAS_AVG;
		battery_per = round(((float)battery_adc_raw * BATT_COEF_A) + BATT_COEF_B);
		if (battery_per > 100)
			battery_per = 100;
		if (battery_per < 0)
			battery_per = 0;

//		DEBUG("adc %u (%3d%%)\n", battery_adc_raw, battery_per);

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
