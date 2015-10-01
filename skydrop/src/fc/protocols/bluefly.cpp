#include "bluefly.h"

#include "protocol.h"
#include "../../drivers/battery.h"

void protocol_bluefly_step(char * buffer)
{
	uint32_t tmp = round(fc.pressure);

	sprintf_P(buffer, PSTR("PRS %lX\n"), tmp);

	//5Hz refresh
	protocol_set_next_step(200);
}
