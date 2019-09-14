/*
 * battery.h
 *
 *  Created on: 5.3.2014
 *      Author: horinek
 */

#ifndef BATTERY_H_
#define BATTERY_H_


#include "../skydrop.h"

extern uint16_t battery_adc_raw;
extern int8_t battery_per;
extern uint32_t battery_next_meas;

extern uint16_t bat_adc_max;

extern uint8_t battery_calibrating_state;

#define BATTERY_CHARGING	(101)
#define BATTERY_FULL		(102)

#define BATTERY_CAL_NONE      (0)
#define BATTERY_CAL_BOOT      (1)
#define BATTERY_CAL_START     (2)
#define BATTERY_CAL_CHARGE    (3)
#define BATTERY_CAL_DISCHARGE (4)
#define BATTERY_CAL_STOP      (5)

void battery_init();
bool battery_step();
void battery_force_update();

/**
 * Return the number of minutes, that the battery runs, if full.
 *
 * @return number of minutes for full capacity
 */
uint16_t bat_runtime_minutes();

/**
 * Check, if we have calibration data for the battery in EEPROM.
 *
 * @return true if data is available, false otherwise.
 */
bool bat_calibrated();

#endif /* BATTERY_H_ */
