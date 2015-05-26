/*
 * usart.c
 *
 * Author: Panasonic
 */ 

// Include header files
#include <avr/io.h>
#include "usart.h"
#include "../Statemachine/statemachine.h"

// Function declaration
void USART0_Init (void) {	// Controller -> PC

	// Set baud rate 115200 at 8.0MHz
	UBRR0L = 8;	
	
	// Double speed USART
	UCSR0A = (1 << U2X0);
	
	// Enable transmission
	UCSR0B = (1 << TXEN0);
	
	// Asynchronous, 8N1 Mode
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);	
}

void USART1_Init (void) {	// Controller <-> PAN1026
	
	// Set baud rate 9600 resp. 115200 at 8.0MHz
	UBRR1L = 103;	// 9600 baud
	
	// Double speed USART
	UCSR1A = (1 << U2X1);
	
	// Enable transmission, reception and reception interrupt
	UCSR1B = (1 << TXEN1) | (1 << RXEN1) | (1 << RXCIE1);
	
	// Asynchronous, 8N1 Mode
	UCSR1C = (1 << UCSZ11) | (1 << UCSZ10);
}

void USART0_SendByte (unsigned char data) {
	// Wait for empty SendByte buffer
	while ( !(UCSR0A & (1 << UDRE0)) );
	
	// Put data into buffer, sends the data
	UDR0 = data;
}

void USART1_SendByte (unsigned char data) {
	// Wait for empty SendByte buffer
	while ( !(UCSR1A & (1 << UDRE1)) );
	
	// Put data into buffer, sends the data
	UDR1 = data;
}

void USART0_SendData (unsigned char data[], unsigned char length) {
	
	unsigned char i;
	
	for(i=0; i<length; i++) {
		USART0_SendByte(data[i]);
	}
}

void USART1_SendData (unsigned char data[], unsigned char length) {
	
	unsigned char i;
	
	for(i=0; i<length; i++) {
		USART1_SendByte(data[i]);
	}
}

void USARTS_SendData (unsigned char data[], unsigned char length) {
	
	// Only send a command once
	if(transmissionCounter == 0) {
	
		USART0_SendData(data,length);
		USART1_SendData(data,length);
		
		transmissionCounter++;
	}
}