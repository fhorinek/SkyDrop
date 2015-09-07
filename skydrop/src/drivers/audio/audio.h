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

void audio_init();
void audio_step();
void audio_off();

#endif /* AUDIO_H_ */
