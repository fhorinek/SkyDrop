#include "protocol.h"

#include "digifly.h"
#include "LK8EX1.h"
#include "bluefly.h"

#include "../fc.h"

uint32_t protocol_next_step = 0;

uint8_t protocol_gps_checksum(char *s)
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

	char buffer[512];

	switch (config.system.protocol)
	{
		case(PROTOCOL_DIGIFLY):
			protocol_digifly_step(buffer);
		break;

		case(PROTOCOL_LK8EX1):
			protocol_lk8ex1_step(buffer);
		break;

		case(PROTOCOL_BLUEFLY):
			protocol_bluefly_step(buffer);
		break;
	}

	//XXX:outputs
	bt_send(buffer);
//	DEBUG("%s", buffer);
}
