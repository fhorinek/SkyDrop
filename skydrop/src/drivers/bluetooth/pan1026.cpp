#include "pan1026.h"
#include "bt.h"

extern pan1026 bt_pan1026;
CreateStdOut(bt_pan1026_out, bt_pan1026.StreamWrite);

//#define DEBUG_BT

enum pan1026_state_e
{
	pan_state_reset,

};

enum pan1026_cmd_e
{
	pan_cmd_none 				= 0,
	pan_cmd_reset 				= 1,
	pan_cmd_fw 					= 2,
	pan_cmd_en_i2c 				= 3,
	pan_cmd_eeprom_write_en 	= 4,
	pan_cmd_eeprom_read 		= 5,
	pan_cmd_write_mac 			= 6,
	pan_cmd_set_mode 			= 7,
	pan_cmd_mng_init 			= 8,
	pan_cmd_write_cod			= 9,
	pan_cmd_spp_setup			= 10,
	pan_cmd_listen				= 11,
	pan_cmd_accept_connection	= 12,
	pan_cmd_io_cap_respose		= 13,
	pan_cmd_confirmation_reply	= 14,
	pan_cmd_create_spp			= 15,
};

enum pan1026_parser_e
{
	pan_parser_idle,
	pan_parser_head,
	pan_hci_packet,
	pan_tcu_packet,
};


#define PAN1026_ERROR \
	do { \
	DEBUG1("HARD ERROR, Restart"); \
	this->Restart(); } while(0); \

void pan1026::TxResume()
{
	this->usart->TxComplete();
}

void pan1026::StreamWrite(uint8_t data)
{
//	DEBUG("%02X ", data);
	this->usart->Write(data);
}

void pan1026::Init(Usart * uart)
{
	this->usart = uart;

	this->usart->Init(BT_UART, 115200);
	this->usart->SetInterruptPriority(MEDIUM);
//	this->usart->SetCtsPin(BT_CTS);
//	this->usart->SetRtsPin(BT_CTS);

	bt_irgh(BT_IRQ_INIT, 0);

	strcpy_P(this->label, PSTR("SkyDrop Pan1026"));

	this->Restart();
}

void pan1026::Restart()
{
	DEBUG("reset_start\n");
	this->connected = false;
	this->parser_buffer_index = 0;
	this->parser_status = pan_parser_idle;

	bt_irgh(BT_IRQ_RESET, 0);

	bt_module_reset();
}

void pan1026::SetNextStep(uint8_t cmd)
{
	this->next_cmd = cmd;
}

