/*
 * task_usb.h
 *
 *  Created on: 6.10.2014
 *      Author: horinek
 */

#ifndef TASK_USB_H_
#define TASK_USB_H_

#include "../tasks.h"
#include "MassStorage/MassStorage.h"

void task_usb_init();
void task_usb_stop();
void task_usb_loop();
void task_usb_irqh(uint8_t type, uint8_t * buff);

extern uint8_t task_usb_sd_ready;

#endif /* TASK_USB_H_ */
