#include "bt.h"

#include "pan1322.h"
#include "pan1026.h"

#define BT_PAN1322	0
#define BT_PAN1026	1

uint8_t bt_module_type = BT_PAN1026;

pan1026 bt_pan1026;
pan1322 bt_pan1322;

bool bt_device_connected = false;


ISR(BT_CTS_PIN_INT)
{
	if (bt_module_type == BT_PAN1322)
		bt_pan1322.TxResume();
	if (bt_module_type == BT_PAN1026)
		bt_pan1026.TxResume();
}

bool bt_pan1322_init()
{
	bt_pan1322.Init();
	bt_pan1322.Restart();

	if (!bt_pan1322.SetBaudrate(921600ul))
		return false;

	if (!bt_pan1322.SetName(PSTR("SkyDrop")))
	{
		if (!bt_pan1322.SetName(PSTR("SkyDrop")))
		{
			DEBUG("SetName FAILED\n");
			return false;
		}
	}

	if (!bt_pan1322.CreateService(PSTR("0000110100001000800000805f9b34fb"), PSTR("SPP"), 01, PSTR("080510")))
	{
		DEBUG("CreateService FAILED\n");
		return false;
	}
	if (!bt_pan1322.SetDiscoverable(true))
	{
		DEBUG("SetDiscoverable FAILED\n");
		return false;
	}


	DEBUG("bt_init SUCESS\n");
	return true;
}

bool bt_pan1026_init()
{
	DEBUG("bt_pan1026_init\n");

	bt_pan1026.Init();

	while(1)
	{
		if (bt_device_connected)
		{
			bt_send("hovno\n");
			_delay_ms(100);
		}

		bt_pan1026.Step();
	}
}

bool bt_init()
{
	DEBUG("bt_init\n");
	BT_UART_PWR_ON;

	if (bt_module_type == BT_PAN1322)
		return bt_pan1322_init();

	if (bt_module_type == BT_PAN1026)
		return bt_pan1026_init();

	return false;
}

bool bt_device_active()
{
	return bt_device_connected;
}


void bt_step()
{
	if (bt_module_type == BT_PAN1322)
		bt_pan1322.Step();
	if (bt_module_type == BT_PAN1026)
		bt_pan1026.Step();
}

void bt_send(char * str)
{
//	if (bt_module_type == BT_PAN1322)
//		bt_pan1322.Step();
	if (bt_module_type == BT_PAN1026)
		bt_pan1026.SendString(str);
}

void bt_irgh(uint8_t type, uint8_t * buf)
{
	switch(type)
	{
		case(BT_IRQ_CONNECTED):
			DEBUG("device connected\n");
			bt_device_connected = true;
		break;
		case(BT_IRQ_DISCONNECTED):
			DEBUG("device disconnected\n");
			bt_device_connected = false;
		break;

	}
}
