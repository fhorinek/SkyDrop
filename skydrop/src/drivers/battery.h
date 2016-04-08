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


void battery_init();
bool battery_step();

uint8_t battery_get_per();

#endif /* BATTERY_H_ */
