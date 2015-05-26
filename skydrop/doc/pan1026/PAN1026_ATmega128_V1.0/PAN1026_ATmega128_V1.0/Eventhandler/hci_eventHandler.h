/*
 * hci_eventHandler.h
 *
 *  Author: Panasonic
 */ 


#ifndef HCI_EVENTHANDLER_H_
#define HCI_EVENTHANDLER_H_

extern unsigned char bdAddress[6];

void hci_eventHandler (unsigned char length);

#endif /* HCI_EVENTHANDLER_H_ */