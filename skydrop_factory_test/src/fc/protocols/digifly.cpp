#include "digifly.h"

#include "protocol.h"
#include "../fc.h"

void protocol_digifly_step()
{
	char tmp[83];

	sprintf_P(tmp, PSTR("D,%0.2f,%0.3f,,,%0.1f,,,,,,,"), fc.vario.vario * 10, fc.vario.pressure, fc.temp.temp / 10.0);
	fprintf_P(protocol_tx, PSTR("$%s*%02X\r\n"), tmp, nmea_checksum(tmp));

	//10Hz refresh
	protocol_set_next_step(100);

	//send the data
	protocol_tx_flush();
}
