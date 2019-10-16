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

#endif /* BATTERY_H_ */
