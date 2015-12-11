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
#define BT_IRQ_INIT_OK		3
#define BT_IRQ_INIT			4
#define BT_IRQ_DEINIT		5
#define BT_IRQ_PAIR			6
#define BT_IRQ_INIT_FAIL	7
#define BT_IRQ_RESET		8

#define BT_PAN1322	0
#define BT_PAN1026	1
#define BT_UNKNOWN	0xFF

void bt_init();
void bt_stop();
void bt_step();

void bt_module_reset();
void bt_module_init();
void bt_module_deinit();
bool bt_device_active();

void bt_send(char * str);
void bt_irgh(uint8_t type, uint8_t * buf);

bool bt_selftest();
uint8_t bt_get_module_type();

#define BT_MOD_STATE_OFF	0
#define BT_MOD_STATE_INIT	1
#define BT_MOD_STATE_OK		2

extern volatile uint8_t bt_module_state;

#endif /* BT_H_ */