void pan1026::ParseHCI()
{
	uint16_t op_code;
	uint8_t status;

#ifdef DEBUG_BT
	DEBUG("\n - HCI ----\n");
	for (uint8_t i = 0; i < this->parser_packet_length; i++)
		DEBUG("%02X ", this->parser_buffer[i]);
	DEBUG("\n");
#endif

	uint8_t event_code = this->parser_buffer[1];

#ifdef DEBUG_BT
	DEBUG("event_code %02X\n", event_code);
#endif

	switch (event_code)
	{
		case(0x0e): // HCI Command Complete
			op_code = this->parser_buffer[4] | (this->parser_buffer[5] << 8);
			status =  this->parser_buffer[6];

#ifdef DEBUG_BT
			DEBUG("op_code %04X\n", op_code);
			DEBUG("status %02X\n", status);
#endif

			switch(op_code)
			{
				case (0x0000): //after HW reset
					this->SetNextStep(pan_cmd_reset);
				break;

				case (0x0c03): //after SW reset
					if (status == 0) //sucess
						this->SetNextStep(pan_cmd_fw);
					else
						PAN1026_ERROR;
				break;

				case (0x1013): // Write BD Address response
					if (status == 0x00)
						this->SetNextStep(pan_cmd_set_mode);
					else
						PAN1026_ERROR;
				break;
			}

		break;

		case(0xff): //HCI extension
			uint8_t ocf = this->parser_buffer[3];
			uint8_t cmd = this->parser_buffer[5];

			if (ocf == 0x08) // Check OCF
			{
				if (cmd == 0x99)// HCI set mode event
				{
					if (this->parser_buffer[6] == 0) //sucess
					{
						this->SetNextStep(pan_cmd_mng_init);
						uint8_t tmp = BT_PAN1026;
						bt_irgh(BT_IRQ_INIT_OK, &tmp);
					}
					else
						PAN1026_ERROR;
				}
				else
				{
					uint8_t info = this->parser_buffer[10];

					switch(info)
					{
						case (0x0D): //GET FW info
							if (this->parser_buffer[11] == 0) //sucess
							{
								DEBUG("Pan1026 firmware version %s\n", this->parser_buffer + 13);
								this->SetNextStep(pan_cmd_en_i2c);
							}
							else
								PAN1026_ERROR;
						break;

						case (0x5B): //Enable I2C Interface
							if (this->parser_buffer[11] == 0) //sucess
							{
								this->SetNextStep(pan_cmd_eeprom_write_en);
							}
							else
								PAN1026_ERROR;
						break;

						case (0x83): //Enable EEPROM Write Enable
							if (this->parser_buffer[11] == 0) //sucess
								this->SetNextStep(pan_cmd_eeprom_read);
							else
								PAN1026_ERROR;
						break;

						case (0x88): //Read MAC
							if (this->parser_buffer[11] == 0) //sucess
							{
								DEBUG("MAC: ");
								memcpy(this->pan_mac_address, this->parser_buffer + 14, 6);
								for (uint8_t i = 0; i < 6; i++)
									DEBUG("%02X ", this->pan_mac_address[i]);
								DEBUG("\n");
								this->SetNextStep(pan_cmd_write_mac);
							}
							else
								PAN1026_ERROR;
						break;
					}
				}
			}
		break;
	}

}

