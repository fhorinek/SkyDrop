#include "bluefly.h"

#include "protocol.h"
#include "../../drivers/battery.h"

void protocol_bluefly_step()
{
	char buffer[32];
	uint32_t tmp = round(fc.pressure);

	sprintf_P(buffer, PSTR("PRS %lX\n"), tmp);

	//10Hz refresh
	protocol_set_next_step(100);

	//send the data
	protocol_tx(buffer);
}
