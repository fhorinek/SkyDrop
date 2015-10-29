#include "watchdog.h"

void wdt_init(wdt_timeout to)
{
	CCPIOWrite(&WDT.CTRL, 0b00000011 | to);
}

void wdt_deinit()
{
	CCPIOWrite(&WDT.CTRL, 0b00000001);
}
