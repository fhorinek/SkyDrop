/*
 * usb.h
 *
 *  Created on: 12.5.2015
 *      Author: horinek
 */

#ifndef USB_H_
#define USB_H_

#include "gui.h"

void gui_usb_init();
void gui_usb_stop();
void gui_usb_loop();
void gui_usb_irqh(uint8_t type, uint8_t * buff);

#endif /* usb_H_ */
