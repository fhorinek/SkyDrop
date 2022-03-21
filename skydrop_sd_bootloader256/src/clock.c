#include "clock.h"
#include "common.h"

void osc_init()
{
	OSC.CTRL |= OSC_RC32MEN_bm;

	while (!(OSC.STATUS & OSC_RC32MRDY_bm));

	CCPIOWrite(&CLK.CTRL, CLK_SCLKSEL_RC32M_gc);
//	CCPIOWrite(&CLK.PSCTRL, 0b00000100);
}

void osc_deinit()
{
	CCPIOWrite(&CLK.CTRL, CLK_SCLKSEL_RC2M_gc);
	OSC.CTRL = OSC_RC2MEN_bm;

	CCPIOWrite(&CLK.PSCTRL, 0);
}
