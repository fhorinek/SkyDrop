#include "digifly.h"

#include "protocol.h"
#include "../fc.h"



void protocol_digifly_step(char * buffer)
{
	char buff[256];

	sprintf_P(buff, PSTR("$D,%0.2f,%0.3f,,,%0.1f,,,,,,,"), fc.vario * 10, fc.pressure, fc.temperature);
	sprintf_P(buffer, PSTR("%s*%02X\n"), buff, protocol_gps_checksum(buff));

	//5Hz refresh
	protocol_set_next_step(200);
}
