/*
 * mng_eventHandler.c
 *
 * Author: Panasonic
 */ 

// Include header files
#include "../USART/usart.h"
#include "mng_eventHandler.h"
#include "../Statemachine/statemachine.h"


void mng_eventHandler(unsigned char length) {

	// Check OpCode
	switch (receptionBuffer[4]) {
		
		// TCU_MNG_INIT_RESPONSE
		case 0x81:
		
			// Check Status
			switch (receptionBuffer[7]) {
				
				// successful initialisation
				case 0x00:
					setState(CoD);
				break;
				
				// parameter failure
				case 0x01: // Set next state accordingly
				break;

				// initialisation finished
				case 0x02: // Set next state accordingly
				break;
			}
		break;
		
		
		
		// TCU_MNG_STANDARD_HCI_SET_RESP_Write_Class_of_Device
		// TCU_MNG_SSP_SET_RESP_HCI_IO_Capability_Request_Reply
		case 0xBD:	
		
			// Check Status
			switch (receptionBuffer[7]) {
				
				// successful
				case 0x00:
				
					// Check command complete OpCode LSB
					switch (receptionBuffer[12]) {
					
						//TCU_MNG_STANDARD_HCI_SET_RESP_Write_Class_of_Device
						case 0x24:
							// Check Status
							switch (receptionBuffer[14]) {
								// successful
								case 0x00:
									setState(SPPSetup);
								break;
							}
						break;
					
						
						// TCU_MNG_SSP_SET_RESP_HCI_IO_Capability_Request_Reply
						case 0x2b:
							// Check Status
							switch (receptionBuffer[14]) {
								// successful
								case 0x00:
									// Stay in state and wait for further responses
								break;
							}
						break;
					
					
						// TCU_MNG_SSP_SET_RESP_HCI_User_Confirmation_Request_Reply
						case 0x2c:
							// Check Status
							switch (receptionBuffer[14]) {
								// successful
								case 0x00:
									// Stay in state and wait for further responses
								break;
							}
						break;		
					}
				break;
				
				// unsuccessful
				default:
					// Set next state accordingly
				break;
			}
		break;
		
		
		
		// TCU_MNG_SET_SCAN_RESP
		case 0x8C:
			// Check Status
			switch (receptionBuffer[7]) {
				// successful scan
				case 0x00:
					setState(SPPConnect);
				break;
				// parameter failure
				case 0x01: 
					// Set next state accordingly
				break;
				// no device initialisation
				case 0x03: 
					// Set next state accordingly
				break;
				// no setup profile
				case 0x08: 
					// Set next state accordingly
				break;
			}
		break;
		
		
		
		// TCU_ACCEPT
		case 0xF1:
			// Check Status
			switch (receptionBuffer[7]) {
				// successful scan
				case 0x00:
					// Stay in state and wait for further responses to come in
				break;
				default: 
					// Handle the possible responses here
				break;
			}	
		break;
		
		
		
		// TCU_MNG_CONNECTION_STATUS_EVENT
		case 0x47:
		
			// Check Status
			switch (receptionBuffer[7]) {
				
				// successful
				case 0x00:
				// Check Connection Status
				switch (receptionBuffer[14]) {
					
					// connected
					case 0x00:
						// Stay in state and wait for further responses to come in
					break;
					
					// Link key
					case 0x03:
						// Stay in state and wait for further responses to come in
					break;
					
					default:
						// Handle the possible responses here
					break;
				}
				break;
			}
		break;		



		// TCU_MNG_REMOTE_DEVICE_NAME_AUTO_NOTIFY_EVENT
		case 0x6E:
			// Stay in state and wait for further responses to come in
			// This section can be used to verify the parameters of the remote device
		break;
		


		// HCI_IO_Capability_Request_Event
		case 0x7D:
		
			// Check OpCode
			switch(receptionBuffer[7]) {
				
				// HCI_IO_Capability_Request_Event
				case 0x31:
					setState(ControlSSP);
				break;
				
				// HCI_IO_Capability_Response_Event
				case 0x32:
					// Stay in state and wait for further responses to come in
				break;
				
				// HCI_IO_User_Confirmation_Request_Event
				case 0x33:
					setState(ControlSSP2);
				break;
				
				// HCI_Simple_Pairing_Complete_Event
				case 0x36:
				
					switch (receptionBuffer[9]) {
					
						// successful
						case 0x00:
							// Stay in state and wait for further responses to come in
						break;
					
						// not successful
						default:
							// Set next state accordingly
						break;
					}
				break;
			}
		break;
	}
}