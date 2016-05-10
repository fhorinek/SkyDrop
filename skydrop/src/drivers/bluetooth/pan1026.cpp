#include "pan1026.h"
#include "bt.h"

extern pan1026 bt_pan1026;
CreateStdOut(bt_pan1026_out, bt_pan1026.StreamWrite);

/*! The max. ATT size of TC35661 is limited to 64 bytes */
#define BTLE_SPP_BUFFER_SIZE 60

#define BT_TIMEOUT			1000
#define BT_NO_TIMEOUT		0

//#define DEBUG_BT

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
	pan_cmd_create_spp			= 15,

	//btle
	pan_cmd_le_mng_init			= 16,
	pan_cmd_le_gat_ser_init		= 17,
	pan_cmd_le_gat_cli_init		= 18,
	pan_cmd_le_gat_sdb_add_primary_service = 19,
	pan_cmd_le_gat_sdb_add_characteristic = 20,
	pan_cmd_le_gat_sdb_add_characteristic_elements = 21,
	pan_cmd_le_start_advertise	= 22,
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
	DEBUG("PAN1026 HARD ERROR, Restart\n"); \
	this->Restart(); } while(0); \

void pan1026::TxResume()
{
	this->usart->TxComplete();
}

void pan1026::StreamWrite(uint8_t data)
{
	DEBUG("%02X ", data);
	this->usart->Write(data);
}

void pan1026::Init(Usart * uart)
{
	this->usart = uart;
	this->repat_last_cmd = false;

	bt_irgh(BT_IRQ_INIT, 0);

	strcpy_P(this->label, PSTR("SkyDrop Pan1026"));

	this->Restart();
}

void pan1026::Restart()
{
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

		case(0x10): //Hardware error
#ifdef DEBUG_BT
			DEBUG("Hardware Error event\n");
#endif
			switch (this->parser_buffer[3])
			{
				case(0x20):
					DEBUG("Short of receiving packet\n");
				break;
				case(0x21):
					DEBUG("Stop bit error\n");
				break;
				case(0x22):
					DEBUG("Over write error\n");
				break;
			}

			this->repat_last_cmd = true;
			this->timer = task_get_ms_tick() + 10; //wait 10ms or more

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
//						this->SetNextStep(pan_cmd_le_mng_init);
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
					config.connectivity.bt_link_key[i] = this->parser_buffer[15 + i];
				}
				eeprom_busy_wait();
				eeprom_update_block((void *)&config.connectivity.bt_link_key, &config_ee.connectivity.bt_link_key, 16);

				DEBUG("\n");
				DEBUG(" Link key type   %02X\n", this->parser_buffer[33]);
				DEBUG(" Sniff interval  %02X\n", this->parser_buffer[34] | (this->parser_buffer[35] << 8));
			}

			if (status == 0x87)
			{
				DEBUG(" Link key failure\n");
				for (uint8_t i = 0; i < 16; i++)
					config.connectivity.bt_link_key[i] = 0;

				eeprom_busy_wait();
				eeprom_update_block((void *)&config.connectivity.bt_link_key, &config_ee.connectivity.bt_link_key, 16);
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

#ifdef DEBUG_BT
			DEBUG("HCI status %02X\n", hci_status);
			DEBUG("HCI op_code %02X\n", hci_op_code);
#endif

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

void pan1026::ParseMNG_LE()
{
	uint8_t status, op_code;

#ifdef DEBUG_BT
	DEBUG("\n - MNG LE ----\n");
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
		case(0x4C): // TCU_MNG_LE_CONNECTION_COMPLETE_EVENT
			DEBUG("LE Connection complete event\n");
			DEBUG(" role %02X\n", this->parser_buffer[10]);
			if (status == 0)
			{
				if (this->parser_buffer[10] != 0x01)
				{
					DEBUG("Connection type is not slave, ignoring\n");
					break;
				}

				this->btle_connection_handles[0] = this->parser_buffer[8] | (this->parser_buffer[9] << 8);
				DEBUG("Handle 0x%04X\n", this->btle_connection_handles[0]);
			}
			else
				PAN1026_ERROR;
		break;

		case(0x81): // TCU_MNG_LE_INIT_RESPONSE
			if (status == 0)
			{
				DEBUG("MNG LE init success\n");
				this->SetNextStep(pan_cmd_le_gat_ser_init);
			}
			else
				PAN1026_ERROR;
		break;

		case(0x88): // TCU_MNG_LE_START_ADVERTISE_RESP
			DEBUG("Start advertise: ");
			switch (status)
			{
				case(0x00):
					DEBUG("success\n");
					//this->SetNextStep(pan_cmd_le_gat_ser_init);
				break;
				case(0x86):
					DEBUG("Parameter error\n");
				break;
				case(0x81):
					DEBUG("Dev Not Initialized\n");
				break;
				case(0x82):
					DEBUG("Command in Progress\n");
				break;
				case(0xA4):
					DEBUG("Device Already Advertising\n");
				break;
			}
		break;

		case(0xFF): //TCU_LE_SYS_INVALID_COMMAND
			DEBUG("MNG LE command from Host CPU is invalid\n");
			DEBUG(" ServiceID_Received %02X\n", this->parser_buffer[7]);
			DEBUG(" Command_OpCode_Received %02X\n", this->parser_buffer[8]);
		break;
	}
}

