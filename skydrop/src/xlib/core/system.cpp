#ifndef SYSTEM_CC_
#define SYSTEM_CC_

#include "system.h"

void SystemPowerIdle()
{
	SLEEP.CTRL = SLEEP_SMODE_IDLE_gc | SLEEP_SEN_bm;
	__asm__ ("sleep");
	SLEEP.CTRL = 0x0;
}

void SystemPowerDown()
{
	SLEEP.CTRL = SLEEP_SMODE_PDOWN_gc | SLEEP_SEN_bm;
	__asm__ ("sleep");
	SLEEP.CTRL = 0x0;
}

void SystemPowerSave()
{
	SLEEP.CTRL = SLEEP_SMODE_PSAVE_gc | SLEEP_SEN_bm;
	__asm__ ("sleep");
	SLEEP.CTRL = 0x0;
}

void SystemPowerStandby()
{
	SLEEP.CTRL = SLEEP_SMODE_STDBY_gc | SLEEP_SEN_bm;
	__asm__ ("sleep");
	SLEEP.CTRL = 0x0;
}

void SystemPowerEStandby()
{
	SLEEP.CTRL = SLEEP_SMODE_ESTDBY_gc | SLEEP_SEN_bm;
	__asm__ ("sleep");
	SLEEP.CTRL = 0x0;
}


#endif /* SYSTEM_CC_ */
