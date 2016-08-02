#include "protocol.h"

#include "digifly.h"
#include "LK8EX1.h"
#include "bluefly.h"
#include "skybean.h"

#include "flynet.h"

#include "../fc.h"

uint32_t protocol_next_step = 0;

uint8_t protocol_nmea_checksum(char *s)
{
	uint8_t c = 0;

    while(*s)
        c ^= *s++;

    return c;
}

void protocol_set_next_step(uint32_t diff)
{
	protocol_next_step = (uint32_t)task_get_ms_tick() + (uint32_t)diff;
}

void protocol_step()
{
	if (protocol_next_step > task_get_ms_tick())
		return;

	switch (config.connectivity.protocol)
	{
		case(PROTOCOL_DIGIFLY):
			protocol_digifly_step();
		break;

		case(PROTOCOL_LK8EX1):
			protocol_lk8ex1_step();
		break;

		case(PROTOCOL_BLUEFLY):
			protocol_bluefly_step();
		break;

		case(PROTOCOL_SKYBEAN):
			protocol_skybean_step();
		break;

		case(PROTOCOL_FLYNET):
			protocol_flynet_step();
		break;
	}
}

void protocol_rx(char c)
{
	switch (config.connectivity.protocol)
	{
		case(PROTOCOL_SKYBEAN):
			protocol_skybean_rx(c);
		break;
	}
}

void protocol_tx(char * buffer)
{
	if (config.connectivity.uart_function > UART_FORWARD_OFF)
		uart_send(buffer);

	bt_send(buffer);
}
