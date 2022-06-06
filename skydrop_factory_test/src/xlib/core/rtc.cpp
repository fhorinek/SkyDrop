
#include "rtc.h"

void RtcInit()
{
	while (RTC.STATUS & RTC_SYNCBUSY_bm);

	OSC.XOSCCTRL = OSC_XOSCSEL_32KHz_gc;// LP mode -> | OSC_X32KLPM_bm;
	OSC.CTRL |= OSC_XOSCEN_bm;
	while (!(OSC.STATUS & OSC_XOSCRDY_bm));
	CLK.RTCCTRL |= CLK_RTCSRC_TOSC32_gc | CLK_RTCEN_bm;

	//activate RTC
	RTC.CTRL |= 0b001;
}

void RtcEnableInterrupts()
{
	RTC.INTFLAGS |=  RTC_OVFIF_bm;
	RTC.INTCTRL |= PMIC_LOLVLEN_bm; // LO
}

void RtcSetPeriod(uint16_t period)
{
	while (RTC.STATUS & RTC_SYNCBUSY_bm);
	RTC.PER = period;
}

uint16_t RtcGetValue()
{
	return RTC.CNT;
}
