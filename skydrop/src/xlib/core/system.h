#ifndef SYSTEM_H_
#define SYSTEM_H_

#include "../common.h"
#include "gpio.h"

#define SystemReset()		CCPIOWrite(&RST.CTRL, RST_SWRST_bm)

#define SystemPsuDetect()	GpioRead(psu_detect)
#define SystemPsuShdn(A)	GpioWrite(psu_shdn, A)

//#define SystemNrfReset(A)	GpioWrite(psu_reset, A)
//#define SystemNrfProg(A)	GpioWrite(psu_prog, A)
#define SystemButtonPress()	GpioRead(user_button)

//#define SystemNordicOn()	GpioWrite(nrf_reset, LOW)
//#define SystemNordicOff()	GpioWrite(nrf_reset, HIGH)

void SystemInit();
void SystemPowerDown();
void SystemPowerSave();
void SystemPowerStandby();
void SystemPowerEStandby();
void SystemPowerIdle();


#endif /* SYSTEM_H_ */
