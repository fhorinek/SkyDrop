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
#define BATTERY_CAL_START     (1)
#define BATTERY_CAL_DISCHARGE (2)
#define BATTERY_CAL_STOP      (3)

#define BATTERY_CAL_INVALID		0xFFFF

#define BAT_CAL_FILE_RAW "/BAT-CAL.RAW"     // file to store raw calibration data collected during calibration

void battery_init();
bool battery_step();
void battery_force_update();
void battery_stop();
void battery_finish_calibration();

/**
 * Return the number of minutes, that the battery runs, if full.
 *
 * @return number of minutes for full capacity
 */
uint16_t battery_runtime_minutes();

/**
 * Check, if we have calibration data for the battery in EEPROM.
 *
 * @return true if data is available, false otherwise.
 */
bool battery_calibrated();

#endif /* BATTERY_H_ */
