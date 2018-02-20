/*
 * sequencer.h
 *
 *  Created on: 3.9.2015
 *      Author: horinek
 */

#ifndef SEQUENCER_H_
#define SEQUENCER_H_

#include "../../common.h"

struct sequence_t
{
	const uint16_t * tone_ptr;
	const uint16_t * length_ptr;
	uint8_t length;
};

#define ARR(...) __VA_ARGS__

#define MK_SEQ(name, tone, length) \
	const uint16_t name ## _tone[] PROGMEM = tone; \
	const uint16_t name ## _length[] PROGMEM = length; \
	const sequence_t name PROGMEM = {name ## _tone, name ## _length, sizeof(name ## _tone) / 2}; \

void seq_start(const sequence_t * seq, uint8_t volume, bool loop = false);
void seq_loop();
void seq_stop();
const sequence_t * seq_active();

extern volatile bool seq_enabled;

#endif /* SEQUENCER_H_ */