void pan1026::ParseGAT_ser()
{
	uint8_t status, op_code;

#ifdef DEBUG_BT
	DEBUG("\n - GAT ser ----\n");
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
		case(0x80): // TCU_LE_GATT_SER_INIT_RESP
			if (status == 0)
			{
				DEBUG("GAT ser init success\n");
				this->SetNextStep(pan_cmd_le_gat_cli_init);
			}
			else
				PAN1026_ERROR;
		break;

		case(0xA0): // TCU_LE_GATT_SDB_ADD_PRIM_SVC_RESP
			if (status == 0)
			{
				DEBUG("GAT add primary service success\n");
				this->btle_service_handles[this->cmd_iter] = this->parser_buffer[8] | (this->parser_buffer[9] << 8);
				DEBUG("Handle 0x%04X\n", this->btle_service_handles[this->cmd_iter]);
				this->cmd_iter++;
				if (this->cmd_iter < 3)
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
				DEBUG("GAT add characteristic success\n");
				this->btle_characteristic_handles[this->cmd_iter] = this->parser_buffer[8] | (this->parser_buffer[9] << 8);
				DEBUG("Handle 0x%04X\n", this->btle_characteristic_handles[this->cmd_iter]);

				this->cmd_iter++;
				if (this->cmd_iter < 6)
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
				DEBUG("GAT add characteristic element success\n");
				this->btle_element_handles[this->cmd_iter] = this->parser_buffer[8] | (this->parser_buffer[9] << 8);
				DEBUG("Handle 0x%04X\n", this->btle_element_handles[this->cmd_iter]);

				this->cmd_iter++;
				if (this->cmd_iter < 7)
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

	}
}

void pan1026::ParseGAT_cli()
{
	uint8_t status, op_code;

#ifdef DEBUG_BT
	DEBUG("\n - GAT cli ----\n");
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
		case(0x80): // TCU_LE_GATT_SER_INIT_RESP
			if (status == 0)
			{
				DEBUG("GAT cli init success\n");
				this->cmd_iter = 0;
				this->SetNextStep(pan_cmd_le_gat_sdb_add_primary_service);
			}
			else
				PAN1026_ERROR;
		break;
	}
}

