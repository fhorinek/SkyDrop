#ifndef SYSTEM_CC_
#define SYSTEM_CC_

#include "system.h"

//wake up sunshine
ISR(wake_interrupt)
{
	GpioSetInterrupt(user_button, gpio_clear);
}

void SystemInit()
{
//  //bt reset default HIGH (bt in reset)
//  GpioSetInvert(bt_reset,  ON);
//	GpioWrite(bt_reset, HIGH);
//	GpioSetDirection(bt_reset, OUTPUT);
//
//	//bt shdn default HIGH (nrf off)
//	GpioSetInvert(bt_shdn,  ON);
//	GpioWrite(bt_shdn, HIGH);
//	GpioSetDirection(bt_shdn, OUTPUT);
//
//	//button setup (High if pressed)
//	GpioSetPull(user_button, gpio_pull_up);
//	GpioSetInvert(user_button, ON);
//	GpioSetDirection(user_button, INPUT);
}

void SystemPowerIdle()
{
	SLEEP.CTRL = SLEEP_SMODE_IDLE_gc | SLEEP_SEN_bm;
	//GpioSetInterrupt(user_button, gpio_interrupt1, gpio_falling);
	__asm__ ("sleep");
	SLEEP.CTRL = 0x0;
}

void SystemPowerDown()
{
	SLEEP.CTRL = SLEEP_SMODE_PDOWN_gc | SLEEP_SEN_bm;
	//GpioSetInterrupt(user_button, gpio_interrupt1, gpio_falling);
	__asm__ ("sleep");
	SLEEP.CTRL = 0x0;
}

void SystemPowerSave()
{
	SLEEP.CTRL = SLEEP_SMODE_PSAVE_gc | SLEEP_SEN_bm;
	//GpioSetInterrupt(user_button, gpio_interrupt1, gpio_falling);
	__asm__ ("sleep");
	SLEEP.CTRL = 0x0;
}

void SystemPowerStandby()
{
	SLEEP.CTRL = SLEEP_SMODE_STDBY_gc | SLEEP_SEN_bm;
	//GpioSetInterrupt(user_button, gpio_interrupt1, gpio_falling);
	__asm__ ("sleep");
	SLEEP.CTRL = 0x0;
}

void SystemPowerEStandby()
{
	SLEEP.CTRL = SLEEP_SMODE_ESTDBY_gc | SLEEP_SEN_bm;
	//GpioSetInterrupt(user_button, gpio_interrupt1, gpio_falling);
	__asm__ ("sleep");
	SLEEP.CTRL = 0x0;
}


#endif /* SYSTEM_CC_ */