void pan1026::ParseMNG()
{
	uint8_t status, hci_status, op_code, t_len;
	uint16_t hci_op_code;

#ifdef DEBUG_BT
	DEBUG("\n - MNG ----\n");
	for (uint8_t i = 0; i < this->parser_packet_length; i++)
		DEBUG("%02X ", this->parser_buffer[i]);
	DEBUG("\n");
#endif

	op_code = this->parser_buffer[4];
	status = this->parser_buffer[7];

#ifdef DEBUG_BT
	DEBUG("op_code: %02X\n", op_code);
	DEBUG("status: %02X\n", status);
#endif

	switch (op_code)
	{

		case(0x47)://TCU_MNG_CONNECTION_STATUS_EVENT
			DEBUG("Connection Status: \n");
			DEBUG(" Status          %02X\n", status);
			DEBUG(" MAC             ");
			for (uint8_t i = 0; i < 6; i++)
				DEBUG("%02X ", this->parser_buffer[8 + 5 - i]);
			DEBUG("\n");
			DEBUG(" Connection status %02X - ", this->parser_buffer[14]);

			switch(this->parser_buffer[14])
			{
				case(0x00):
					DEBUG("Connected");
				break;
				case(0x01):
					DEBUG("Disconnected");
				break;
				case(0x02):
					DEBUG("Connection Failure");
				break;
				case(0x03):
					DEBUG("Link key");
				break;
				case(0x04):
					DEBUG("Mode Change Active");
				break;
				case(0x05):
					DEBUG("Mode Change Hold");
				break;
				case(0x06):
					DEBUG("Mode Change Sniff");
				break;
				case(0x07):
					DEBUG("Mode Change Park");
				break;
			}
			DEBUG("\n");

			if (this->parser_buffer[14] == 0x03)
			{
				DEBUG(" Link key        ");
				for (uint8_t i = 0; i < 16; i++)
				{
					DEBUG("%02X ", this->parser_buffer[15 + i]);
					this->link_key[i] = this->parser_buffer[15 + i];
				}
				DEBUG("\n");
				DEBUG(" Link key type   %02X\n", this->parser_buffer[33]);
				DEBUG(" Sniff interval  %02X\n", this->parser_buffer[34] | (this->parser_buffer[35] << 8));
			}
		break;


		case(0x55):
			DEBUG("Connection request from: \n ");
			memcpy(this->client_mac_address, this->parser_buffer + 7, 6);
			for (uint8_t i = 6; i > 0; i--)
				DEBUG("%02X ", this->client_mac_address[i - 1]);
			DEBUG("\n");

			this->SetNextStep(pan_cmd_accept_connection);

		break;

		case(0x6e): //TCU_MNG_REMOTE_DEVICE_NAME_AUTO_NOTIFY_EVENT
			DEBUG("Client info\n");
			t_len = this->parser_buffer[13];
			DEBUG("Device name len %d\n", t_len);
			if (t_len > sizeof(this->client_name) - 1)
				t_len = sizeof(this->client_name) - 1;

			this->client_name[t_len] = 0;
			memcpy(this->client_name, this->parser_buffer + 14, t_len);
			DEBUG("Device name ");
		break;

		case(0x7d):
			hci_status = this->parser_buffer[6];
			hci_op_code = this->parser_buffer[7];

			DEBUG("HCI status %02X\n", hci_status);
			DEBUG("HCI op_code %02X\n", hci_op_code);

			switch(hci_op_code)
			{
				case(0x31)://HCI_IO_Capability_Request_Event
					this->SetNextStep(pan_cmd_io_cap_respose);
				break;

				case(0x32)://HCI_IO_Capability_Response_Event
					DEBUG("IO_Capability_Response\n");
					DEBUG(" IO_Capability    %02X\n", this->parser_buffer[15]);
					DEBUG(" OOB_Data_Present %02X\n", this->parser_buffer[16]);
					DEBUG(" Authentication_Requirement %02X\n", this->parser_buffer[16]);
				break;

				case(0x33)://HCI_IO_User_Confirmation_Request_Event
					this->SetNextStep(pan_cmd_confirmation_reply);
				break;

				case(0x36)://HCI_Simple_Pairing_Complete_Event
					DEBUG("Simple_Pairing_Complete\n");
					DEBUG(" Status %02X\n", this->parser_buffer[9]);
				break;
			}

		break;


		case(0x81): // TCU_MNG_INIT_RESPONSE
			if (status == 0)
				this->SetNextStep(pan_cmd_write_cod);
			else
				PAN1026_ERROR;
		break;

		case(0x8c): // TCU_MNG_SET_SCAN_RESP
			if (status == 0)
				this->SetNextStep(pan_cmd_create_spp);
			else
				PAN1026_ERROR;
		break;

		case(0x93): //TCU_MNG_CONNECTION_ACCEPT_RESP
			DEBUG("Pair status:\n ");
			switch(status)
			{
				case(0x00):
					DEBUG("Successful\n");
				break;
				case(0x01):
					DEBUG("Parameter Failure\n");
				break;
				case(0x03):
					DEBUG("No Device Initialization\n");
				break;
				case(0x06):
					DEBUG("No Connection\n");
				break;
			}
		break;

		case(0xBD): //Encapsulated HCI_RESP
			hci_status = this->parser_buffer[14];
			hci_op_code = this->parser_buffer[12] | (this->parser_buffer[13] << 8);

			DEBUG("HCI status %02X\n", hci_status);
			DEBUG("HCI op_code %04X\n", hci_op_code);

			switch (hci_op_code)
			{
				case(0x0c24): //Write Class of Device Command response
					if (status == 0 && hci_status == 0)
						this->SetNextStep(pan_cmd_spp_setup);
					else
						PAN1026_ERROR
				break;

				case(0x042b):// TCU_MNG_SSP_SET_RESP_HCI_IO_Capability_Request_Reply
				break;

				case(0x042c):// TCU_MNG_SSP_SET_RESP_HCI_User_Confirmation_Request_Reply
				break;
			}
		break;

	}
}

