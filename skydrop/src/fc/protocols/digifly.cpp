#include "digifly.h"

#include "protocol.h"
#include "../fc.h"

void protocol_digifly_step()
{
	char tmp[83];
	char buffer[83];

	sprintf_P(tmp, PSTR("D,%0.2f,%0.3f,,,%0.1f,,,,,,,"), fc.vario * 10, fc.pressure, fc.temperature / 10.0);
	sprintf_P(buffer, PSTR("$%s*%02X\n"), tmp, protocol_nmea_checksum(tmp));

	//10Hz refresh
	protocol_set_next_step(100);

	//send the data
	protocol_tx(buffer);
}
