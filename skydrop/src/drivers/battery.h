/*
 * battery.h
 *
 *  Created on: 5.3.2014
 *      Author: horinek
 */

#ifndef BATTERY_H_
#define BATTERY_H_


#include "../skydrop.h"

//854*x + 399
#define VDC2ADC(V)	(854 * V + 399)
#define ADC2VDC(V)	((V - 399) / 854.0)




void battery_init();
bool battery_step();

#endif /* BATTERY_H_ */
