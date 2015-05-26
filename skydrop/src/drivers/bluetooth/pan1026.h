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

enum pan1026_state_e
{
	pan_state_reset,

};

enum pan1026_cmd_e
{
	pan_cmd_none 			= 0,
	pan_cmd_reset 			= 1,
	pan_cmd_fw 				= 2,
	pan_cmd_en_i2c 			= 3,
	pan_cmd_eeprom_write_en = 4,
	pan_cmd_eeprom_read 	= 5,
	pan_cmd_write_mac 		= 6,
	pan_cmd_set_mode 		= 7,
	pan_cmd_mng_init 		= 8,
	pan_cmd_write_cod		= 9,
	pan_cmd_spp_setup		= 10,
	pan_cmd_listen			= 11,
	pan_cmd_accept_connection = 12,
	pan_cmd_io_cap_respose	= 13,
	pan_cmd_confirmation_reply	= 14,
	pan_cmd_create_spp		= 15,
};

enum pan1026_parser_e
{
	pan_parser_idle,
	pan_parser_head,
	pan_hci_packet,
	pan_tcu_packet,
};

#define PARSER_BUFFER_SIZE 64

class pan1026
{
public:

	Usart usart;

	bool connected;

	void Init();
	void Restart();
	void TxResume();

	void Step();

	pan1026_cmd_e next_cmd;
	pan1026_state_e state;
	pan1026_parser_e parser_status;
	uint16_t parser_packet_length;
	uint8_t parser_buffer_index;
	uint8_t parser_buffer[PARSER_BUFFER_SIZE];

	void SetNextStep(pan1026_cmd_e cmd);

	void Parse(uint8_t c);
	void ParseHCI();
	void ParseMNG();
	void ParseSPP();

	void StreamWrite(uint8_t data);
	void RawSendStatic(const uint8_t * data, uint8_t len);

	void SendString(char * str);

	uint8_t pan_mac_address[6];

	uint8_t client_mac_address[6];

	char label[32];
	char client_name[32];

	uint8_t link_key[16];
};

#endif /* PAN1026_H_ */
