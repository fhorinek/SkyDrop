/*
 * rtc.h
 *
 *  Created on: 22.11.2013
 *      Author: horinek
 */

#ifndef RTC_H_
#define RTC_H_

#include "../common.h"
#include "clock.h"

#define rtc_overflow_interrupt	RTC_OVF_vect

void RtcInit();
void RtcEnableInterrupts();
void RtcSetPeriod(uint16_t period);
uint16_t RtcGetValue();

#endif /* RTC_H_ */
