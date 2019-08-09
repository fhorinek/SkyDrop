#include "bluefly.h"

#include "protocol.h"
#include "../../drivers/battery.h"

void protocol_bluefly_step()
{
	uint32_t tmp = round(fc.vario.pressure);

	fprintf_P(protocol_tx, PSTR("PRS %lX\n"), tmp);

	//10Hz refresh
	protocol_set_next_step(100);

	//send the data
	protocol_tx_flush();
}
