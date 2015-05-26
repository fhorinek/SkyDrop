/*
 * statemachine.c
 *
 * Author: Panasonic
 */ 

// Include header files
#include <util/delay.h>
#include "statemachine.h"
#include "../USART/usart.h"
#include "../commands.h"
#include "../EventHandler/hci_eventHandler.h"


// Variable declaration
unsigned char i = 0;							// Generic counting variable
unsigned char receptionCounter = 0;				// Counts the received bytes/characters
unsigned char responseCounter = 0;				// Counts the responses
unsigned char transmissionCounter = 0;			// Counts the transmitted commands
unsigned char responseOneByteQuantity = 0;		// Holds the amount of bytes of the first response
unsigned char responseTwoByteQuantity = 0;		// Holds the amount of bytes of the second response
unsigned char responseThreeByteQuantity = 0;	// Holds the amount of bytes of the third response
unsigned char responseFourByteQuantity = 0;		// Holds the amount of bytes of the fourth response
unsigned char receptionBuffer[140] = {0};		// Holds the received data
unsigned char responseOne[35] = {0};			// Holds the data of the first response
unsigned char responseTwo[35] = {0};			// Holds the data of the second response
unsigned char responseThree[35] = {0};			// Holds the data of the third response
unsigned char responseFour[35] = {0};			// Holds the data of the fourth response



void setState(State newState) {
	
	// Set new state
	globalState = newState;
	
	// Reset response counter
	receptionCounter = 0;
	
	// Reset transmission counter
	transmissionCounter = 0;
}

void statemachine(void)
{
	// State control
	switch (globalState) {
		case Reset:
			// Send reset command
			USART0_SendData(TCU_HCI_RESET_REQ,4);
			USART1_SendData(TCU_HCI_RESET_REQ,4);
		break;
		
		case Firmware:
			// Send firmware request command
			USARTS_SendData(TCU_HCI_GET_FIRMWARE_VERSION_REQ,13);
		break;
	
		case I2C:
			// Send enable i2c request
			USARTS_SendData(TCU_HCI_M2_BTL_SET_I2C_ENABLE_REQ,15);
		break;
		
		case EnableEEPROM:
			// Send enable EEPROM request
			USARTS_SendData(TCU_HCI_M2_BTL_EEPROM_WRITE_ENABLE_REQ,13);
		break;
		
		case ReadEEPROM:
			// Send read EEPROM request
			USARTS_SendData(TCU_HCI_M2_GENERAL_READ_EEPROM_REQ,20);
		break;
		
		case WriteBDAddress:
			// Assemble BD Address to TCU_HCI_WRITE_BD_ADDR_REQ
			for (unsigned char i=0; i<6; i++) {
				TCU_HCI_WRITE_BD_ADDR_REQ[4+i] = bdAddress[5-i];
			}
			// Send write BD Address request
			USARTS_SendData(TCU_HCI_WRITE_BD_ADDR_REQ,10);
		break;
		
		case SetMode:
			// Send set mode request
			USARTS_SendData(TCU_HCI_SET_MODE_REQ,7);
		break;
		
		case MNGInit:
			// Send MNG initialisation request (Device Name: PAN1026A, Length 0x08)
			USARTS_SendData(TCU_MNG_INIT_REQ,18);
		break;
		
		case CoD:
			// Send Class of Device
			USARTS_SendData(TCU_MNG_STANDARD_HCI_SET_REQ_Write_Class_of_Device,13);
		break;
		
		case SPPSetup:
			// Send SPP setup request
			USARTS_SendData(TCU_SPP_SETUP_REQ,7);
		break;
		
		case MNGScan:
			// Send inquiry and page scan request
			USARTS_SendData(TCU_MNG_SET_SCAN_REQ,8);
		break;
		
		case SPPConnect:
			for(i=0; i<6; i++) {
				TCU_SPP_CONNECT_REQ[7+i] = REMOTE_BDADDRESS[i];
			}
				// Send ACL and SPP connection request
				USARTS_SendData(TCU_SPP_CONNECT_REQ,23);
		break;

		case ControlSSP:
			// Add BD address defined in commands.h to HCI IO capability reply
			for(i=0; i<6; i++) {
				TCU_MNG_SSP_SET_REQ_HCI_IO_Capability_Request_Reply[10+i] = REMOTE_BDADDRESS[i];
			}
			// Send secure simple pairing reply
			USARTS_SendData(TCU_MNG_SSP_SET_REQ_HCI_IO_Capability_Request_Reply,19);
		break;
		
		
		case ControlSSP2:
			// Add BD address defined in commands.h to HCI user confirmation request reply
			for(i=0; i<6; i++) {
				TCU_MNG_SSP_SET_REQ_HCI_User_Confirmation_Request_Reply[10+i] = REMOTE_BDADDRESS[i];
			}
			// Send user confirmation request reply
			USARTS_SendData(TCU_MNG_SSP_SET_REQ_HCI_User_Confirmation_Request_Reply,17);
		break;
		
		case SPPDataTransfer:
			// Send user confirmation request reply
			USARTS_SendData(TCU_SPP_DATA_TRANSFER_REQ,21);
		break;	
		
	}
}