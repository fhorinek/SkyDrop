/*
 * sequencer.cpp
 *
 *  Created on: 3.9.2015
 *      Author: horinek
 */

#include "sequencer.h"

#include "../uart.h"
#include "../../fc/fc.h"
#include "buzzer.h"
#include "audio.h"

//sequencer
volatile bool seq_enabled = false;

volatile const uint16_t * seq_tone_ptr;
volatile const uint16_t * seq_length_ptr;
volatile uint8_t seq_index;
volatile uint8_t seq_len;
volatile uint16_t seq_duration;
volatile uint8_t seq_volume;

#define AUDIO_SILENT_AFTER_SEQ	250

void seq_start(const sequence_t * seq, uint8_t volume)
{
	audio_off();
	seq_enabled = true;

	seq_len = pgm_read_byte(&seq->length);
	seq_tone_ptr = (const uint16_t*)pgm_read_word(&seq->tone_ptr);
	seq_length_ptr = (const uint16_t*)pgm_read_word(&seq->length_ptr);
	seq_index = 0;
	seq_volume = volume;
}

void seq_next_tone()
{
	uint16_t tone;
	if (seq_index < seq_len)
	{
		//load tone and length from pgm
		tone = pgm_read_word(&seq_tone_ptr[seq_index]);
		seq_duration = pgm_read_word(&seq_length_ptr[seq_index]);
	}
	else
	{
		//this will separate sequence end from vario sound
		tone = 0;
		seq_duration = AUDIO_SILENT_AFTER_SEQ;
	}

	seq_index++;

	buzzer_set_vol(seq_volume);
	buzzer_set_freq(tone);
}

//audio_step @ 100Hz (called from fc meas_timer)
#define AUDIO_STEP_MS	10

void seq_loop()
{
	if (seq_duration > AUDIO_STEP_MS)
	{
		seq_duration -= AUDIO_STEP_MS;
	}
	else
	{
		if (seq_index == seq_len + 1)
		{
			seq_enabled = false;
			audio_off();
		}
		else
			seq_next_tone();
	}
};
