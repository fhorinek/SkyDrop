/*
 * hci_eventHandler.c
 *
 *  Author: Panasonic
 */ 

// Include header files
#include "hci_eventHandler.h"
#include "../Statemachine/statemachine.h"

// Global variable
unsigned char bdAddress[6];

void hci_eventHandler (unsigned char length) {
	
	// Variables
	short opCode = 0;
	unsigned char i = 0;
	
	// Handle data
	// Check EventCode
	switch (receptionBuffer[1]) {
		
		
		case 0x0e:			// HCI Command Complete (reset, write BD address)
	
			opCode = (short) receptionBuffer[4] + receptionBuffer[5];
		
			switch (opCode) {
				
				case 0x00:		// Hardware reset response
						setState(Reset);	
				break;
				
				
				case 0x0F:		// Reset response
					// Check Status
					if(receptionBuffer[6] == 0x00) {	// Successful
						setState(Firmware);
					} else {							// Failed
						// Handle faults here
					}
				break;
				
				
				case 0x23: // Write BD Address response
					// Check Status
					if(receptionBuffer[6] == 0x00) {	// Successful
						setState(SetMode);
					} else {							// Failed
						// Handle faults here
					}
				break;
			}
		break;
		
		
		
		
		case 0xff: // firmware version, M2_BTL_RESP, M2_GENERAL_RESP, SET_MODE_RESP
		
			// Check OCF
			switch (receptionBuffer[3]) {
				
				case 0x08:	
			
					// Check command
					switch(receptionBuffer[5]){
						case 0x99:
							// HCI set mode event
							if(receptionBuffer[6] == 0x00) {		// Successful
								setState(MNGInit);
							} else {								// Failed
								// Handle faults here
							}						
						break;
					}
				
				
					// Check InformationID
					switch (receptionBuffer[10]) {
	
						case 0x0D:		// Get Firmware Version
						// Check Status
						if(receptionBuffer[11] == 0x00) {			// Successful
							setState(I2C);
							} else {								// Failed
							// // Handle faults here
						}
						break;
	
	
						case 0x5B:		// Enable I2C Interface			
						// Check Status
						if(receptionBuffer[11] == 0x00) {			// Successful
							setState(EnableEEPROM);
							} else {								// Failed
							// Handle faults here
						}
						break;


						case 0x83: // Enable EEPROM Write Enable	
						// Check Status
						if(receptionBuffer[11] == 0x00) {			// Successful
							setState(ReadEEPROM);
							} else {								// Failed
							// Handle faults here	
						}
						break;


						case 0x88: // Read Data from EEPROM				
						// Check Status
						if(receptionBuffer[11] == 0x00) {			// Successful
		
							// Read BD Address from EEPROM
							for (i=0; i<6; i++) {
								bdAddress[i] = receptionBuffer[14+i];
							}
							
							setState(WriteBDAddress);
							
						} else {									// Failed
							// Handle faults here
						}
						break;
					}							
				break;
			}	
		break;	
	}
}
