#include "skybean.h"

#include "protocol.h"
#include "../fc.h"

void protocol_skybean_rx(char c)
{
	DEBUG("DATA RX: %02X\n");
}

void protocol_skybean_step()
{
	char buffer[32];

	strcpy_P(buffer, PSTR(""));

	//10Hz refresh
	protocol_set_next_step(100);

	//send the data
	protocol_tx(buffer);
}
