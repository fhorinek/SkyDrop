/*
 * bt.h
 *
 *  Created on: 8.7.2014
 *      Author: horinek
 */

#ifndef BT_H_
#define BT_H_

#include "../../common.h"
#include "../uart.h"
#include "../../tasks/tasks.h"

//#include "btcomm.h"
#include "pan_lite.h"

bool bt_init();
void bt_step();
bool bt_device_active();
void bt_ms_irq();

#endif /* BT_H_ */