void pan1026::ParseSPP()
{

#ifdef DEBUG_BT
	DEBUG("\n - SPP ----\n");
	for (uint8_t i = 0; i < this->parser_packet_length; i++)
		DEBUG("%02X ", this->parser_buffer[i]);
	DEBUG("\n");
#endif

	uint8_t op_code, status;
	op_code = this->parser_buffer[4];
	status = this->parser_buffer[7];

#ifdef DEBUG_BT
	DEBUG("op_code: %02X\n", op_code);
	DEBUG("status: %02X\n", status);
#endif

	switch(op_code)
	{
		case(0x81): //TCU_SPP_SETUP_RESP
			if (status == 0)
				this->SetNextStep(pan_cmd_listen);
			else
				PAN1026_ERROR;
		break;

		case(0x43): //TCU_SPP_CONNECT_EVENT
			if (status == 0)
				bt_irgh(BT_IRQ_CONNECTED, 0);
			else
				PAN1026_ERROR;
		break;

		case(0x44): //TCU_SPP_DISCONNECT_EVENT
			if (status == 0)
				bt_irgh(BT_IRQ_DISCONNECTED, 0);
			else
				PAN1026_ERROR;
		break;

		case(0xF1): // TCU_SPP_DATA_SEND_EVENT
#ifdef DEBUG_BT
			DEBUG("data send!\n");
#endif
		break;
	}
}

void pan1026::Parse(uint8_t c)
{
//	DEBUG("1026>%02X %c\n", c, c);

	if (this->usart->rx_ovf)
	{
		DEBUG("RX OVF\n");
		this->usart->rx_ovf = false;
	}


	switch(this->parser_status)
	{
		case(pan_parser_idle):
			this->parser_buffer[0] = c;
			this->parser_buffer_index = 1;
			this->parser_status = pan_parser_head;
		break;

		case(pan_parser_head):
			this->parser_buffer[this->parser_buffer_index] = c;
			this->parser_buffer_index++;
			if (this->parser_buffer_index == 3)
			{
				if (this->parser_buffer[0] == 0x04) //HCI
				{
					this->parser_status = pan_hci_packet;
					this->parser_packet_length = 3 + this->parser_buffer[2];
				}
				else //TCU
				{
					uint32_t tmp_len = this->parser_buffer[0] | (this->parser_buffer[1] << 8) | ((uint32_t)this->parser_buffer[2] << 16);
					if (tmp_len > 0xFFFF)
					{
						DEBUG("WARNING len = %lu\n", tmp_len);
						PAN1026_ERROR;
					}
					this->parser_packet_length = tmp_len;
					this->parser_status = pan_tcu_packet;
				}
			}
		break;

		case(pan_hci_packet):
			this->parser_buffer[this->parser_buffer_index] = c;
			this->parser_buffer_index++;


			if (this->parser_buffer_index == this->parser_packet_length)
			{
				this->ParseHCI();
				this->parser_status = pan_parser_idle;
			}
		break;

		case(pan_tcu_packet):
			this->parser_buffer[this->parser_buffer_index] = c;
			this->parser_buffer_index++;

			if (this->parser_buffer_index == this->parser_packet_length)
			{
				switch(this->parser_buffer[3]) //service id
				{
					case (0xE1):
						this->ParseMNG();
					break;

					case (0xE5):
						this->ParseSPP();
					break;

				}
				this->parser_status = pan_parser_idle;
			}
		break;


	}
}

