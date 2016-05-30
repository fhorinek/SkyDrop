/*
 * LK8EX1.cpp
 *
 *  Created on: 30.9.2015
 *      Author: horinek
 */

#include "LK8EX1.h"

#include "protocol.h"
#include "../../drivers/battery.h"

void protocol_lk8ex1_step(char * buffer)
{
	char buff[256];

	uint16_t bat;

	if (battery_per == BATTERY_CHARGING)
		bat = 999;
	else if (battery_per == BATTERY_FULL)
		bat = 1100;
	else
		bat = 1000 + (uint16_t)battery_per;

	sprintf_P(buff, PSTR("$LK8EX1,%0.0f,99999,%0.0f,%d,%u,"), fc.pressure, (fc.vario * 100.0), fc.temperature / 10, bat);
	sprintf_P(buffer, PSTR("%s*%02X\n"), buff, protocol_nmea_checksum(buff));

	//10Hz refresh
	protocol_set_next_step(100);
}
