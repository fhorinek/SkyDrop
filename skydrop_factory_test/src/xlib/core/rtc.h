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

/**
 * Available RTC clock sources
 */
typedef enum xlib_core_rtc_source_e
{
	rtc_1000Hz_ulp,		//!< 1 kHz from internal 32 kHz ULP
	rtc_1024Hz_tosc,	//!< 1.024 kHz from 32.768 kHz Crystal Oscillator on TOSC
	rtc_1024Hz_rc,		//!< 1.024 kHz from internal 32.768 kHz RC Oscillator
	rtc_32kHz_tosc,		//!< 32.768 kHz from 32.768 kHz Crystal Oscillator on TOSC
} xlib_core_rtc_source;

/**
 * Available RTC clock sources
 */
typedef enum xlib_core_rtc_prescaler_e
{
	rtc_off 	= 0b000,
	rtc_div1 	= 0b001,
	rtc_div2 	= 0b010,
	rtc_div8 	= 0b011,
	rtc_div16 	= 0b100,
	rtc_div64 	= 0b101,
	rtc_div256 	= 0b110,
	rtc_div1024 = 0b111,

} xlib_core_rtc_prescaler;

#define rtc_overflow	0b01
#define rtc_compare		0b10

#define rtc_overflow_interrupt	RTC_OVF_vect
#define rtc_compare_interrupt	RTC_COMP_vect

void RtcInit(xlib_core_rtc_source source, xlib_core_rtc_prescaler pre);
void RtcEnableInterrupts(uint8_t flags);
void RtcDisableInterrupts(uint8_t flags);
void RtcSetPeriod(uint16_t period);
void RtcSetCompare(uint16_t cmp);
uint16_t RtcGetValue();
void RtcSetValue(uint16_t val);

#endif /* RTC_H_ */