const uint8_t PROGMEM TCU_HCI_RESET_REQ[] = {0x01, 0x03, 0x0c, 0x00};
const uint8_t PROGMEM TCU_HCI_GET_FIRMWARE_VERSION_REQ[] = {0x01, 0x08, 0xfc, 0x09, 0x00, 0xa1, 0x00, 0x00, 0x00, 0x14, 0x0d, 0xff, 0x00};
const uint8_t PROGMEM TCU_HCI_M2_BTL_SET_I2C_ENABLE_REQ[] = {0x01, 0x08, 0xfc, 0x0b, 0x00, 0xa0, 0x00, 0x00, 0x00, 0x14, 0x5b, 0xff, 0x02, 0x03, 0x01};
const uint8_t PROGMEM TCU_HCI_M2_BTL_EEPROM_WRITE_ENABLE_REQ[] = {0x01, 0x08, 0xfc, 0x09, 0x00, 0xa0, 0x00, 0x00, 0x00, 0x14, 0x83, 0xff, 0x00};
const uint8_t PROGMEM TCU_HCI_M2_GENERAL_READ_EEPROM_REQ[] = {0x01, 0x08, 0xfc, 0x10, 0x00, 0xa1, 0x00, 0x00, 0x00, 0x14, 0x88, 0xff, 0x10, 0x06, 0xa0, 0x01, 0x01, 0x06, 0x02, 0x00};
const uint8_t PROGMEM TCU_HCI_WRITE_BD_ADDR_REQ[] = {0x01, 0x13, 0x10, 0x06};
const uint8_t PROGMEM TCU_HCI_SET_MODE_REQ[] = {0x01, 0x08, 0xfc, 0x03, 0x00, 0x99, 0x01};
const uint8_t PROGMEM TCU_MNG_INIT_REQ[] = {0xe1, 0x01};
const uint8_t PROGMEM TCU_MNG_STANDARD_HCI_SET_REQ_Write_Class_of_Device[] = {0x0d, 0x00, 0x00, 0xe1, 0x3d, 0x06, 0x00, 0x24, 0x0c, 0x03, 0x10, 0x05, 0x01};
const uint8_t PROGMEM TCU_SPP_SETUP_REQ[] = {0x07, 0x00, 0x00, 0xe5, 0x01, 0x00, 0x00};
const uint8_t PROGMEM TCU_MNG_SET_SCAN_REQ[] = {0x08, 0x00, 0x00, 0xe1, 0x0c, 0x01, 0x00, 0x03};
const uint8_t PROGMEM TCU_MNG_CONNECTION_ACCEPT_REQ[] = {0xe1, 0x13};
const uint8_t PROGMEM TCU_MNG_SSP_SET_REQ_HCI_IO_Capability_Request_Reply[] = {0x13, 0x00, 0x00, 0xe1, 0x3d, 0x0c, 0x00, 0x2b, 0x04, 0x09};
const uint8_t PROGMEM TCU_MNG_SSP_SET_REQ_HCI_User_Confirmation_Request_Reply[] = {0x11, 0x00, 0x00, 0xe1, 0x3d, 0x0a, 0x00, 0x2c, 0x04, 0x06};
const uint8_t PROGMEM TCU_MNG_SSP_SET_REQ_HCI_User_Confirmation_Negative_Reply[] = {0x11, 0x00, 0x00, 0xe1, 0x3d, 0x0a, 0x00, 0x2d, 0x04, 0x06};


#define RAW(data) \
	this->RawSendStatic(data, sizeof(data))

#define TCU_LEN(len) \
	this->StreamWrite(((len) & 0x000000FF) >> 0); \
	this->StreamWrite(((len) & 0x0000FF00) >> 8); \
	this->StreamWrite(((len) & 0x00FF0000) >> 16);

#define PARAM_LEN(len) \
	this->StreamWrite(((len) & 0x00FF) >> 0); \
	this->StreamWrite(((len) & 0xFF00) >> 8);


void pan1026::RawSendStatic(const uint8_t * data, uint8_t len)
{
	for(uint8_t i = 0; i < len; i++)
		this->StreamWrite(pgm_read_byte(&data[i]));
}


