#include "protocol.h"

#include "digifly.h"
#include "LK8EX1.h"
#include "bluefly.h"
#include "skybean/skybean.h"

#include "generic.h"

#include "../fc.h"

uint8_t protocol_rx_buffer[PROTOCOL_RX_BUFFER];

uint8_t protocol_tx_buffer[PROTOCOL_TX_BUFFER];
uint16_t protocol_tx_index = 0;

uint32_t protocol_next_step = 0;

CreateStdOut(protocol_tx, protocol_tx_write);


void protocol_tx_write(uint8_t c)
{
	if (protocol_tx_index < (PROTOCOL_TX_BUFFER - 1))
	{
		protocol_tx_buffer[protocol_tx_index] = c;
		protocol_tx_index++;
	}
}

void protocol_tx_write(uint16_t len, uint8_t * data)
{
	if (protocol_tx_index + len > (PROTOCOL_TX_BUFFER - 1))
		len = (PROTOCOL_TX_BUFFER - 1) - protocol_tx_index;

	memcpy(protocol_tx_buffer + protocol_tx_index, data, len);
	protocol_tx_index += len;
}

void protocol_tx_flush()
{
	if (config.connectivity.uart_function > UART_FORWARD_OFF)
		uart_send(protocol_tx_index, protocol_tx_buffer);

	bt_send(protocol_tx_index, protocol_tx_buffer);

	protocol_tx_index = 0;
}

void protocol_init()
{
	protocol_skybean_init();
	protocol_generic_init();
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
	}
}

void protocol_rx(char c)
{
	switch (config.connectivity.protocol)
	{
		case(PROTOCOL_SKYBEAN):
			protocol_skybean_rx(c);
		break;

		default:
			protocol_generic_rx(c);
		break;
	}
}