void pan1026::Parse(uint8_t c)
{
//	DEBUG("1026<%02X %c\n", c, c);

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
const uint8_t PROGMEM device_name[] = {'S', 'k', 'y', 'D', 'r', 'o', 'p', ' ', 'B', 'T', 'L', 'E'};
const uint8_t PROGMEM manufacturer_name[] = {'S', 'k', 'y', 'B', 'e', 'a', 'n'};


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

void pan1026::WaitForAnswer()
{
	this->timer = task_get_ms_tick() + BT_TIMEOUT;
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
		DEBUG("\n\n< CMD %d\n", this->next_cmd);

		switch(this->next_cmd)
		{
			//init
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

			//classic
			case(pan_cmd_mng_init):
				t_len = 3 + sizeof(TCU_MNG_INIT_REQ) + 5 + strlen(this->label);
				TCU_LEN(t_len);

				RAW(TCU_MNG_INIT_REQ);

				t_len = 3 + strlen(this->label);
				WRITE_16B(t_len);
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
				use_link_key = 0;
				for (uint8_t i = 0; i < 16; i++)
					if (config.connectivity.bt_link_key[i] != 0)
					{
						use_link_key = 1;
						break;
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
					DEBUG("Using link key: ");
					this->StreamWrite(0x01); //use link key
					for (uint8_t i = 0; i < 16; i++)
					{
						this->StreamWrite(config.connectivity.bt_link_key[i]);
						DEBUG("%02X ", config.connectivity.bt_link_key[i]);
					}
					DEBUG("\n");
				}
				else
				{
					this->StreamWrite(0x00); //do not use link key
					DEBUG("Not using link key\n");
				}
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

			//btle
			case(pan_cmd_le_mng_init):
				t_len = 3 + sizeof(TCU_MNG_LE_INIT_REQ) + 3 + strlen(this->label);
				TCU_LEN(t_len);

				RAW(TCU_MNG_LE_INIT_REQ);

				t_len = 1 + strlen(this->label);
				WRITE_16B(t_len);
				this->StreamWrite(strlen(this->label)); //label name len
				fprintf_P(bt_pan1026_out, PSTR("%s"), this->label); //label name
			break;

			case(pan_cmd_le_gat_ser_init):
				RAW(TCU_LE_GATT_SER_INIT_REQ);
			break;

			case(pan_cmd_le_gat_cli_init):
				RAW(TCU_LE_GATT_CLI_INIT_REQ);
			break;

			case(pan_cmd_le_gat_sdb_add_primary_service):
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
				t_len =  3 + 4 + 4 + ((this->cmd_iter != 5) ? 2 : 16);
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
					this->StreamWrite(0x28); //INDICATE | WRITE
					//UUID Length
					this->StreamWrite(0x10);
					//UUID Value
					RAW(spp_over_ble_characteristic_uuid);
				}
			break;

			case(pan_cmd_le_gat_sdb_add_characteristic_elements):
				t_len = 3 + 4 + 7;
				if (this->cmd_iter == 0)
					t_len += 2 + sizeof(device_name);
				if (this->cmd_iter == 1)
					t_len += 2 + 2;
				if (this->cmd_iter == 2)
					t_len += 2 + sizeof(manufacturer_name);
				if (this->cmd_iter == 3)
					t_len += 2 + 9;
				if (this->cmd_iter == 4)
					t_len += 2 + 22;
				if (this->cmd_iter == 5)
					t_len += 16 + BTLE_SPP_BUFFER_SIZE;
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
					WRITE_16B(sizeof(device_name));
					//Attribute Value
					RAW(device_name);
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
					WRITE_16B(22);
					//Attribute Value
					char tmp[23];
					GetID_str(tmp);
					for (uint8_t i = 0; i < 22; i++)
						this->StreamWrite(tmp[i]);
					//Permissions
					WRITE_16B(0);
				}

				//spp_over_ble -> spp_over_ble
				if (this->cmd_iter == 5)
				{
					//Characteristic handle
					WRITE_16B(this->btle_characteristic_handles[CHAR_SPP_SPP]);
					//Attribute Type Length
					this->StreamWrite(0x10);
					//Attribute Type
					RAW(spp_over_ble_characteristic_uuid);
					//Attribute Value Length
					WRITE_16B(BTLE_SPP_BUFFER_SIZE);
					//Attribute Value
					for (uint8_t i = 0; i < BTLE_SPP_BUFFER_SIZE; i++)
						this->StreamWrite(0);
					//Permissions
					WRITE_16B(0);
				}

				//spp_over_ble -> spp_over_ble2
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
					WRITE_16B(0);
					//Permissions
					WRITE_16B(0x0003); //GAT READ | GAT WRITE
				}

			break;

			case(pan_cmd_le_start_advertise):
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
				this->StreamWrite(0x00); //10; public address
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
					this->StreamWrite(sizeof(device_name) + 1); //Length sizeof(device_name) + 1
					this->StreamWrite(0x09); //Complete local name
					RAW(device_name); //sizeof(device_name)

					//padding
					for (uint8_t i = sizeof(device_name) + 2; i < 0x1f; i++)
						this->StreamWrite(0x00);
			break;

		}

		this->last_cmd = this->next_cmd;
		this->next_cmd = pan_cmd_none;
	}

	if (this->repat_last_cmd && this->timer < task_get_ms_tick())
	{
		this->repat_last_cmd = false;
		this->next_cmd = this->last_cmd;
	}

//	if (this->timer != BT_NO_TIMEOUT && this->timer < task_get_ms_tick())
//		{
//			DEBUG("PAN1026 timeout, last cmd %d\n", this->last_cmd);
//			this->timer = BT_NO_TIMEOUT;
//		}

}

void pan1026::SendString(char * str)
{
	uint16_t len = strlen(str);

	TCU_LEN(len + 3 + 1 + 1 + 2 + 2);
	this->StreamWrite(0xe5); //SPP
	this->StreamWrite(0x08); //TCU_SPP_DATA_TRANSFER_REQ
	WRITE_16B(len + 2); 	 //Parameter Length
	WRITE_16B(len);			 //Length_of_Data
	//data
	for (uint16_t i = 0; i < len; i++)
		this->StreamWrite(str[i]);
}
