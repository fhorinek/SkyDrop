
#include <xlib/core/rtc32.h>
#include "debug.h"
#include "debug_on.h"

#include "drivers/x256a3bu_support.h"

void Rtc32Init()
{
	while (RTC32.SYNCCTRL & RTC32_SYNCBUSY_bm);

	CCPIOWrite(&VBAT.CTRL, VBAT_RESET_bm);
	VBAT.CTRL = VBAT_ACCEN_bm;
	_delay_us(200);

	VBAT.CTRL |= VBAT_XOSCEN_bm | VBAT_XOSCSEL_bm;
	while (!(VBAT.STATUS & VBAT_XOSCRDY_bm));

	//activate RTC
	RTC32.CTRL |= RTC32_ENABLE_bm;
}

void Rtc32EnableInterrupts()
{
	RTC32.INTFLAGS |=  RTC32_OVFIF_bm;
	RTC32.INTCTRL |= PMIC_LOLVLEN_bm; // LO
}

void Rtc32SetPeriod(uint32_t period)
{
	RTC32.CTRL &= ~RTC32_ENABLE_bm;
	while (RTC32.SYNCCTRL & RTC32_SYNCBUSY_bm);

	RTC32.PER = period;

	RTC32.CTRL |= RTC32_ENABLE_bm;
	while (RTC32.SYNCCTRL & RTC32_SYNCBUSY_bm);
}


uint32_t Rtc32GetValue()
{
	RTC32.CTRL |= RTC32_SYNCCNT_bm;
	while (RTC32.SYNCCTRL & RTC32_SYNCBUSY_bm);
	return RTC32.CNT;
}

