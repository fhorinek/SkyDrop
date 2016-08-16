#ifndef SYSTEM_H_
#define SYSTEM_H_

#include "../common.h"
#include "gpio.h"

#define SystemReset()		CCPIOWrite(&RST.CTRL, RST_SWRST_bm)

void SystemPowerDown();
void SystemPowerSave();
void SystemPowerStandby();
void SystemPowerEStandby();
void SystemPowerIdle();


#endif /* SYSTEM_H_ */
