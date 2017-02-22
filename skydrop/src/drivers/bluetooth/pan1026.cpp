#include "pan1026.h"
#include "bt.h"

#include "../../fc/protocols/protocol.h"

extern pan1026 bt_pan1026;
CreateStdOut(bt_pan1026_out, bt_pan1026.StreamWrite);

/*! The max. ATT size of TC35661 is limited to 64 bytes */
#define PAN1026_SPP_MTU			512//max 543
#define PAN1026_MTU_RX			64 //max 64
#define PAN1026_MTU_TX			64 //max 64
#define PAN1026_MTU_TX_FALLBACK	20 //min 20

#define BT_TIMEOUT			1000
#define BT_TIMEOUT_CONN		2000
#define BT_TIMEOUT_ACCEPT	200
#define BT_TIMEOUT_TCU_ERR	50
#define BT_TIMEOUT_SEND		100
#define BT_NO_TIMEOUT		0xFFFFFFFF

//#define DEBUG_BT_ENABLED

#ifdef DEBUG_BT_ENABLED
	#define DEBUG_BT(...) DEBUG(__VA_ARGS__)
#else
	#define DEBUG_BT(...)
#endif

#define SERVICE_GENERIC	0
#define SERVICE_DEVICE	1
#define SERVICE_SPP		2

#define CHAR_GENERIC_NAME			0
#define CHAR_GENERIC_APPEARANCE		1
#define CHAR_DEVICE_MANUFACTURER	2
#define CHAR_DEVICE_MODEL_NUMBER	3
#define CHAR_DEVICE_SYSTEM_ID		4
#define CHAR_SPP_SPP				5
#define CHAR_SPP_SPP_DESC			6

#define BTLE_NOTIFICATION			0x0001
#define BTLE_INDICATION				0x0002

enum pan1026_cmd_e
{
	//init
	pan_cmd_none 				= 0,
	pan_cmd_reset 				= 1,
	pan_cmd_fw 					= 2,
	pan_cmd_en_i2c 				= 3,
	pan_cmd_eeprom_write_en 	= 4,
	pan_cmd_eeprom_read 		= 5,
	pan_cmd_write_mac 			= 6,
	pan_cmd_set_mode 			= 7,

	//classic
	pan_cmd_mng_init 			= 8,
	pan_cmd_write_cod			= 9,
	pan_cmd_spp_setup			= 10,
	pan_cmd_listen				= 11,
	pan_cmd_accept_connection	= 12,
	pan_cmd_io_cap_respose		= 13,
	pan_cmd_confirmation_reply	= 14,
	pan_cmd_spp_send			= 15,

	//btle
	pan_cmd_le_mng_init								= 16,
	pan_cmd_le_gat_ser_init							= 17,
	pan_cmd_le_gat_cli_init							= 18,
	pan_cmd_le_gat_sdb_add_primary_service 			= 19,
	pan_cmd_le_gat_sdb_add_characteristic			= 20,
	pan_cmd_le_gat_sdb_add_characteristic_elements  = 21,
	pan_cmd_le_gen_random_address 					= 22,
	pan_cmd_le_set_random_address 					= 23,
	pan_cmd_le_start_advertise						= 24,
	pan_cmd_le_update_char_element 					= 25,
	pan_cmd_le_read_val_accept 						= 26,
	pan_cmd_le_read_multiple_accept 				= 27,
	pan_cmd_le_write_val_accept 					= 28,
	pan_cmd_le_val_notification 					= 29,
	pan_cmd_le_read_char_des_accept 				= 30,
	pan_cmd_le_write_char_des_accept 				= 31,
	pan_cmd_le_val_indication 						= 32,
	pan_cmd_le_mtu_req 								= 33,
	pan_cmd_le_mtu_accept							= 34,
};

enum pan1026_parser_e
{
	pan_parser_idle,
	pan_parser_head,
	pan_hci_packet,
	pan_tcu_packet,
	pan_parser_wait,
};


#define PAN1026_ERROR \
	do { \
	DEBUG_BT("PAN1026 HARD ERROR, Restart\n"); \
	this->Restart(); } while(0); \

void pan1026::TxResume()
{
	this->usart->TxComplete();
}

void pan1026::StreamWrite(uint8_t data)
{
	DEBUG_BT("%02X ", data);
	this->usart->Write(data);
}

void pan1026::Init(Usart * uart)
{
	this->usart = uart;

	bt_irqh(BT_IRQ_INIT, 0);

	this->Restart();
}

void pan1026::Restart()
{
	this->parser_buffer_index = 0;
	this->parser_status = pan_parser_idle;
	this->btle_connection = false;
	this->repat_last_cmd = false;
	this->btle_notifications = 0x00;//BTLE_NOTIFICATION;
	this->btle_connection = false;
	this->mtu_size = PAN1026_MTU_TX;
	this->SetBusy();

	bt_irqh(BT_IRQ_RESET, 0);

	bt_module_reset();
}

void pan1026::SetBusy(uint16_t timeout)
{
	this->busy = true;
	if (timeout > 0)
		this->busy_timer = task_get_ms_tick() + timeout;
	else
		this->busy_timer = BT_NO_TIMEOUT;
}

void pan1026::ClearBusy()
{
	DEBUG_BT("ClearBusy\n");
	this->busy = false;
	this->busy_timer = BT_NO_TIMEOUT;
}

void pan1026::SetNextStep(uint8_t cmd)
{
	this->next_cmd = cmd;
}

