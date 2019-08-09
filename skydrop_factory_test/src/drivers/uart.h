/*
 * uart.h
 *
 *  Created on: 23.7.2014
 *      Author: horinek
 */

#ifndef UART_H_
#define UART_H_

#include "../common.h"
#include <xlib/core/usart.h>

void uart_init();
void uart_low_speed();
void uart_stop();

void uart_send(char * msg);
void uart_send(uint16_t len, uint8_t * data);


void DUMP_REG(uint8_t val);

extern Usart uart;

#endif /* UART_H_ */
