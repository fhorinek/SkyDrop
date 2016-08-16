#include "info.h"

void skybean_info(uint8_t op)
{
	uint16_t len;

	switch(op)
	{
		case(0x00): //device info string
			len = 1 + 7;

			skybean_startpacket(len, SKYBEAN_INFO, op);
			skybean_stream.Write(7); //len
			fprintf_P(skybean_tx, PSTR("SkyDrop"));
		break;

		case(0x01): //device ID
			len = 1 + 11;

			skybean_startpacket(len, SKYBEAN_INFO, op);
			skybean_stream.Write(11); //len
			skybean_stream.Write(11, device_id);

		break;

		default:
			skybean_startpacket(0, SKYBEAN_INFO, SB_ERROR);
	}
}
