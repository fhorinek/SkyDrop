#include "skybean.h"

#include "../protocol.h"
#include "../../fc.h"

#include "info.h"
#include "file.h"
#include "special.h"

Stream skybean_stream;

//use "shared" rx buffer
RingBuffer skybean_rx_buffer(PROTOCOL_RX_BUFFER, protocol_rx_buffer);

CreateStdOut(skybean_tx, skybean_stream.Write);

void skybean_startpacket(uint16_t len, uint8_t op_hi, uint8_t op_lo)
{
	len += 2;
	skybean_stream.StartPacket(len);
	skybean_stream.Write(op_hi);
	skybean_stream.Write(0x80 | op_lo);
}


void protocol_skybean_packet()
{
	DEBUG("protocol_skybean_packet\n");
	skybean_stream.Debug();

	uint8_t op_hi = skybean_stream.Read();
	uint8_t op_lo = skybean_stream.Read();

	DEBUG("hi %02X\n", op_hi);
	DEBUG("lo %02X\n", op_lo);

	switch (op_hi)
	{
		case(SKYBEAN_INFO): //device info
			skybean_info(op_lo);
		break;

		case(SKYBEAN_FILE): //file operatios
			skybean_file(op_lo);
		break;

		case(SKYBEAN_SPECIAL): //special
			skybean_special(op_lo);
		break;

		default:
			skybean_startpacket(0, op_hi, SB_ERROR);
	}

	protocol_tx_flush();
}

void protocol_skybean_init()
{
	DEBUG("protocol_skybean_init\n");
	skybean_stream.Init(protocol_tx, &skybean_rx_buffer);
	skybean_stream.RegisterOnPacket(protocol_skybean_packet);
}

void protocol_skybean_rx(char c)
{
	skybean_stream.Decode(c);
}

void protocol_skybean_step()
{
	//empty for now
}
