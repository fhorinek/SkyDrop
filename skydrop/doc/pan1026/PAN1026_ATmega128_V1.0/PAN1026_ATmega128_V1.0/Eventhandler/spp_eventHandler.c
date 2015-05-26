/*
 * spp_eventHandler.c
 *
 * Author: Panasonic
 */ 

// Include header files
#include "../USART/usart.h"
#include "spp_eventHandler.h"
#include "../Statemachine/statemachine.h"


void spp_eventHandler(unsigned char length) {

	// Check OpCode
	switch (receptionBuffer[4]) {
		
		// SPP_SETUP_RESP
		case 0x81:
			// Check Status
			switch (receptionBuffer[7]) {
				// successful initialisation
				case 0x00:
					setState(MNGScan);
				break;
				// parameter failure
				case 0x01: 
					// Set next state accordingly
				break;
				// no device initialisation
				case 0x03: 
					// Set next state accordingly
				break;
				// Setup SPP
				case 0x40: 
					// Set next state accordingly
				break;
			}
		break;
		
		
		// TCU_SPP_CONNECT_EVENT
		case 0x43:
			// Check status
			switch (receptionBuffer[7]) {
				// successful
				case 0x00:
					setState(SPPDataTransfer);
				break;
				default: 
					// Handle the possible responses here
				break;
			}
		break;
		
		
		// TCU_SPP_DATA_SEND_EVENT
		case 0xF1:
			// Data transmission has been completed.
			// Set next state accordingly
		break;
	}
}