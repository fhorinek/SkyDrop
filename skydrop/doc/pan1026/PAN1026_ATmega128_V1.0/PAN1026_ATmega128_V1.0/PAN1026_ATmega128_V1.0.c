/*
 * PAN1026_ATmega128_V1.0.c
 *
 * Author: Panasonic
 */ 

// Header files
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "USART/usart.h"
#include "Statemachine/statemachine.h"
#include "EventHandler/eventHandler.h"


int main(void)
{
	// Time to activate the debug terminal
	_delay_ms(5000);
	
	// Initialize USART channels
	USART0_Init();
	USART1_Init();

	// Set initial state
	setState(Reset);

	// Enable global interrupts
	sei();
	
    while(1){
		
		// Handle the states	
		statemachine();
		
		// Handle Incoming Events
		eventHandler();

	}
}


// Reception interrupt routine
ISR(USART1_RX_vect){
	
	receptionBuffer[receptionCounter] = UDR1;
	receptionCounter++;
	
}



