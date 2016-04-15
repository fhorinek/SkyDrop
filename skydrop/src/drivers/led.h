/*
 * led.h
 *
 *  Created on: 23.7.2014
 *      Author: horinek
 */

#ifndef LED_H_
#define LED_H_

#include "../skydrop.h"

void led_init();
void led_stop();
void led_set(uint16_t red, uint16_t green, uint16_t blue);

void lcd_bckl(uint8_t val);

void led_notify_enable();
void led_notify_disable();

#endif /* LED_H_ */
