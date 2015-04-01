#include "bt.h"

bool bt_init()
{
	DEBUG("bt_init\n");
	BT_UART_PWR_ON;

	bt_link.Init();
	bt_link.Restart();

	if (!bt_link.SetBaudrate(921600ul))
		return false;

	if (!bt_link.SetName(PSTR("SkyDrop")))
	{
		if (!bt_link.SetName(PSTR("SkyDrop")))
		{
			DEBUG("SetName FAILED\n");
			return false;
		}
	}

	if (!bt_link.CreateService(PSTR("0000110100001000800000805f9b34fb"), PSTR("SPP"), 01, PSTR("080510")))
	{
		DEBUG("CreateService FAILED\n");
		return false;
	}
	if (!bt_link.SetDiscoverable(true))
	{
		DEBUG("SetDiscoverable FAILED\n");
		return false;
	}


	DEBUG("bt_init SUCESS\n");
	return true;
}

bool bt_device_active()
{
//	return (blue.isConnected() && blue.isStreamActive());
}


void bt_step()
{
	bt_link.Step();
}
