/*
 * battery.h
 *
 *  Created on: 5.3.2014
 *      Author: horinek
 */

#ifndef BATTERY_H_
#define BATTERY_H_


#include "../skydrop.h"

extern int16_t battery_adc_raw;
extern int8_t battery_per;
extern uint32_t battery_next_meas;

#define BATTERY_CHARGING	(101)
#define BATTERY_FULL		(102)

void battery_init();
bool battery_step();
void battery_update();

#endif /* BATTERY_H_ */
