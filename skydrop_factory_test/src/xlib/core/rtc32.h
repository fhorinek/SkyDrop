/*
 * rtc.h
 *
 *  Created on: 22.11.2013
 *      Author: horinek
 */

#ifndef RTC32_H_
#define RTC32_H_

#include "../common.h"
#include "clock.h"

void Rtc32Init();
void Rtc32EnableInterrupts();
void Rtc32SetPeriod(uint32_t period);
uint32_t Rtc32GetValue();

#endif /* RTC32_H_ */
