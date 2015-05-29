/*
 * pan_lite.h
 *
 *  Created on: 25.9.2014
 *      Author: horinek
 */

#ifndef PAN1322_H_
#define PAN1322_H_

#include "../../common.h"
#include "../uart.h"
#include <xlib/core/usart.h>

//ERR=-
//+RSLE 					pair sucesfull
//+RCOI=XXXXXXXXXXXX		incoming connection (12 HEX bt address)
//+RCCRCNF					connected
//+RDII						disconnected
//+RDAI=DDD,data			data

#define BT_STATE_START	0
#define BT_STATE_ERR	1
#define BT_STATE_RESP	2
#define BT_STATE_DATA	3
#define BT_STATE_OK		4

#define BT_EVENT_PAIR			0
#define BT_EVENT_INCOMING		1
#define BT_EVENT_CONNECTED		2
#define BT_EVENT_DISCONNECTED	3
#define BT_EVENT_DATA			4
#define BT_EVENT_ERROR			5

class pan1322
{
	bool connected;
	bool ok_ans;
	bool error;

	uint8_t p_state;
	uint8_t p_len;
	uint8_t p_type;
	char p_buff[16];
	uint8_t p_index;
	uint16_t data_len;

public:

	Usart usart;

	void Init();
	void TxResume();
	void Restart();

	bool WaitForOK(uint16_t timeout_ms = 500);
	bool SetName(const char * name);
	bool SetDiscoverable(bool discoverable);
	bool CreateService(const char * uuid, const char* name, uint8_t channel, const char * deviceClass);

	bool SetBaudrate(uint32_t baud);

	void Step();
	void Parse(uint8_t c);
	bool AcceptConnection();
	void FindRN();

	bool isConnected();

	void StreamHead(uint16_t len);
	void StreamWrite(uint8_t data);
	void StreamTail();

	void SendString(char * str);

	bool isIdle();
};

extern FILE * bt_pan1322_out;


#endif /* PAN_LITE_H_ */
