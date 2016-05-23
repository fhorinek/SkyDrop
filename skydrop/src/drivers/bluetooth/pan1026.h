/*
 * pan1026.h
 *
 *  Created on: 18.5.2015
 *      Author: horinek
 */

#ifndef PAN1026_H_
#define PAN1026_H_

#include "../../common.h"
#include "../uart.h"
#include "../../xlib/core/usart.h"

#define PARSER_BUFFER_SIZE 64
#define BTLE_SPP_BUFFER_SIZE 85

class pan1026
{
public:

	Usart * usart;

	bool connected;

	void Init(Usart * uart);
	void Restart();
	void TxResume();

	void Step();

	uint8_t next_cmd;
	uint8_t last_cmd;
	uint8_t state;
	uint8_t parser_status;
	uint16_t parser_packet_length;
	uint8_t parser_buffer_index;
	uint8_t parser_buffer[PARSER_BUFFER_SIZE];

	uint8_t cmd_iter;

	bool btle_connection;

	uint16_t btle_service_handles[3];
	uint16_t btle_characteristic_handles[7];
	uint16_t btle_characteristic_element_handles[7];
	uint16_t btle_notifications;

	uint16_t btle_connection_handle;

	bool repat_last_cmd;
	bool busy;

	void SetNextStep(uint8_t cmd);

	void Parse(uint8_t c);
	void ParseHCI();
	void ParseMNG();
	void ParseSPP();

	void ParseMNG_LE();
	void ParseGAT_cli();
	void ParseGAT_ser();

	bool Idle();
	void WaitForAnswer();
	void StreamWrite(uint8_t data);
	void RawSendStatic(const uint8_t * data, uint8_t len);

	void SendString(char * str);

	uint8_t pan_mac_address[6];

	uint8_t client_mac_address[6];

	char label[32];
	char client_name[32];

	uint32_t timer;
};

#endif /* PAN1026_H_ */
