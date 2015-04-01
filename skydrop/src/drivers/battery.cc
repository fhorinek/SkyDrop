#include "battery.h"
#include "../tasks/tasks.h"


void battery_init()
{
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
#define BATTERY_MEAS_PERIOD 	20
//#define BATTERY_MEAS_PERIOD 	1

uint32_t battery_next_meas = 0;
uint8_t  battery_meas_state = BATTERY_STATE_PREPARE;

uint16_t battery_meas_acc = 0;
uint8_t  battery_meas_cnt = 0;

int16_t battery_adc_raw = 0;
int8_t battery_per = 0;

#define BATT_COEF_A	(0.4245490451)
#define BATT_COEF_B  (-690.3297755603)

bool battery_step()
{
	if (battery_next_meas > time_get_actual())
		return false;

	switch (battery_meas_state)
	{
	case(BATTERY_STATE_IDLE):
		battery_adc_raw = battery_meas_acc / BATTERY_MEAS_AVG;
		battery_per = round((battery_adc_raw * BATT_COEF_A) + BATT_COEF_B);
		if (battery_per > 100)
			battery_per = 100;
		if (battery_per < 0)
			battery_per = 0;

		task_irqh(TASK_IRQ_BAT, (uint8_t *)&battery_per);

		DEBUG1("adc %u (%3d%%)", battery_adc_raw, battery_per);

		battery_meas_state = BATTERY_STATE_PREPARE;
		battery_next_meas = time_get_actual() + BATTERY_MEAS_PERIOD;
		battery_meas_acc = 0;
		battery_meas_cnt = 0;

		return true;
	break;

	case(BATTERY_STATE_PREPARE):
		battery_meas_state = BATTERY_STATE_START;

		BATTERY_ADC_PWR_ON;
		BATTERY_ADC_ENABLE;
		bat_en_high(BAT_EN_ADC);
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

		bat_en_low(BAT_EN_ADC);

		uint16_t tmp = AdcPipeValue(pipea0);
		battery_meas_acc += tmp;

		battery_meas_cnt++;

		if (battery_meas_cnt >= BATTERY_MEAS_AVG)
		{
			battery_meas_state = BATTERY_STATE_IDLE;

			BATTERY_ADC_DISABLE;
			BATTERY_ADC_PWR_OFF;
		}
		else
			battery_meas_state = BATTERY_STATE_START;
	break;
	}

	return false;
}
