#include "protocol.h"

#include "digifly.h"
#include "../fc.h"

uint32_t protocol_next_step = 0;

void protocol_set_next_step(uint32_t diff)
{
	protocol_next_step = task_get_ms_tick() + diff;
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
	}

	//XXX:outputs
	bt_send(buffer);
}
