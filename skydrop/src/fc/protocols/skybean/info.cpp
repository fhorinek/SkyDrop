#include "info.h"

void skybean_info(uint8_t op)
{
	uint16_t len;
	char tmp[32];

	switch(op)
	{
		case(0x00): //device info string
			len = 1 + 7;

			skybean_startpacket(len, SKYBEAN_INFO, op);
			skybean_stream.Write(7); //len
			fprintf_P(skybean_tx, PSTR("SkyDrop"));
		break;

		case(0x01): //device ID
			GetID_str(tmp);
			len = strlen(tmp);

			skybean_startpacket(len + 1, SKYBEAN_INFO, op);
			skybean_stream.Write(len); //len
			skybean_stream.Write(len, (uint8_t *)tmp);

		break;

		case(0x02): //HW version
			sprintf(tmp, "drop_%04u", (hw_revision == HW_REW_1504) ? 1504 : 1506);
			len = strlen(tmp);

			skybean_startpacket(len + 1, SKYBEAN_INFO, op);
			skybean_stream.Write(len); //len
			skybean_stream.Write(len, (uint8_t *)tmp);
		break;

		case(0x03): //SW version
			sprintf(tmp, "build %04lu", BUILD_NUMBER);
			len = strlen(tmp);

			skybean_startpacket(len + 1, SKYBEAN_INFO, op);
			skybean_stream.Write(len); //len
			skybean_stream.Write(len, (uint8_t *)tmp);
		break;

		default:
			skybean_startpacket(0, SKYBEAN_INFO, SB_ERROR);
	}
}
