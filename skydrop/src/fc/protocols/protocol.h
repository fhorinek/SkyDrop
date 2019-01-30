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
void protocol_rx(char c);

#define PROTOCOL_RX_BUFFER	64
#define PROTOCOL_TX_BUFFER	256

extern uint8_t protocol_rx_buffer[PROTOCOL_RX_BUFFER];
extern uint8_t protocol_tx_buffer[PROTOCOL_TX_BUFFER];

extern FILE * protocol_tx;

void protocol_tx_write(uint8_t c);
void protocol_tx_write(uint16_t len, uint8_t * data);
void protocol_tx_flush();

void protocol_init();

#endif /* PROTOCOL_H_ */
