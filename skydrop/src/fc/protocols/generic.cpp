#include "generic.h"

#include "protocol.h"
#include "../conf.h"

#define	GENERIC_IDLE		0
#define	GENERIC_DATA		1
#define	GENERIC_CHECKSUM_A	2
#define	GENERIC_CHECKSUM_B	3

uint8_t protocol_generic_state = GENERIC_IDLE;
uint8_t protocol_generic_len;
uint8_t protocol_generic_checksum;

void protocol_generic_init()
{
	protocol_generic_state = GENERIC_IDLE;
}

void protocol_generic_parse()
{
	char * buffer_pos = (char *)protocol_rx_buffer;

	DEBUG("Generic RX data '%s'\n", protocol_rx_buffer);

	if (!cmpn(buffer_pos, "SBC", 3))
		return;

	buffer_pos = find_comma(buffer_pos);

	if (cmpn(buffer_pos, "VOL", 3))
	{
		buffer_pos = find_comma(buffer_pos);

		uint8_t val = atoi_c(buffer_pos);
		if (val <= 100)
		{
			config.gui.vario_volume = val;
		}
	}
}

void protocol_generic_rx(uint8_t c)
{
	uint8_t tmp_chsum;

	switch (protocol_generic_state)
	{
		case(GENERIC_IDLE):
			if (c == '$')
			{
				protocol_generic_len = 0;
				protocol_generic_state = GENERIC_DATA;
			}
		break;

		case(GENERIC_DATA):
			if (c == '*')
			{
				protocol_rx_buffer[protocol_generic_len] = 0;
				protocol_generic_state = GENERIC_CHECKSUM_A;
			}
			else
			{
				protocol_rx_buffer[protocol_generic_len] = c;
				protocol_generic_len++;
			}
		break;

		case(GENERIC_CHECKSUM_A):
			protocol_generic_checksum = hex_to_num(c) << 4;
			protocol_generic_state = GENERIC_CHECKSUM_B;
		break;

		case(GENERIC_CHECKSUM_B):
			protocol_generic_checksum |= hex_to_num(c);
			protocol_generic_state = GENERIC_IDLE;

//			DEBUG(">>'%s'\n", protocol_rx_buffer);

			tmp_chsum = nmea_checksum((char *)protocol_rx_buffer);
//			DEBUG(">>> %02X, %02X\n", protocol_generic_checksum, tmp_chsum);


			if (protocol_generic_checksum == tmp_chsum)
				protocol_generic_parse();
		break;
	}
}
