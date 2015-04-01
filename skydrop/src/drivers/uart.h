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

//DEBUG
extern uint8_t debug_level;

#define DEBUG(format, ...) \
	do \
	if (debug_level > 0) \
	{ \
		printf_P(PSTR(format), ##__VA_ARGS__); \
		uart.FlushTxBuffer(); \
	} \
	while(0) \

//#define DEBUG1

#define DEBUG1(x...) \
	do \
	if (debug_level > 1) \
	{ \
		DEBUG("%S@%d: ", PSTR(__FILE__), __LINE__); \
		DEBUG(x); \
		DEBUG("\n"); \
		uart.FlushTxBuffer(); \
	} \
	while(0)


void uart_init_buffers();
void uart_init();
void uart_low_speed();
void uart_stop();


void DUMP_REG(uint8_t val);

extern Usart uart;

#endif /* UART_H_ */
