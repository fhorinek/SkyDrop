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

#endif /* PROTOCOL_H_ */
