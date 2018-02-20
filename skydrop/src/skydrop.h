/*
 * skydrop.h
 *
 *  Created on: 17.2.2014
 *      Author: horinek
 */

#ifndef SKYDROP_H_
#define SKYDROP_H_

#include "common.h"

#include "fc/conf.h"

#include "drivers/led.h"
#include "drivers/uart.h"
#include "drivers/time.h"
#include "drivers/battery.h"
#include "drivers/buttons.h"

#include "drivers/audio/audio.h"
#include "drivers/audio/buzzer.h"

#include "drivers/storage/storage.h"

#include "drivers/bluetooth/bt.h"

#include "drivers/sensors/devices.h"


#include "tasks/tasks.h"

extern uint8_t system_rst;

void Setup();
void Post();

#endif /* SKYDROP_H_ */
