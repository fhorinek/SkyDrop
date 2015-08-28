/*
 * audio.h
 *
 *  Created on: 12.5.2015
 *      Author: horinek
 */

#ifndef AUDIO_H_
#define AUDIO_H_

#include "../../common.h"

extern volatile bool audio_demo;
extern volatile float audio_demo_val;

struct beep_t
{
	const uint16_t * tone_ptr;
	const uint16_t * length_ptr;
	uint8_t length;
};

#define ARR(...) __VA_ARGS__

#define MK_BEEP(name, tone, length) \
	const uint16_t name ## _tone[] PROGMEM = tone; \
	const uint16_t name ## _length[] PROGMEM = length; \
	const beep_t name PROGMEM = {name ## _tone, name ## _length, sizeof(name ## _tone) / 2}; \



void audio_init();
void audio_step();
void audio_off();
void audio_beep_loop();
void audio_beep_start(const beep_t * beep);

#endif /* AUDIO_H_ */
