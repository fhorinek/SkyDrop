/*
 * protocol.h
 *
 *  Created on: 9.9.2015
 *      Author: horinek
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include "../../common.h"

void protocol_step();
void protocol_set_next_step(uint32_t diff);
uint8_t protocol_nmea_checksum(char * s);

void protocol_tx(char * buffer);
void protocol_rx(char c);

#endif /* PROTOCOL_H_ */
