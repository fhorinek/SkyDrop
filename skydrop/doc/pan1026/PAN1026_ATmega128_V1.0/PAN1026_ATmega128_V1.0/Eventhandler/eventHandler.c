/*
 * eventHandler.c
 *
 * Author: Panasonic
 */ 

// Include header files
#include <util/delay.h>
#include "../USART/usart.h"
#include "eventHandler.h"
#include "../Statemachine/statemachine.h"
#include "hci_eventHandler.h"
#include "mng_eventHandler.h"
#include "spp_eventHandler.h"


void eventHandler (void) {
	
	unsigned char length = 0;	// Holds the length of the response
	unsigned char i,j = 0;		// Generic counting variables
		
	// Wait for the response
	_delay_ms(300);
	length = receptionCounter;	// Read length of the received data
	receptionCounter = 0;		// Reset reception counter
	
	
	// send debugging information to the PC terminal
	USART0_SendData(receptionBuffer,length);
	
	
	// Check, whether HCI or Complete Mode
	if(receptionBuffer[0] == 0x04) {		// HCI Mode
		
		hci_eventHandler(length);
		
	} else {								// Complete Mode
			
		// For multiple responses (4 responses): 
		// Get the amount of bytes from the reception buffer and the data for each single response
		responseOneByteQuantity = receptionBuffer[0];
		
		for (i=0; i<responseOneByteQuantity; i++){
			responseOne[i] = receptionBuffer[i];
		}	
		
		if(receptionBuffer[responseOneByteQuantity] != 0x00){
							
			responseTwoByteQuantity = receptionBuffer[responseOneByteQuantity];
			
			for (i=0; i<responseTwoByteQuantity; i++){
				responseTwo[i] =  receptionBuffer[responseOneByteQuantity+i];
			}
			
			responseCounter++;
			
			if(receptionBuffer[responseOneByteQuantity+responseTwoByteQuantity] != 0x00){
				
				responseThreeByteQuantity = receptionBuffer[responseOneByteQuantity+responseTwoByteQuantity];
				
				for (i=0; i<responseThreeByteQuantity; i++){
					responseThree[i] =  receptionBuffer[responseOneByteQuantity+responseTwoByteQuantity+i];
				}			
			
				responseCounter++;
				
				if(receptionBuffer[responseOneByteQuantity+responseTwoByteQuantity+responseThreeByteQuantity] != 0x00){
				
					responseFourByteQuantity = receptionBuffer[responseOneByteQuantity+responseTwoByteQuantity+responseThreeByteQuantity];
			
					for (i=0; i<responseFourByteQuantity; i++){
						responseFour[i] =  receptionBuffer[responseOneByteQuantity+responseTwoByteQuantity+responseThreeByteQuantity+i];
					}
					
					responseCounter++;
					
				}
			}
		}
		
		// For multiple responses: Handle the responses separately
		for(i=0; i<=responseCounter; i++){
		
			switch(responseCounter) {			
				
				// First response
				case 0:
					for(j=0; j < responseOneByteQuantity; j++){
						receptionBuffer[j] = responseOne[j];
					}					
				break;
				
				// Second response
				case 1:					
					for(j=0; j < responseTwoByteQuantity; j++){
						receptionBuffer[j] = responseTwo[j];
					}
				break;
				
				// Third response
				case 2:
					for(j=0; j < responseThreeByteQuantity; j++){
						receptionBuffer[j] = responseThree[j];
					}
				break;
				
				// Fourth response
				case 3:
					for(j=0; j < responseFourByteQuantity; j++){
						receptionBuffer[j] = responseFour[j];
					}
				break;
			}
		
			// Check Service ID
			switch (receptionBuffer[3]) {
			
				// Start MNG_EventHandler
				case 0xE1:
					mng_eventHandler(length);
				break;
			
				// Start SPP_EventHandler
				case 0xE5:
					spp_eventHandler(length);
				break;
			}	
		}	
	}
	
	// Clear buffers
	if(responseCounter != 0) {
		for (j=0; j<35; j++){
			responseOne[j] = 0x00;
			responseTwo[j] = 0x00;
			responseThree[j] = 0x00;
			responseFour[j] = 0x00;
		}
	}
	
	// Reset the response Counter
	responseCounter = 0;
	
	// Clear reception buffer
	for (j=0; j<140; j++) {
		receptionBuffer[j] = 0x00;
	}	
}