void pan1026::ParseHCI()
{
	uint16_t op_code;
	uint8_t status;

#ifdef DEBUG_BT_ENABLED
	DEBUG_BT("\n - HCI ----\n");
	for (uint8_t i = 0; i < this->parser_packet_length; i++)
		DEBUG_BT("%02X ", this->parser_buffer[i]);
	DEBUG_BT("\n");
#endif

	uint8_t event_code = this->parser_buffer[1];

	DEBUG_BT("event_code %02X\n", event_code);

	switch (event_code)
	{
		case(0x0e): // HCI Command Complete
			op_code = this->parser_buffer[4] | (this->parser_buffer[5] << 8);
			status =  this->parser_buffer[6];

			DEBUG_BT("op_code %04X\n", op_code);
			DEBUG_BT("status %02X\n", status);

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

		case(0x10): //Hardware error
			DEBUG_BT("Hardware Error event\n");
			switch (this->parser_buffer[3])
			{
				case(0x20):
					DEBUG_BT("Short of receiving packet\n");
				break;
				case(0x21):
					DEBUG_BT("Stop bit error\n");
				break;
				case(0x22):
					DEBUG_BT("Over write error\n");
				break;
			}

			this->repat_last_cmd = true;
			this->repeat_timer = task_get_ms_tick() + 10; //wait 10ms or more
			// PAN1026_ERROR;
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
								DEBUG_BT("Pan1026 firmware version %s\n", this->parser_buffer + 13);
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
								DEBUG_BT("MAC: ");
								memcpy(this->pan_mac_address, this->parser_buffer + 14, 6);
								for (uint8_t i = 0; i < 6; i++)
									DEBUG_BT("%02X ", this->pan_mac_address[i]);
								DEBUG_BT("\n");
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

#ifdef DEBUG_BT_ENABLED
	DEBUG_BT("\n - MNG ----\n");
	for (uint8_t i = 0; i < this->parser_packet_length; i++)
		DEBUG_BT("%02X ", this->parser_buffer[i]);
	DEBUG_BT("\n");
#endif

	op_code = this->parser_buffer[4];
	status = this->parser_buffer[7];

	DEBUG_BT("op_code: %02X\n", op_code);
	DEBUG_BT("status: %02X\n", status);

	switch (op_code)
	{

		case(0x47)://TCU_MNG_CONNECTION_STATUS_EVENT
			DEBUG_BT("TCU_MNG_CONNECTION_STATUS_EVENT\n");
			DEBUG_BT("Connection Status: \n");
			DEBUG_BT(" Status          %02X\n", status);
			DEBUG_BT(" MAC             ");
			for (uint8_t i = 0; i < 6; i++)
				DEBUG_BT("%02X ", this->parser_buffer[8 + 5 - i]);
			DEBUG_BT("\n");
			DEBUG_BT(" Connection status %02X - ", this->parser_buffer[14]);

			switch(this->parser_buffer[14])
			{
				case(0x00):
					DEBUG_BT("Connected");
				break;
				case(0x01):
					DEBUG_BT("Disconnected");
				break;
				case(0x02):
					DEBUG_BT("Connection Failure");
				break;
				case(0x03):
					DEBUG_BT("Link key");
				break;
				case(0x04):
					DEBUG_BT("Mode Change Active");
				break;
				case(0x05):
					DEBUG_BT("Mode Change Hold");
				break;
				case(0x06):
					DEBUG_BT("Mode Change Sniff");
				break;
				case(0x07):
					DEBUG_BT("Mode Change Park");
				break;
			}
			DEBUG_BT("\n");

			if (this->parser_buffer[14] == 0x03)
			{
				DEBUG_BT(" Link key        ");
				for (uint8_t i = 0; i < 16; i++)
					DEBUG_BT("%02X ", this->parser_buffer[15 + i]);
				DEBUG_BT("\n");
				DEBUG_BT(" Link key type   %02X\n", this->parser_buffer[33]);
				DEBUG_BT(" Sniff interval  %02X\n", this->parser_buffer[34] | (this->parser_buffer[35] << 8));

				DEBUG_BT("Storing link key\n");

				for (uint8_t i = 0; i < 16; i++)
					config.connectivity.bt_link_key[i] = this->parser_buffer[15 + i];

				eeprom_busy_wait();
				eeprom_update_block((void *)&config.connectivity.bt_link_key, &config_ee.connectivity.bt_link_key, 16);

				for (uint8_t i = 0; i < 6; i++)
				{
					config.connectivity.bt_link_partner[i] = this->parser_buffer[8 + i];
					DEBUG_BT("%02X ", config.connectivity.bt_link_partner[i]);
				}
				DEBUG_BT("\n");

				eeprom_busy_wait();
				eeprom_update_block((void *)&config.connectivity.bt_link_partner, &config_ee.connectivity.bt_link_partner, 6);


			}

			if (status == 0x87)
			{
				DEBUG_BT(" Link key failure\n");
				memset((void *)config.connectivity.bt_link_partner, 0, sizeof(config.connectivity.bt_link_partner));
				memset((void *)config.connectivity.bt_link_key, 0, sizeof(config.connectivity.bt_link_key));

				eeprom_busy_wait();
				eeprom_update_block((void *)&config.connectivity.bt_link_key, &config_ee.connectivity.bt_link_key, 16);
			}
		break;


		case(0x55):
			DEBUG_BT("Connection request from: \n ");
			memcpy(this->client_mac_address, this->parser_buffer + 7, 6);
			for (uint8_t i = 6; i > 0; i--)
				DEBUG_BT("%02X ", this->client_mac_address[i - 1]);
			DEBUG_BT("\n");

			this->SetNextStep(pan_cmd_accept_connection);

		break;

		case(0x6e): //TCU_MNG_REMOTE_DEVICE_NAME_AUTO_NOTIFY_EVENT
			DEBUG_BT("Client info\n");
			t_len = this->parser_buffer[13];
			DEBUG_BT("Device name len %d\n", t_len);
			if (t_len > sizeof(this->client_name) - 1)
				t_len = sizeof(this->client_name) - 1;

			this->client_name[t_len] = 0;
			memcpy(this->client_name, this->parser_buffer + 14, t_len);
			DEBUG_BT("Device name: %s\n", this->client_name);
		break;

		case(0x7d):
			hci_status = this->parser_buffer[6];
			hci_op_code = this->parser_buffer[7];

			DEBUG_BT("HCI status %02X\n", hci_status);
			DEBUG_BT("HCI op_code %02X\n", hci_op_code);

			switch(hci_op_code)
			{
				case(0x31)://HCI_IO_Capability_Request_Event
					this->SetNextStep(pan_cmd_io_cap_respose);
				break;

				case(0x32)://HCI_IO_Capability_Response_Event
					DEBUG_BT("IO_Capability_Response\n");
					DEBUG_BT(" IO_Capability    %02X\n", this->parser_buffer[15]);
					DEBUG_BT(" OOB_Data_Present %02X\n", this->parser_buffer[16]);
					DEBUG_BT(" Authentication_Requirement %02X\n", this->parser_buffer[16]);
				break;

				case(0x33)://HCI_IO_User_Confirmation_Request_Event
					this->SetNextStep(pan_cmd_confirmation_reply);
				break;

				case(0x36)://HCI_Simple_Pairing_Complete_Event
					DEBUG_BT("Simple_Pairing_Complete\n");
					DEBUG_BT(" Status %02X\n", this->parser_buffer[9]);
				break;
			}

		break;


		case(0x81): // TCU_MNG_INIT_RESPONSE
		DEBUG_BT("TCU_MNG_INIT_RESPONSE\n");
			if (status == 0)
				this->SetNextStep(pan_cmd_write_cod);
			else
				PAN1026_ERROR;
		break;

		case(0x8c): // TCU_MNG_SET_SCAN_RESP
			DEBUG_BT("TCU_MNG_SET_SCAN_RESP\n");
			if (status == 0)
			{
				DEBUG_BT(" sucess\n");
				this->SetNextStep(pan_cmd_le_mng_init);
			}
			else
				PAN1026_ERROR;
		break;

		case(0x93): //TCU_MNG_CONNECTION_ACCEPT_RESP
			DEBUG_BT("Pair status:\n ");
			switch(status)
			{
				case(0x00):
					DEBUG_BT("Successful\n");
				break;
				case(0x01):
					DEBUG_BT("Parameter Failure\n");
				break;
				case(0x03):
					DEBUG_BT("No Device Initialization\n");
				break;
				case(0x06):
					DEBUG_BT("No Connection\n");
				break;
			}
		break;

		case(0xBD): //Encapsulated HCI_RESP
			hci_status = this->parser_buffer[14];
			hci_op_code = this->parser_buffer[12] | (this->parser_buffer[13] << 8);

			DEBUG_BT("HCI status %02X\n", hci_status);
			DEBUG_BT("HCI op_code %04X\n", hci_op_code);

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

		case(0xF1)://TCU_ACCEPT
			DEBUG_BT("TCU_ACCEPT\n ");
			switch(status)
			{
				case(0x00):
					DEBUG_BT("Successful\n");
				break;
				case(0x01):
					DEBUG_BT("Parameter Failure\n");
				break;
				case(0x03):
					DEBUG_BT("No Device Initialization\n");
				break;
				case(0x04):
					DEBUG_BT("On Device Searching\n");
				break;
				case(0x05):
					DEBUG_BT("On Service Searching\n");
				break;
				case(0x0A):
					DEBUG_BT("Establish ACL connection\n");
				break;
				case(0x0C):
					DEBUG_BT("Not Connection Established yet\n");
				break;
				case(0x0D):
					DEBUG_BT("Connection with Multi-connection restricted device\n");
				break;
				case(0x40):
					DEBUG_BT("Setup SPP\n");
				break;
				case(0x41):
					DEBUG_BT("No setup SPP\n");
				break;
				case(0x42):
					DEBUG_BT("Establish SPP connection\n");
				break;
				case(0x43):
					DEBUG_BT("On releasing SPP connection\n");
				break;
				case(0x44):
					DEBUG_BT("No SPP connection\n");
				break;

				default:
					DEBUG_BT("Unknown status\n");
				break;
			}

		break;

		case(0xFF)://TCU_SYS_INVALID_COMMAND
			DEBUG_BT("TCU_SYS_INVALID_COMMAND\n");
			DEBUG_BT(" ServiceID_Received %02X\n", this->parser_buffer[7]);
			DEBUG_BT(" Opcode_Received  %02X\n", this->parser_buffer[8]);
		break;

	}
}

void pan1026::ParseSPP()
{

#ifdef DEBUG_BT_ENABLED
	DEBUG_BT("\n - SPP ----\n");
	for (uint8_t i = 0; i < this->parser_packet_length; i++)
		DEBUG_BT("%02X ", this->parser_buffer[i]);
	DEBUG_BT("\n");
#endif

	uint8_t op_code, status;
	op_code = this->parser_buffer[4];
	status = this->parser_buffer[7];

	byte2 tmp2;
	uint16_t i;

	DEBUG_BT("op_code: %02X\n", op_code);
	DEBUG_BT("status: %02X\n", status);

	switch(op_code)
	{
		case(0x81): //TCU_SPP_SETUP_RESP
		DEBUG_BT("TCU_SPP_SETUP_RESP\n");
			if (status == 0)
				this->SetNextStep(pan_cmd_listen);
			else
				PAN1026_ERROR;
		break;

		case(0x43): //TCU_SPP_CONNECT_EVENT
			DEBUG_BT("TCU_SPP_CONNECT_EVENT\n");
			if (status == 0)
			{
				bt_irqh(BT_IRQ_CONNECTED, 0);
				this->SetBusy(BT_TIMEOUT_CONN);
			}
			else
				PAN1026_ERROR;
		break;

		case(0x44): //TCU_SPP_DISCONNECT_EVENT
			DEBUG_BT("TCU_SPP_DISCONNECT_EVENT\n");
			if (status == 0)
			{
				bt_irqh(BT_IRQ_DISCONNECTED, 0);
				//this is done automatically by the module
				//this->SetNextStep(pan_cmd_le_start_advertise);
				this->mtu_size = PAN1026_MTU_TX;
			}
			else
				PAN1026_ERROR;
		break;

		case(0x48): // TCU_SPP_DATA_RECEIVE_EVENT
			DEBUG_BT("TCU_SPP_DATA_RECEIVE_EVENT\n");
			tmp2.uint8[0] = this->parser_buffer[7];
			tmp2.uint8[1] = this->parser_buffer[8];

			for (i = 0; i < tmp2.uint16; i++)
				protocol_rx(this->parser_buffer[9 + i]);
		break;

		case(0xF1): // TCU_SPP_DATA_SEND_EVENT
			DEBUG_BT("TCU_SPP_DATA_SEND_EVENT\n");

			this->ClearBusy();
		break;
	}
}

void pan1026::ParseMNG_LE()
{
	uint8_t status, op_code, tmp;

#ifdef DEBUG_BT_ENABLED
	DEBUG_BT("\n - MNG LE ----\n");
	for (uint8_t i = 0; i < this->parser_packet_length; i++)
		DEBUG_BT("%02X ", this->parser_buffer[i]);
	DEBUG_BT("\n");
#endif

	op_code = this->parser_buffer[4];
	status = this->parser_buffer[7];

	DEBUG_BT("op_code: %02X\n", op_code);
	DEBUG_BT("status: %02X\n", status);

	switch (op_code)
	{
		case(0x4C): // TCU_MNG_LE_CONNECTION_COMPLETE_EVENT
			DEBUG_BT("LE Connection complete event\n");
			DEBUG_BT(" role %02X\n", this->parser_buffer[10]);
			if (status == 0)
			{
				if (this->parser_buffer[10] != 0x01)
				{
					DEBUG_BT("Connection type is not slave, ignoring\n");
					break;
				}

				this->btle_connection_handle = this->parser_buffer[8] | (this->parser_buffer[9] << 8);
				DEBUG_BT("Handle 0x%04X\n", this->btle_connection_handle);

				bt_irqh(BT_IRQ_CONNECTED, NULL);
				this->btle_connection = true;

				//make room for MTU negotiation
				this->SetBusy(BT_TIMEOUT_CONN);

				this->SetNextStep(pan_cmd_le_mtu_req);
			}
			else
				PAN1026_ERROR;
		break;

		case(0x93): // TCU_MNG_LE_DISCONNECT_EVENT
#ifdef DEBUG_BT_ENABLED
			DEBUG_BT("LE Disconnection event\n");
		 	uint16_t handle = this->parser_buffer[7] | (this->parser_buffer[8] << 8);
			DEBUG_BT("Handle 0x%04X\n", handle);
#endif
			bt_irqh(BT_IRQ_DISCONNECTED, NULL);
			this->btle_connection = false;

			this->SetNextStep(pan_cmd_le_start_advertise);
		break;

		case(0x81): // TCU_MNG_LE_INIT_RESPONSE
			if (status == 0)
			{
				DEBUG_BT("MNG LE init success\n");
				this->SetNextStep(pan_cmd_le_gat_ser_init);
			}
			else
				PAN1026_ERROR;
		break;

		case(0x88): // TCU_MNG_LE_START_ADVERTISE_RESP
			DEBUG_BT("Start advertise: ");
			switch (status)
			{
				case(0x00):
					DEBUG_BT("success\n");
					tmp = BT_PAN1026;
					bt_irqh(BT_IRQ_INIT_OK, &tmp);
				break;
				case(0x86):
					DEBUG_BT("Parameter error\n");
				break;
				case(0x81):
					DEBUG_BT("Dev Not Initialized\n");
				break;
				case(0x82):
					DEBUG_BT("Command in Progress\n");
				break;
				case(0xA4):
					DEBUG_BT("Device Already Advertising\n");
				break;
			}
		break;

		case(0x84): //TCU_MNG_LE_SET_RAND_ADDRESS_RESP
			DEBUG_BT("TCU_MNG_LE_SET_RAND_ADDRESS_RESP\n");
			if (status == 0x00)
			{
				DEBUG_BT(" sucess\n");
				this->cmd_iter = 0;
				this->SetNextStep(pan_cmd_le_gat_sdb_add_primary_service);
			}
			else
				PAN1026_ERROR;
		break;

		case(0x54): //TCU_MNG_LE_GEN_RESOLVABLE_BDADDR_RESP
			DEBUG_BT("TCU_MNG_LE_GEN_RESOLVABLE_BDADDR_RESP\n");
			if (status == 0x00)
			{
				DEBUG_BT(" sucess\n");

				this->SetNextStep(pan_cmd_le_set_random_address);

				bool store_mac = true;

				for (uint8_t i = 0; i < 6; i++)
					if (config.connectivity.btle_mac[i] != 0)
					{
						store_mac = false;
						break;
					}


				if (!store_mac)
					break;

				DEBUG_BT("Storing new random address...\n");
				memcpy((void *)config.connectivity.btle_mac, &this->parser_buffer[8], 6);
				eeprom_busy_wait();
				eeprom_update_block((void *)&config.connectivity.btle_mac, &config_ee.connectivity.btle_mac, 6);
			}
			else
				PAN1026_ERROR;
		break;

		case(0xF1): //TCU_LE_ACCEPT
			DEBUG_BT("TCU_LE_ACCEPT\n");
			DEBUG_BT(" ServiceID_Received %02X\n", this->parser_buffer[8]);
			DEBUG_BT(" Command_OpCode_Received %02X\n", this->parser_buffer[9]);

			switch (status)
			{
				case(0x00): //Success
					return;
				break;

				case(0x01): //The abnormalities in a parameter
					//if it is start advertise problem than reboot the start up sequence
					if (this->parser_buffer[8] == 0xD1 && this->parser_buffer[9] == 0x08)
					{
						PAN1026_ERROR;
						return;
					}
				break;

				case(0x07): //LE_Error, reboot the module
					PAN1026_ERROR;
					return;
				break;
			}

			//have som time to recover from boo boo
			this->SetBusy(BT_TIMEOUT_TCU_ERR);
		break;

		case(0xFF): //TCU_LE_SYS_INVALID_COMMAND
			DEBUG_BT("MNG LE command from Host CPU is invalid\n");
			DEBUG_BT(" ServiceID_Received %02X\n", this->parser_buffer[7]);
			DEBUG_BT(" Command_OpCode_Received %02X\n", this->parser_buffer[8]);
		break;
	}
}

void pan1026::ParseGAT_ser()
{
	uint8_t status, op_code;
	uint16_t handle, len;

#ifdef DEBUG_BT_ENABLED
	DEBUG_BT("\n - GAT ser ----\n");
	for (uint8_t i = 0; i < this->parser_packet_length; i++)
		DEBUG_BT("%02X ", this->parser_buffer[i]);
	DEBUG_BT("\n");
#endif

	op_code = this->parser_buffer[4];
	status = this->parser_buffer[7];

	DEBUG_BT("op_code: %02X\n", op_code);
	DEBUG_BT("status: %02X\n", status);

	switch (op_code)
	{
		case(0x80): // TCU_LE_GATT_SER_INIT_RESP
			if (status == 0)
			{
				DEBUG_BT("GAT ser init success\n");
				this->SetNextStep(pan_cmd_le_gat_cli_init);
			}
			else
				PAN1026_ERROR;
		break;

		case(0xA0): // TCU_LE_GATT_SDB_ADD_PRIM_SVC_RESP
			if (status == 0)
			{
				DEBUG_BT("GAT add primary service success\n");
				this->btle_service_handles[this->cmd_iter] = this->parser_buffer[8] | (this->parser_buffer[9] << 8);
				DEBUG_BT("Handle 0x%04X\n", this->btle_service_handles[this->cmd_iter]);
				this->cmd_iter++;
				if (this->cmd_iter < 3) // 3
				{
					this->SetNextStep(pan_cmd_le_gat_sdb_add_primary_service);
				}
				else
				{
					this->SetNextStep(pan_cmd_le_gat_sdb_add_characteristic);
					this->cmd_iter = 0;
				}
			}
			else
				PAN1026_ERROR;
		break;

		case(0xA2): // TCU_LE_GATT_SDB_ADD_CHAR_DECL_RESP
			if (status == 0)
			{
				DEBUG_BT("GAT add characteristic success\n");
				this->btle_characteristic_handles[this->cmd_iter] = this->parser_buffer[8] | (this->parser_buffer[9] << 8);
				DEBUG_BT("Handle 0x%04X\n", this->btle_characteristic_handles[this->cmd_iter]);

				this->cmd_iter++;
				if (this->cmd_iter < 6)//6
				{
					this->SetNextStep(pan_cmd_le_gat_sdb_add_characteristic);
				}
				else
				{
					this->cmd_iter = 0;
					this->SetNextStep(pan_cmd_le_gat_sdb_add_characteristic_elements);
				}
			}
			else
				PAN1026_ERROR;
		break;

		case(0xA3): // TCU_LE_GATT_SDB_ADD_CHAR_ELE_RESP
			if (status == 0)
			{
				DEBUG_BT("GAT add characteristic element success\n");
				this->btle_characteristic_element_handles[this->cmd_iter] = this->parser_buffer[8] | (this->parser_buffer[9] << 8);
				DEBUG_BT("Handle 0x%04X\n", this->btle_characteristic_element_handles[this->cmd_iter]);

				this->cmd_iter++;
				if (this->cmd_iter < 7) //7
				{
					this->SetNextStep(pan_cmd_le_gat_sdb_add_characteristic_elements);
				}
				else
				{
					this->cmd_iter = 0;
					this->SetNextStep(pan_cmd_le_start_advertise);
				}
			}
			else
				PAN1026_ERROR;
		break;

		case(0xC1): //TCU_LE_GATT_SER_EXG_MTU_EVENT
			DEBUG_BT("TCU_LE_GATT_SER_EXG_MTU_EVENT\n");
			handle = this->parser_buffer[7] | (this->parser_buffer[8] << 8);
			DEBUG_BT(" conn handle %04X\n", handle);
			handle = this->parser_buffer[9] | (this->parser_buffer[10] << 8);
			DEBUG_BT(" MTU size %04X\n", handle);

			if (handle < this->mtu_size)
				this->mtu_size = handle;

			//accept request
			this->SetNextStep(pan_cmd_le_mtu_accept);
		break;

		case(0xC2): //TCU_LE_GATT_SER_READ_CHAR_VAL_EVENT
			DEBUG_BT("TCU_LE_GATT_SER_READ_CHAR_VAL_EVENT\n");
			handle = this->parser_buffer[7] | (this->parser_buffer[8] << 8);
			DEBUG_BT(" conn handle %04X\n", handle);
			handle = this->parser_buffer[9] | (this->parser_buffer[10] << 8);
			DEBUG_BT(" char handle %04X\n", handle);

			//update characteristic

			//accept request
			this->SetNextStep(pan_cmd_le_read_val_accept);
		break;

		case(0xC3): //TCU_LE_GATT_SER_WRITE_CHAR_VAL_EVENT
			DEBUG_BT("TCU_LE_GATT_SER_WRITE_CHAR_VAL_EVENT\n");
			len = this->parser_buffer[5] | (this->parser_buffer[6] << 8);
			DEBUG_BT(" param len %d\n", len);
			handle = this->parser_buffer[7] | (this->parser_buffer[8] << 8);
			DEBUG_BT(" conn handle %04X\n", handle);
			handle = this->parser_buffer[9] | (this->parser_buffer[10] << 8);
			DEBUG_BT(" char handle %04X\n", handle);

			//data!
			DEBUG_BT(" data: ");
			for (uint16_t i = 0; i < len - 4; i++)
			{
				DEBUG_BT("%c", this->parser_buffer[11 + i]);
				protocol_rx(this->parser_buffer[11 + i]);
			}
			DEBUG_BT("\n");

			//accept request
			this->SetNextStep(pan_cmd_le_write_val_accept);
		break;

		case(0xCA): //TCU_LE_GATT_SER_READ_MULTIPLE_EVENT
			DEBUG_BT("TCU_LE_GATT_SER_READ_MULTIPLE_EVENT\n");
			handle = this->parser_buffer[7] | (this->parser_buffer[8] << 8);
			DEBUG_BT(" conn handle %04X\n", handle);

			//accept request
			this->SetNextStep(pan_cmd_le_read_multiple_accept);
		break;

		case(0x82): //TCU_LE_GATT_SER_READ_CHAR_VAL_ACCEPT_RESP
			DEBUG_BT("TCU_LE_GATT_SER_READ_CHAR_VAL_ACCEPT_RESP\n");
			handle = this->parser_buffer[7] | (this->parser_buffer[8] << 8);
			DEBUG_BT(" handle %04X\n", handle);
			DEBUG_BT(" status %02X\n", this->parser_buffer[9]);

			//release
			this->SetBusy(BT_TIMEOUT_ACCEPT);

		break;

		case(0x84): //TCU_LE_GATT_SER_WRITE_CHAR_DESP_ACCEPT_RESP
			DEBUG_BT("TCU_LE_GATT_SER_WRITE_CHAR_DESP_ACCEPT_RESP\n");
			handle = this->parser_buffer[7] | (this->parser_buffer[8] << 8);
			DEBUG_BT(" handle %04X\n", handle);
			DEBUG_BT(" status %02X\n", this->parser_buffer[9]);

			//release
			this->SetBusy(BT_TIMEOUT_ACCEPT);
		break;

		case(0x8A): //TCU_LE_GATT_SER_READ_MULTIPLE_ACCEPT_RESP
			DEBUG_BT("TCU_LE_GATT_SER_READ_MULTIPLE_ACCEPT_RESP\n");
			handle = this->parser_buffer[7] | (this->parser_buffer[8] << 8);
			DEBUG_BT(" handle %04X\n", handle);
			DEBUG_BT(" status %02X\n", this->parser_buffer[9]);

			//release
			this->SetBusy(BT_TIMEOUT_ACCEPT);
		break;

		case(0xC8)://TCU_LE_GATT_SER_READ_CHAR_DESP_EVENT
			DEBUG_BT("TCU_LE_GATT_SER_READ_CHAR_DESP_EVENT\n");

			handle = this->parser_buffer[7] | (this->parser_buffer[8] << 8);
			DEBUG_BT(" conn handle %04X\n", handle);
			handle = this->parser_buffer[9] | (this->parser_buffer[10] << 8);
			DEBUG_BT(" char desc handle %04X\n", handle);

			//accept request
			this->SetNextStep(pan_cmd_le_read_char_des_accept);
		break;

		case(0x83)://TCU_LE_GATT_SER_WRITE_CHAR_VAL_ACCEPT_RESP
			DEBUG_BT("TCU_LE_GATT_SER_WRITE_CHAR_VAL_ACCEPT_RESP\n");

			handle = this->parser_buffer[7] | (this->parser_buffer[8] << 8);
			DEBUG_BT(" conn handle %04X\n", handle);
			DEBUG_BT(" status %02X\n", this->parser_buffer[9]);

			//release
			this->SetBusy(BT_TIMEOUT_ACCEPT);
		break;

		case(0x88)://TCU_LE_GATT_SER_READ_CHAR_DESP_ACCEPT_RESP
			DEBUG_BT("TCU_LE_GATT_SER_READ_CHAR_DESP_ACCEPT_RESP\n");

			handle = this->parser_buffer[7] | (this->parser_buffer[8] << 8);
			DEBUG_BT(" conn handle %04X\n", handle);
			DEBUG_BT(" status %02X\n", this->parser_buffer[9]);

			//release
			this->SetBusy(BT_TIMEOUT_ACCEPT);
		break;

		case(0x45)://TCU_LE_GATT_SER_CHAR_VAL_NOTIFICATION_EVENT
			DEBUG_BT("TCU_LE_GATT_SER_CHAR_VAL_NOTIFICATION_EVENT\n");

			handle = this->parser_buffer[7] | (this->parser_buffer[8] << 8);
			DEBUG_BT(" conn handle %04X\n", handle);

			bt_output.Forward(this->last_send_len);
			this->last_send_len = 0;

			this->ClearBusy();
		break;

		case(0x46)://TCU_LE_GATT_SER_CHAR_VAL_INDICATION_EVENT
			DEBUG_BT("TCU_LE_GATT_SER_CHAR_VAL_INDICATION_EVENT\n");

			handle = this->parser_buffer[7] | (this->parser_buffer[8] << 8);
			DEBUG_BT(" conn handle %04X\n", handle);
			DEBUG_BT(" status %02X\n", this->parser_buffer[9]);

			bt_output.Forward(this->last_send_len);
			this->last_send_len = 0;

			this->ClearBusy();
		break;

		case(0xA5)://TCU_LE_GATT_SDB_UPD_CHAR_ELE_RESP
			DEBUG_BT("TCU_LE_GATT_SDB_UPD_CHAR_ELE_RESP\n");

			//accept request
			this->SetNextStep(pan_cmd_le_write_char_des_accept);
		break;

		case(0xC4): //TCU_LE_GATT_SER_WRITE_CHAR_DESP_EVENT
			DEBUG_BT("TCU_LE_GATT_SER_WRITE_CHAR_DESP_EVENT\n");
			len = this->parser_buffer[5] | (this->parser_buffer[6] << 8);
			DEBUG_BT(" param len %d\n", len);
			handle = this->parser_buffer[7] | (this->parser_buffer[8] << 8);
			DEBUG_BT(" conn handle %04X\n", handle);
			handle = this->parser_buffer[9] | (this->parser_buffer[10] << 8);
			DEBUG_BT(" char handle %04X\n", handle);

			//data!
			#ifdef DEBUG_BT_ENABLED
				DEBUG_BT(" data: ");
				for (uint16_t i = 0; i < len - 4; i++)
					DEBUG_BT("%c", this->parser_buffer[11 + i]);
				DEBUG_BT("\n");
			#endif

			if (handle == this->btle_characteristic_element_handles[CHAR_SPP_SPP_DESC])
				this->btle_notifications = this->parser_buffer[11] | (this->parser_buffer[12] << 8);

			this->SetBusy();

			//update
			this->SetNextStep(pan_cmd_le_update_char_element);
		break;

		case(0x81)://TCU_LE_GATT_SER_EXG_MTU_ACCEPT_RESP
			DEBUG_BT("TCU_LE_GATT_SER_EXG_MTU_ACCEPT_RESP\n");

			handle = this->parser_buffer[7] | (this->parser_buffer[8] << 8);
			DEBUG_BT(" conn handle %04X\n", handle);
			DEBUG_BT(" status %02X\n", this->parser_buffer[9]);
			handle = this->parser_buffer[10] | (this->parser_buffer[11] << 8);
			DEBUG_BT(" MTU size %u\n", handle);

			if (handle < this->mtu_size)
				this->mtu_size = handle;
		break;
	}
}

void pan1026::ParseGAT_cli()
{
	uint8_t status, op_code;
	uint16_t handle;

#ifdef DEBUG_BT_ENABLED
	DEBUG_BT("\n - GAT cli ----\n");
	for (uint8_t i = 0; i < this->parser_packet_length; i++)
		DEBUG_BT("%02X ", this->parser_buffer[i]);
	DEBUG_BT("\n");
#endif

	op_code = this->parser_buffer[4];
	status = this->parser_buffer[7];

	DEBUG_BT("op_code: %02X\n", op_code);
	DEBUG_BT("status: %02X\n", status);

	switch (op_code)
	{
		case(0x80): // TCU_LE_GATT_SER_INIT_RESP
			if (status == 0)
			{
				DEBUG_BT("GAT cli init success\n");
				this->cmd_iter = 0;
				this->SetNextStep(pan_cmd_le_gen_random_address);
			}
			else
				PAN1026_ERROR;
		break;

		case(0x41): //TCU_LE_GATT_CLI_EXG_MTU_EVENT
			DEBUG_BT("TCU_LE_GATT_CLI_EXG_MTU_EVENT\n");
			handle = this->parser_buffer[7] | (this->parser_buffer[8] << 8);
			DEBUG_BT(" conn handle %04X\n", handle);
			DEBUG_BT(" status %02X\n", this->parser_buffer[9]);
			if (this->parser_buffer[9] != 0x00) //if status is not Success
			{
				this->mtu_size = PAN1026_MTU_TX_FALLBACK;
				break;
			}

			handle = this->parser_buffer[10] | (this->parser_buffer[11] << 8);
			DEBUG_BT(" MTU size %u\n", handle);

			if (handle < this->mtu_size)
				this->mtu_size = handle;
		break;
	}
}

void pan1026::Parse(uint8_t c)
{
//	DEBUG_BT("1026<%02X %c\n", c, c);

	if (this->usart->rx_ovf)
	{
		DEBUG_BT("RX OVF\n");
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
						DEBUG_BT("WARNING len = %lu\n", tmp_len);

						DEBUG_BT(" %02X %02X %02X\n", this->parser_buffer[0], this->parser_buffer[1], this->parser_buffer[2]);

						//start timeout only if is the module fully initialized
						if (bt_device_active())
						{
							DEBUG_BT("connection is active!\nDO NOT PANIC\njust drop it");
							this->parser_status = pan_parser_wait;
							this->parser_timer = task_get_ms_tick() + BT_TIMEOUT_TCU_ERR;
							this->SetBusy(BT_TIMEOUT_TCU_ERR);
							break;
						}
						else
							PAN1026_ERROR;

					}
					this->parser_packet_length = tmp_len;
					this->parser_status = pan_tcu_packet;
				}
			}
		break;

		case(pan_parser_wait):
			if (this->parser_timer < task_get_ms_tick())
			{
				this->parser_buffer[0] = c;
				this->parser_buffer_index = 1;
				this->parser_status = pan_parser_head;
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

					case (0xD1):
						this->ParseMNG_LE();
					break;

					case (0xD2):
						this->ParseGAT_cli();
					break;

					case (0xD3):
						this->ParseGAT_ser();
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

const uint8_t PROGMEM TCU_MNG_LE_INIT_REQ[] = {0xd1, 0x01};
const uint8_t PROGMEM TCU_LE_GATT_SER_INIT_REQ[] = {0x07, 0x00, 0x00, 0xd3, 0x00, 0x00, 0x00};
const uint8_t PROGMEM TCU_LE_GATT_CLI_INIT_REQ[] = {0x07, 0x00, 0x00, 0xd2, 0x00, 0x00, 0x00};

const uint8_t PROGMEM tcu_mng_le_start_advertise_direct_address[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/* UUID e079c6a0-aa8b-11e3-a903-0002a5d5c51b */
const uint8_t PROGMEM spp_over_ble_service_uuid[] = {0x1b, 0xc5, 0xd5, 0xa5, 0x02, 0x00, 0x03, 0xa9, 0xe3, 0x11, 0x8b, 0xaa, 0xa0, 0xc6, 0x79, 0xe0};
const uint8_t PROGMEM spp_over_ble_characteristic_uuid[] = {0x1b, 0xc5, 0xd5, 0xa5, 0x02, 0x00, 0xef, 0x9c, 0xe3, 0x11, 0x89, 0xaa, 0xc0, 0x12, 0x83, 0xb3};

const uint8_t PROGMEM manufacturer_name[] = {'S', 'k', 'y', 'B', 'e', 'a', 'n'};
const uint8_t PROGMEM device_name_spp[] = {'S', 'k', 'y', 'D', 'r', 'o', 'p', ' ', 'S', 'P', 'P'};
const uint8_t PROGMEM device_name_ble[] = {'S', 'k', 'y', 'D', 'r', 'o', 'p', ' ', 'B', 'L', 'E'};

const uint8_t PROGMEM TCU_MNG_LE_GEN_RESOLVABLE_BDADDR_REQ[] = {0x07, 0x00, 0x00, 0xD1, 0x17, 0x00, 0x00};
const uint8_t PROGMEM TCU_MNG_LE_SET_RAND_ADDRESS_REQ[] = {0x0D, 0x00, 0x00, 0xD1, 0x04, 0x06, 0x00};

#define RAW(data) \
	this->RawSendStatic(data, sizeof(data))

#define TCU_LEN(len) \
	do{\
	this->StreamWrite(((len) & 0x000000FF) >> 0); \
	this->StreamWrite(((len) & 0x0000FF00) >> 8); \
	this->StreamWrite(((len) & 0x00FF0000) >> 16);}while(0);

#define WRITE_16B(len) \
	do{\
	this->StreamWrite(((len) & 0x00FF) >> 0); \
	this->StreamWrite(((len) & 0xFF00) >> 8);}while(0);


void pan1026::RawSendStatic(const uint8_t * data, uint8_t len)
{
	for(uint8_t i = 0; i < len; i++)
		this->StreamWrite(pgm_read_byte(&data[i]));
}

void pan1026::Step()
{
	uint16_t t_len;
	uint8_t use_link_key;

	while (!this->usart->isRxBufferEmpty())
	{
		this->Parse(this->usart->Read());
	}

	if (this->next_cmd != pan_cmd_none)
	{
		DEBUG_BT("\n\n< CMD %d\n", this->next_cmd);

		switch(this->next_cmd)
		{
			//init
			case(pan_cmd_reset):
				DEBUG_BT("pan_cmd_reset\n");
				RAW(TCU_HCI_RESET_REQ);
			break;

			case(pan_cmd_fw):
				DEBUG_BT("pan_cmd_fw\n");
				RAW(TCU_HCI_GET_FIRMWARE_VERSION_REQ);
			break;

			case(pan_cmd_en_i2c):
				DEBUG_BT("pan_cmd_en_i2c\n");
				RAW(TCU_HCI_M2_BTL_SET_I2C_ENABLE_REQ);
			break;

			case(pan_cmd_eeprom_write_en):
				DEBUG_BT("pan_cmd_eeprom_write_en\n");
				RAW(TCU_HCI_M2_BTL_EEPROM_WRITE_ENABLE_REQ);
			break;

			case(pan_cmd_eeprom_read):
				DEBUG_BT("pan_cmd_eeprom_read\n");
				RAW(TCU_HCI_M2_GENERAL_READ_EEPROM_REQ);
			break;

			case(pan_cmd_write_mac):
				DEBUG_BT("pan_cmd_write_mac\n");
				RAW(TCU_HCI_WRITE_BD_ADDR_REQ);
				//add mac
				for (uint8_t i = 6; i > 0; i--)
					this->StreamWrite(this->pan_mac_address[i-1]);
			break;

			case(pan_cmd_set_mode):
				DEBUG_BT("pan_cmd_set_mode\n");
				_delay_ms(100);
				RAW(TCU_HCI_SET_MODE_REQ);
			break;

			//classic
			case(pan_cmd_mng_init):
				DEBUG_BT("pan_cmd_mng_init\n");
				t_len = 3 + sizeof(TCU_MNG_INIT_REQ) + 5 + sizeof(device_name_spp);
				TCU_LEN(t_len);

				RAW(TCU_MNG_INIT_REQ);

				t_len = 3 + sizeof(device_name_spp);
				WRITE_16B(t_len);
				this->StreamWrite(0x04); //SPP
				this->StreamWrite(0x02); //sniff settings
				this->StreamWrite(sizeof(device_name_spp)); //label name len
				RAW(device_name_spp);
			break;

			case(pan_cmd_write_cod):
				DEBUG_BT("pan_cmd_write_cod\n");
				RAW(TCU_MNG_STANDARD_HCI_SET_REQ_Write_Class_of_Device);
			break;

			case(pan_cmd_spp_setup):
				DEBUG_BT("pan_cmd_spp_setup\n");
				RAW(TCU_SPP_SETUP_REQ);
			break;

			case(pan_cmd_listen):
				DEBUG_BT("pan_cmd_listen\n");
				RAW(TCU_MNG_SET_SCAN_REQ);
			break;

			case(pan_cmd_accept_connection):
				DEBUG_BT("pan_cmd_accept_connection\n");
				use_link_key = 1;

				DEBUG_BT("stored: ");
				for (uint8_t i = 0; i < 6; i++)
					DEBUG_BT("%02X ", config.connectivity.bt_link_partner[i]);

				DEBUG_BT("\n");
				DEBUG_BT("client: ");
				for (uint8_t i = 0; i < 6; i++)
					DEBUG_BT("%02X ", this->client_mac_address[i]);
				DEBUG_BT("\n");

				for (uint8_t i = 0; i < 6; i++)
				{
					if (config.connectivity.bt_link_partner[i] != this->client_mac_address[i])
					{
						use_link_key = 0;
						break;
					}
				}

				t_len = 3 + sizeof(TCU_MNG_CONNECTION_ACCEPT_REQ) + 2 + 8 + ((use_link_key) ? 16 : 0);
				TCU_LEN(t_len);

				RAW(TCU_MNG_CONNECTION_ACCEPT_REQ);

				t_len = 1 + 6 + 1 + ((use_link_key) ? 16 : 0);
				WRITE_16B(t_len);

				this->StreamWrite(0x00); //Accept
				//add client mac
				for (uint8_t i = 0; i < 6; i++)
					this->StreamWrite(this->client_mac_address[i]);

				if (use_link_key)
				{
					DEBUG_BT("\nUsing link key: ");
					this->StreamWrite(0x01); //use link key
					for (uint8_t i = 0; i < 16; i++)
					{
						this->StreamWrite(config.connectivity.bt_link_key[i]);
						DEBUG_BT("%02X ", config.connectivity.bt_link_key[i]);
					}
					DEBUG_BT("\n");
				}
				else
				{
					this->StreamWrite(0x00); //do not use link key
					DEBUG_BT("\nNot using link key\n");
				}
			break;

			case(pan_cmd_io_cap_respose):
				DEBUG_BT("pan_cmd_io_cap_respose\n");
				RAW(TCU_MNG_SSP_SET_REQ_HCI_IO_Capability_Request_Reply);
				//add client mac
				for (uint8_t i = 0; i < 6; i++)
					this->StreamWrite(this->client_mac_address[i]);
				this->StreamWrite(0x03); //NoInputNoOutput
				this->StreamWrite(0x00); //OOB authentication data not present XXX tuto dat 1 ak to bude mat
				this->StreamWrite(0x03); //MITM Protection Not Required – No Bonding. Numeric comparison with automatic accept allowed.
			break;

			case(pan_cmd_confirmation_reply):
				DEBUG_BT("pan_cmd_confirmation_reply\n");
				RAW(TCU_MNG_SSP_SET_REQ_HCI_User_Confirmation_Request_Reply);
	//			RAW(TCU_MNG_SSP_SET_REQ_HCI_User_Confirmation_Negative_Reply);
				//add client mac
				for (uint8_t i = 0; i < 6; i++)
					this->StreamWrite(this->client_mac_address[i]);
				this->StreamWrite(0x00); //???

			break;

			//btle
			case(pan_cmd_le_mng_init):
				DEBUG_BT("pan_cmd_le_mng_init\n");
				t_len = 3 + sizeof(TCU_MNG_LE_INIT_REQ) + 3 + sizeof(device_name_ble);
				TCU_LEN(t_len);

				RAW(TCU_MNG_LE_INIT_REQ);

				t_len = 1 + sizeof(device_name_ble);
				WRITE_16B(t_len);
				this->StreamWrite(sizeof(device_name_ble)); //label name len
				RAW(device_name_ble); //label name
			break;

			case(pan_cmd_le_gat_ser_init):
				DEBUG_BT("pan_cmd_le_gat_ser_init\n");
				RAW(TCU_LE_GATT_SER_INIT_REQ);
			break;

			case(pan_cmd_le_gat_cli_init):
				DEBUG_BT("pan_cmd_le_gat_cli_init\n");
				RAW(TCU_LE_GATT_CLI_INIT_REQ);
			break;

			case(pan_cmd_le_gat_sdb_add_primary_service):
				DEBUG_BT("pan_cmd_le_gat_sdb_add_primary_service %d\n", this->cmd_iter);
				t_len = 3 + 5 + ((this->cmd_iter != 2) ? 2 : 16);
				TCU_LEN(t_len);

				//ServiceID
				this->StreamWrite(0xD3);
				//OpCode
				this->StreamWrite(0x20);
				//Param length
				WRITE_16B(t_len - 7);

				//Attribute Value Length
				this->StreamWrite(t_len - 8);
				if (this->cmd_iter == 0)
					WRITE_16B(0x1800); //generic_access

				if (this->cmd_iter == 1)
					WRITE_16B(0x180A); //device_information

				if (this->cmd_iter == 2)
					RAW(spp_over_ble_service_uuid); //SPP uuid 16B

			break;

			case(pan_cmd_le_gat_sdb_add_characteristic):
				DEBUG_BT("pan_cmd_le_gat_sdb_add_characteristic %d\n", this->cmd_iter);
				//t_len =  3 + 4 + 4 + ((this->cmd_iter != 5) ? 2 : 16);
				t_len =  3 + 4 + 4 + 2;
				TCU_LEN(t_len);

				//ServiceID
				this->StreamWrite(0xD3);
				//OpCode
				this->StreamWrite(0x22);
				//Param length
				WRITE_16B(t_len - 7);

				//generic_access -> device_name
				if (this->cmd_iter == 0)
				{
					//handle
					WRITE_16B(this->btle_service_handles[SERVICE_GENERIC]);
					//Characteristic Properties
					this->StreamWrite(0x02); //READ
					//UUID Length
					this->StreamWrite(0x02);
					//UUID Value
					WRITE_16B(0x2A00);
				}

				//generic_access -> appearance
				if (this->cmd_iter == 1)
				{
					//handle
					WRITE_16B(this->btle_service_handles[SERVICE_GENERIC]);
					//Characteristic Properties
					this->StreamWrite(0x02); //READ
					//UUID Length
					this->StreamWrite(0x02);
					//UUID Value
					WRITE_16B(0x2A01);
				}

				//device_information -> manufacturer_name
				if (this->cmd_iter == 2)
				{
					//handle
					WRITE_16B(this->btle_service_handles[SERVICE_DEVICE]);
					//Characteristic Properties
					this->StreamWrite(0x02); //READ
					//UUID Length
					this->StreamWrite(0x02);
					//UUID Value
					WRITE_16B(0x2A29);
				}

				//device_information -> model_number
				if (this->cmd_iter == 3)
				{
					//handle
					WRITE_16B(this->btle_service_handles[SERVICE_DEVICE]);
					//Characteristic Properties
					this->StreamWrite(0x02); //READ
					//UUID Length
					this->StreamWrite(0x02);
					//UUID Value
					WRITE_16B(0x2A24);
				}

				//device_information -> system_id
				if (this->cmd_iter == 4)
				{
					//handle
					WRITE_16B(this->btle_service_handles[SERVICE_DEVICE]);
					//Characteristic Properties
					this->StreamWrite(0x02); //READ
					//UUID Length
					this->StreamWrite(0x02);
					//UUID Value
					WRITE_16B(0x2A23);
				}

				//spp_over_ble -> spp_over_ble
				if (this->cmd_iter == 5)
				{
					//handle
					WRITE_16B(this->btle_service_handles[SERVICE_SPP]);
					//Characteristic Properties
					this->StreamWrite(0x18); //NOTIFY | WRITE
					//UUID Length
					this->StreamWrite(0x2);
					//UUID Value
					WRITE_16B(0xFFE1);
				}
			break;

			case(pan_cmd_le_gat_sdb_add_characteristic_elements):
				DEBUG_BT("pan_cmd_le_gat_sdb_add_characteristic_elements %d\n", this->cmd_iter);
				t_len = 3 + 4 + 7;
				if (this->cmd_iter == 0)
					t_len += 2 + sizeof(device_name_ble);
				if (this->cmd_iter == 1)
					t_len += 2 + 2;
				if (this->cmd_iter == 2)
					t_len += 2 + sizeof(manufacturer_name);
				if (this->cmd_iter == 3)
					t_len += 2 + 9;
				if (this->cmd_iter == 4)
					t_len += 2 + 8;
				if (this->cmd_iter == 5)
					t_len += 2 + PAN1026_BUFFER_SIZE;
				if (this->cmd_iter == 6)
					t_len += 2 + 2;

				TCU_LEN(t_len);

				//ServiceID
				this->StreamWrite(0xD3);
				//OpCode
				this->StreamWrite(0x23);
				//Param length
				WRITE_16B(t_len - 7);

				//generic_access -> device_name
				if (this->cmd_iter == 0)
				{
					//Characteristic handle
					WRITE_16B(this->btle_characteristic_handles[CHAR_GENERIC_NAME]);
					//Attribute Type Length
					this->StreamWrite(0x02);
					//Attribute Type
					WRITE_16B(0x2A00);
					//Attribute Value Length
					WRITE_16B(sizeof(device_name_ble));
					//Attribute Value
					RAW(device_name_ble);
					//Permissions
					WRITE_16B(0);
				}

				//generic_access -> appearance
				if (this->cmd_iter == 1)
				{
					//Characteristic handle
					WRITE_16B(this->btle_characteristic_handles[CHAR_GENERIC_APPEARANCE]);
					//Attribute Type Length
					this->StreamWrite(0x02);
					//Attribute Type
					WRITE_16B(0x2A01);
					//Attribute Value Length
					WRITE_16B(2);
					//Attribute Value
					WRITE_16B(5186); //APPEARANCE_CATEGORY_LOCATION_AND_NAVIGATION_DISPLAY_DEVICE
					//Permissions
					WRITE_16B(0);
				}

				//device_information -> manufacturer_name
				if (this->cmd_iter == 2)
				{
					//Characteristic handle
					WRITE_16B(this->btle_characteristic_handles[CHAR_DEVICE_MANUFACTURER]);
					//Attribute Type Length
					this->StreamWrite(0x02);
					//Attribute Type
					WRITE_16B(0x2A29);
					//Attribute Value Length
					WRITE_16B(sizeof(manufacturer_name));
					//Attribute Value
					RAW(manufacturer_name);
					//Permissions
					WRITE_16B(0);
				}

				//device_information -> model_number
				if (this->cmd_iter == 3)
				{
					//Characteristic handle
					WRITE_16B(this->btle_characteristic_handles[CHAR_DEVICE_MODEL_NUMBER]);
					//Attribute Type Length
					this->StreamWrite(0x02);
					//Attribute Type
					WRITE_16B(0x2A24);
					//Attribute Value Length
					WRITE_16B(9);
					//Attribute Value
					char tmp[10];
					sprintf(tmp, "drop_%d", (hw_revision == HW_REW_1504) ? 1504 : 1506);
					for (uint8_t i = 0; i < 9; i++)
						this->StreamWrite(tmp[i]);
					//Permissions
					WRITE_16B(0);
				}

				//device_information -> system_id
				if (this->cmd_iter == 4)
				{
					//Characteristic handle
					WRITE_16B(this->btle_characteristic_handles[CHAR_DEVICE_SYSTEM_ID]);
					//Attribute Type Length
					this->StreamWrite(0x02);
					//Attribute Type
					WRITE_16B(0x2A23);
					//Attribute Value Length
					WRITE_16B(8);
					//Attribute Value
					for (uint8_t i = 0; i < 8; i++)
						this->StreamWrite(device_id[i]);
					//Permissions
					WRITE_16B(0);
				}

				//spp_over_ble -> spp_over_ble
				if (this->cmd_iter == 5)
				{
					//Characteristic handle
					WRITE_16B(this->btle_characteristic_handles[CHAR_SPP_SPP]);
					//Attribute Type Length
					this->StreamWrite(0x02);
					//Attribute Type
					WRITE_16B(0xFFE1);
					//Attribute Value Length
					WRITE_16B(PAN1026_BUFFER_SIZE);
					//Attribute Value
					for (uint8_t i = 0; i < PAN1026_BUFFER_SIZE; i++)
						this->StreamWrite(0);
					//Permissions
					WRITE_16B(0);
				}

				//spp_over_ble -> spp_over_ble descriptor
				if (this->cmd_iter == 6)
				{
					//Characteristic handle
					WRITE_16B(this->btle_characteristic_handles[CHAR_SPP_SPP]);
					//Attribute Type Length
					this->StreamWrite(0x02);
					//Attribute Type
					WRITE_16B(0x2902);
					//Attribute Value Length
					WRITE_16B(2);
					//Attribute Value
					WRITE_16B(this->btle_notifications);
					//Permissions
					WRITE_16B(0x0003); //GAT READ | GAT WRITE
				}

			break;

			case(pan_cmd_le_gen_random_address):
				DEBUG_BT("pan_cmd_le_gen_random_address\n");
				RAW(TCU_MNG_LE_GEN_RESOLVABLE_BDADDR_REQ);
			break;

			case(pan_cmd_le_set_random_address):
				DEBUG_BT("pan_cmd_le_set_random_address\n");
				RAW(TCU_MNG_LE_SET_RAND_ADDRESS_REQ);

				for (uint8_t i = 0; i < 6; i++)
					this->StreamWrite(config.connectivity.btle_mac[i]);
			break;

			case(pan_cmd_le_start_advertise):
				DEBUG_BT("pan_cmd_le_start_advertise\n");
				t_len = 3 + 19 + 32 + 32;
				TCU_LEN(t_len);

				//ServiceID
				this->StreamWrite(0xD1); // 1
				//OpCode
				this->StreamWrite(0x08); // 2

				t_len = 15 + 32 + 32;
				WRITE_16B(t_len); //4
				//Adv_Interval_Min
				WRITE_16B(0x0040); //6;
				//Adv_Interval_Max
				WRITE_16B(0x0040); //8;
				//Advertising_Type
				this->StreamWrite(0x00); //9; Connectable undirected advertising
				//Own_Address_Type
				this->StreamWrite(0x01); //10; random address
				//Direct_Address_Type
				this->StreamWrite(0x00); //11; public address
				//Direct_Address
				RAW(tcu_mng_le_start_advertise_direct_address); //17; 6B
				//Adv_Channel_Map
				this->StreamWrite(0x07); //18; channel 37 | 38 | 39
				//Adv_Filter_Policy
				this->StreamWrite(0x00); //19; Allow Scan Request from Any, Allow Connect Request from Any
				//Adv_Data_Length
				this->StreamWrite(0x1f); //31
				//Adv_Data (31B)
					this->StreamWrite(0x02); //length
					this->StreamWrite(0x01); //AD flags
					this->StreamWrite(0x01);

					//128-bits uuids (SPP)
					this->StreamWrite(0x11); //Length 0x10 * cnt + 1
					this->StreamWrite(0x07); //Complete list of 128-bit UUIDs available.
					RAW(spp_over_ble_service_uuid); //SPP uuid 16B

					//16-bits uuids (generic_access, device_information)
					this->StreamWrite(0x05); //Length 0x02 * cnt + 1
					this->StreamWrite(0x03); //Complete list of 16-bit UUIDs available.
					//generic_access
					WRITE_16B(0x1800);
					//device_information
					WRITE_16B(0x180A);

					//padding (4B)
					this->StreamWrite(0x00);
					this->StreamWrite(0x00);
					this->StreamWrite(0x00);
					this->StreamWrite(0x00);

				//Scan_Resp_Data_Len
				this->StreamWrite(0x1f); //31
				//Scan_Resp_Data (31B)
					this->StreamWrite(sizeof(device_name_ble) + 1); //Length sizeof(device_name) + 1
					this->StreamWrite(0x09); //Complete local name
					RAW(device_name_ble);

					//padding
					for (uint8_t i = sizeof(device_name_ble) + 2; i < 0x1f; i++)
						this->StreamWrite(0x00);
			break;

			case(pan_cmd_le_update_char_element):
				DEBUG_BT("pan_cmd_le_update_char_element\n");
				t_len = 3 + 4 + 6;

				TCU_LEN(t_len);

				//ServiceID
				this->StreamWrite(0xD3);
				//OpCode
				this->StreamWrite(0x25);
				//Param length
				WRITE_16B(t_len - 7);

				//Characteristic handle
				WRITE_16B(this->btle_characteristic_element_handles[CHAR_SPP_SPP_DESC]);
				//Attribute Value Length

				WRITE_16B(2);
				//Attribute Value
				WRITE_16B(this->btle_notifications);
			break;

			case(pan_cmd_le_read_val_accept):
				DEBUG_BT("pan_cmd_le_read_val_accept\n");
				t_len = 3 + 4 + 5;
				TCU_LEN(t_len);

				//ServiceID
				this->StreamWrite(0xD3); // 1
				//OpCode
				this->StreamWrite(0x02); // 2

				//Parameter Length
				WRITE_16B(t_len - 7); // 4
				//Connection Handle
				WRITE_16B(this->btle_connection_handle);
				//Status
				this->StreamWrite(0x00); //Success
				//Error Handle
				WRITE_16B(0x0000); //Not valid
			break;

			case(pan_cmd_le_read_multiple_accept):
				DEBUG_BT("pan_cmd_le_read_multiple_accept\n");
				t_len = 3 + 4 + 5;
				TCU_LEN(t_len);

				//ServiceID
				this->StreamWrite(0xD3); // 1
				//OpCode
				this->StreamWrite(0x0A); // 2

				//Parameter Length
				WRITE_16B(t_len - 7); // 4
				//Connection Handle
				WRITE_16B(this->btle_connection_handle);
				//Status
				this->StreamWrite(0x00); //Success
				//Error Handle
				WRITE_16B(0x0000); //Not valid
			break;

			case(pan_cmd_le_write_val_accept):
				DEBUG_BT("pan_cmd_le_write_val_accept\n");
				t_len = 3 + 4 + 5;
				TCU_LEN(t_len);

				//ServiceID
				this->StreamWrite(0xD3); // 1
				//OpCode
				this->StreamWrite(0x03); // 2

				//Parameter Length
				WRITE_16B(t_len - 7); // 4
				//Connection Handle
				WRITE_16B(this->btle_connection_handle);
				//Status
				this->StreamWrite(0x00); //Success
				//Error Handle
				WRITE_16B(0x0000); //Not valid
			break;

			case(pan_cmd_le_read_char_des_accept):
				DEBUG_BT("pan_cmd_le_read_char_des_accept\n");
				t_len = 3 + 4 + 5;
				TCU_LEN(t_len);

				//ServiceID
				this->StreamWrite(0xD3); // 1
				//OpCode
				this->StreamWrite(0x08); // 2

				//Parameter Length
				WRITE_16B(t_len - 7); // 4
				//Connection Handle
				WRITE_16B(this->btle_connection_handle);
				//Status
				this->StreamWrite(0x00); //Success
				//Error Handle
				WRITE_16B(0x0000); //Not valid
			break;

			case(pan_cmd_le_write_char_des_accept):
				DEBUG_BT("pan_cmd_le_write_char_des_accept\n");
				t_len = 3 + 4 + 5;
				TCU_LEN(t_len);

				//ServiceID
				this->StreamWrite(0xD3); // 1
				//OpCode
				this->StreamWrite(0x04); // 2

				//Parameter Length
				WRITE_16B(t_len - 7); // 4
				//Connection Handle
				WRITE_16B(this->btle_connection_handle);
				//Status
				this->StreamWrite(0x00); //Success
				//Error Handle
				WRITE_16B(0x0000); //Not valid
			break;

			case(pan_cmd_le_mtu_req):
				DEBUG_BT("pan_cmd_le_mtu_req\n");
				t_len = 3 + 8;
				TCU_LEN(t_len);

				//ServiceID
				this->StreamWrite(0xD2); // 1
				//OpCode
				this->StreamWrite(0x01); // 2

				//Parameter Length
				WRITE_16B(t_len - 7); // 4
				//Connection Handle
				WRITE_16B(this->btle_connection_handle);
				//MTU size
				WRITE_16B(PAN1026_MTU_RX); //MAX 64
			break;

			case(pan_cmd_le_mtu_accept):
				DEBUG_BT("pan_cmd_le_mtu_accept\n");
				t_len = 3 + 9;
				TCU_LEN(t_len);

				//ServiceID
				this->StreamWrite(0xD3); // 1
				//OpCode
				this->StreamWrite(0x01); // 2

				//Parameter Length
				WRITE_16B(t_len - 7); // 4
				WRITE_16B(this->btle_connection_handle);
				//Status
				this->StreamWrite(0x00); // 6
				//MTU size
				WRITE_16B(PAN1026_MTU_RX); //MAX 64
			break;
		}

		this->last_cmd = this->next_cmd;
		this->next_cmd = pan_cmd_none;
	}

	if (this->repat_last_cmd && this->repeat_timer < task_get_ms_tick())
	{
		this->repat_last_cmd = false;
		this->next_cmd = this->last_cmd;
		this->repeat_timer = BT_NO_TIMEOUT;
	}

	if (this->busy && this->busy_timer < task_get_ms_tick())
	{
		DEBUG_BT("Releasing busy flag\n");

		this->ClearBusy();
	}

	this->SendString();

}

bool pan1026::Idle()
{
	if (this->busy)
	{
		DEBUG_BT("BT BUSY with cmd %d\n", this->last_cmd);
		return false;
	}
	else
		return true;
}

void pan1026::SendString()
{
	uint16_t len = bt_output.Length();
	if (len == 0)
		return;

	if (!this->Idle())
		return;

	if (this->btle_connection)
	{
		uint16_t t_len;
		if (len > this->mtu_size - 4)
			len = this->mtu_size - 4;


		if (this->btle_notifications & BTLE_NOTIFICATION)
		{
			DEBUG_BT("Sending notification len %u\n", len);
			t_len = 3 + 8 + len;
			TCU_LEN(t_len);

			//ServiceID
			this->StreamWrite(0xD3); // 1
			//OpCode
			this->StreamWrite(0x05); // 2

			//Parameter Length
			WRITE_16B(t_len - 7); // 4
			//Connection Handle
			WRITE_16B(this->btle_connection_handle);// 6
			//Characteristic Handle
			WRITE_16B(this->btle_characteristic_element_handles[CHAR_SPP_SPP]);// 8
			//data
			for (uint16_t i = 0 ; i < len; i++)
				this->StreamWrite(bt_output.Read());

			bt_output.Rewind(len);
			this->last_send_len = len;

			this->last_cmd = pan_cmd_le_val_notification;
			this->SetBusy(BT_TIMEOUT_SEND);

			return;
		}

		//no indication or notification are enabled
		bt_output.Clear();
	}
	else
	{
		if (len > PAN1026_SPP_MTU)
			len = PAN1026_SPP_MTU;

		TCU_LEN(len + 3 + 1 + 1 + 2 + 2);
		this->StreamWrite(0xe5); //SPP
		this->StreamWrite(0x08); //TCU_SPP_DATA_TRANSFER_REQ
		WRITE_16B(len + 2); 	 //Parameter Length
		WRITE_16B(len);			 //Length_of_Data
		//data
		for (uint16_t i = 0; i < len; i++)
			this->StreamWrite(bt_output.Read());

		this->last_cmd = pan_cmd_spp_send;
		this->SetBusy(BT_TIMEOUT_SEND);
	}
}