void pan1026::Step()
{
	uint16_t t_len;

	while (!this->usart->isRxBufferEmpty())
	{
		this->Parse(this->usart->Read());
	}

	if (this->next_cmd != pan_cmd_none)
		DEBUG("\n\n< CMD %d\n", this->next_cmd);

	switch(this->next_cmd)
	{
		case(pan_cmd_reset):
			RAW(TCU_HCI_RESET_REQ);
		break;

		case(pan_cmd_fw):
			RAW(TCU_HCI_GET_FIRMWARE_VERSION_REQ);
		break;

		case(pan_cmd_en_i2c):
			RAW(TCU_HCI_M2_BTL_SET_I2C_ENABLE_REQ);
		break;

		case(pan_cmd_eeprom_write_en):
			RAW(TCU_HCI_M2_BTL_EEPROM_WRITE_ENABLE_REQ);
		break;

		case(pan_cmd_eeprom_read):
			RAW(TCU_HCI_M2_GENERAL_READ_EEPROM_REQ);
		break;

		case(pan_cmd_write_mac):
			RAW(TCU_HCI_WRITE_BD_ADDR_REQ);
			//add mac
			for (uint8_t i = 6; i > 0; i--)
				this->StreamWrite(this->pan_mac_address[i-1]);
		break;

		case(pan_cmd_set_mode):
			_delay_ms(100);
			RAW(TCU_HCI_SET_MODE_REQ);
		break;

		case(pan_cmd_mng_init):
			t_len = 3 + sizeof(TCU_MNG_INIT_REQ) + 5 + strlen(this->label);
			TCU_LEN(t_len);

			RAW(TCU_MNG_INIT_REQ);

			t_len = 3 + strlen(this->label);
			PARAM_LEN(t_len);
			this->StreamWrite(0x04); //SPP
			this->StreamWrite(0x02); //sniff settings
			this->StreamWrite(strlen(this->label)); //label name len
			fprintf_P(bt_pan1026_out, PSTR("%s"), this->label); //label name
		break;

		case(pan_cmd_write_cod):
			RAW(TCU_MNG_STANDARD_HCI_SET_REQ_Write_Class_of_Device);
		break;

		case(pan_cmd_spp_setup):
			RAW(TCU_SPP_SETUP_REQ);
		break;

		case(pan_cmd_listen):
			RAW(TCU_MNG_SET_SCAN_REQ);
		break;

		case(pan_cmd_accept_connection):
			t_len = 3 + sizeof(TCU_MNG_CONNECTION_ACCEPT_REQ) + 2 + 8;// +16;
			TCU_LEN(t_len);

			RAW(TCU_MNG_CONNECTION_ACCEPT_REQ);

			t_len = 1 + 6 + 1;// + 16;
			PARAM_LEN(t_len);
			this->StreamWrite(0x00); //Accept
			//add client mac
			for (uint8_t i = 0; i < 6; i++)
				this->StreamWrite(this->client_mac_address[i]);
			this->StreamWrite(0x00); //do not use link key
//			for (uint8_t i = 0; i < 16; i++)
//				this->StreamWrite(this->link_key[i]);
		break;

		case(pan_cmd_io_cap_respose):
			RAW(TCU_MNG_SSP_SET_REQ_HCI_IO_Capability_Request_Reply);
			//add client mac
			for (uint8_t i = 0; i < 6; i++)
				this->StreamWrite(this->client_mac_address[i]);
			this->StreamWrite(0x03); //NoInputNoOutput
			this->StreamWrite(0x00); //OOB authentication data not present XXX tuto dat 1 ak to bude mat
			this->StreamWrite(0x03); //MITM Protection Not Required – No Bonding. Numeric comparison with automatic accept allowed.
		break;

		case(pan_cmd_confirmation_reply):
			RAW(TCU_MNG_SSP_SET_REQ_HCI_User_Confirmation_Request_Reply);
//			RAW(TCU_MNG_SSP_SET_REQ_HCI_User_Confirmation_Negative_Reply);
			//add client mac
			for (uint8_t i = 0; i < 6; i++)
				this->StreamWrite(this->client_mac_address[i]);
			this->StreamWrite(0x00); //???

		default:
		break;
	}
	this->next_cmd = pan_cmd_none;

}

void pan1026::SendString(char * str)
{
	uint16_t len = strlen(str);

	TCU_LEN(len + 3 + 1 + 1 + 2 + 2);
	this->StreamWrite(0xe5); //SPP
	this->StreamWrite(0x08); //TCU_SPP_DATA_TRANSFER_REQ
	PARAM_LEN(len + 2); 	 //Parameter Length
	PARAM_LEN(len);			 //Length_of_Data
	//data
	for (uint16_t i = 0; i < len; i++)
		this->StreamWrite(str[i]);
}
