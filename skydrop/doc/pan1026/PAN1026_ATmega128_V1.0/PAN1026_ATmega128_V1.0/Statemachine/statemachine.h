/*
 * statemachine.h
 *
 * Author: Panasonic
 */ 


#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_

extern unsigned char i;
extern unsigned char receptionCounter;
extern unsigned char responseCounter;
extern unsigned char transmissionCounter;
extern unsigned char responseOneByteQuantity;
extern unsigned char responseTwoByteQuantity;
extern unsigned char responseThreeByteQuantity;
extern unsigned char responseFourByteQuantity;
extern unsigned char receptionBuffer[140];
extern unsigned char responseOne[35];
extern unsigned char responseTwo[35];
extern unsigned char responseThree[35];
extern unsigned char responseFour[35];

typedef enum {Reset, Firmware, I2C, EnableEEPROM, ReadEEPROM, WriteBDAddress, SetMode, MNGInit, CoD, SPPSetup, MNGScan, SPPConnect, ControlSSP, ControlSSP2, SPPDataTransfer} State;
	
State globalState;

void setState(State newState);
void statemachine(void);

#endif /* STATEMACHINE_H_ */