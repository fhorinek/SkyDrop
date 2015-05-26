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

#define BT_IRQ_CONNECTED	0
#define BT_IRQ_DISCONNECTED	1
#define BT_IRQ_ERROR		2

bool bt_init();
void bt_step();

void bt_send(char * str);
void bt_irgh(uint8_t type, uint8_t * buf);

#endif /* BT_H_ */
