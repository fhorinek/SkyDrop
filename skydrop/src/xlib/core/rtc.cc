
#include "rtc.h"

void RtcInit(xlib_core_rtc_source source, xlib_core_rtc_prescaler pre)
{
	while (RTC.STATUS & RTC_SYNCBUSY_bm);

	switch (source)
	{
		case(rtc_1000Hz_ulp):
			CLK.RTCCTRL |= CLK_RTCSRC_ULP_gc | CLK_RTCEN_bm;
			break;
		case(rtc_1024Hz_tosc):
			OSC.XOSCCTRL = OSC_XOSCSEL_32KHz_gc | OSC_X32KLPM_bm;
			OSC.CTRL |= OSC_XOSCEN_bm;
			while (!(OSC.STATUS & OSC_XOSCRDY_bm));
			CLK.RTCCTRL |= CLK_RTCSRC_TOSC_gc | CLK_RTCEN_bm;
			break;
		case(rtc_1024Hz_rc):
			OSC.CTRL |= OSC_RC32KEN_bm;					//enable source
			while (!(OSC.STATUS & OSC_RC32KRDY_bm));	//stabilize
			CLK.RTCCTRL |= CLK_RTCSRC_RCOSC_gc | CLK_RTCEN_bm;
			break;
		case(rtc_32kHz_tosc):
			OSC.XOSCCTRL = OSC_XOSCSEL_32KHz_gc;// LP mode -> | OSC_X32KLPM_bm;
			OSC.CTRL |= OSC_XOSCEN_bm;
			while (!(OSC.STATUS & OSC_XOSCRDY_bm));
			CLK.RTCCTRL |= CLK_RTCSRC_TOSC32_gc | CLK_RTCEN_bm;
			break;
	}
	//activate RTC
	RTC.CTRL |= pre;
}

void RtcEnableInterrupts(uint8_t flags)
{
	if (flags & rtc_overflow)
	{
		RTC.INTFLAGS |=  RTC_OVFIF_bm;
		RTC.INTCTRL |= PMIC_LOLVLEN_bm; // LO
	}
	if (flags & rtc_compare)
	{
		RTC.INTFLAGS |=  RTC_COMPIF_bm;
		RTC.INTCTRL |= (PMIC_LOLVLEN_bm << 2); // LO
	}
}


void RtcDisableInterrupts(uint8_t flags)
{
	if (flags & rtc_overflow)
		RTC.INTCTRL &= ~PMIC_LOLVLEN_bm; // LO
	if (flags & rtc_compare)
		RTC.INTCTRL &= ~(PMIC_LOLVLEN_bm << 2); // LO
}


void RtcSetPeriod(uint16_t period)
{
	while (RTC.STATUS & RTC_SYNCBUSY_bm);
	RTC.PER = period;
}

void RtcSetCompare(uint16_t cmp)
{
	while (RTC.STATUS & RTC_SYNCBUSY_bm);
	RTC.COMP = cmp;
}

uint16_t RtcGetValue()
{
	return RTC.CNT;
}

void RtcSetValue(uint16_t val)
{
	RTC.CNT = val;
}

