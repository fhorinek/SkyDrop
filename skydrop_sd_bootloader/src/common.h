#ifndef COMMON_H_
#define COMMON_H_

#include <avr/io.h>
#include <util/delay.h>

#define VERSION		0x0001

#define USART_USART		USARTC0
#define USART_PORT		PORTC
#define WAIT_TIME		100
#define UART_TX_PIN		3

#define LED_OFF			PORTD.OUTSET = 0b00110000;

#define LED_GREEN		PORTD.OUTCLR = 0b00010000;

#define LED_RED			PORTD.OUTCLR = 0b00100000;

#define LED_INIT		PORTD.DIRSET = 0b00110000;\
						PORTA.DIRSET = 0b01000000;\
						PORTA.OUTSET = 0b01000000;

#define LED_DEINIT		PORTD.DIRCLR = 0b00110000;\
						PORTA.DIRCLR = 0b01000000;

#define SD_SS_HI		PORTR.OUTSET = 0b00000010;
#define SD_SS_LO		PORTR.OUTCLR = 0b00000010;


void CCPIOWrite( volatile uint8_t * address, uint8_t value );

#endif /* COMMON_H_ */
