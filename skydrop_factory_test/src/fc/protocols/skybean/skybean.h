/*
 * skybean.h
 *
 *  Created on: 9.9.2015
 *      Author: horinek
 */

#ifndef SKYBEAN_H_
#define SKYBEAN_H_

#include "../../../common.h"
#include "../protocol.h"

extern FILE * skybean_tx;
extern Stream skybean_stream;

#define SB_FAIL				0x00
#define SB_SUCCESS			0x01
#define SB_ERROR			0xFF

#define SKYBEAN_INFO		0x00
#define SKYBEAN_TELEMETRY	0x01
#define SKYBEAN_CONFIG		0x02
#define SKYBEAN_FILE		0x03
#define SKYBEAN_SPECIAL		0xFF

void protocol_skybean_init();
void protocol_skybean_step();
void protocol_skybean_rx(char c);

void skybean_startpacket(uint16_t len, uint8_t op_hi, uint8_t op_lo);

#endif /* DIGIFLY_H_ */
