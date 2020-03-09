#include "vario.h"

#include "../uart.h"
#include "../../fc/fc.h"
#include "buzzer.h"
#include "audio.h"

#define MS_TO_TICKS			31
#define AUDIO_LOW_PASS		10.0

#define VARIO_OFF		0
#define VARIO_BEEP		1
#define VARIO_PAUSE		2
#define VARIO_CONT		3
#define VARIO_PRE_BEEP	4

volatile uint8_t audio_vario_mode = VARIO_OFF;
volatile uint16_t audio_vario_pause;
volatile uint16_t audio_vario_length;
volatile float audio_vario_freq = 0;

volatile int16_t audio_vario_prebeep_length = 0;
volatile int16_t audio_vario_prebeep_frequency = 0;


int16_t vario_ivario_old = 0;
bool vario_force_change = false;

extern Timer audio_timer;


//linear aproximation between two points
uint16_t get_near(float vario, volatile uint16_t * src)
{
	vario = vario * 2; //1 point for 50cm
	float findex = floor(vario) +  20;
	float m = vario - floor(vario);

	uint8_t index = findex;

	if (findex > 39)
	{
		index = 39;
		m = 1.0;
	}

	if (findex < 0)
	{
		index = 0;
		m = 0.0;
	}

	int16_t start = src[index];

	start = start + (float)((int16_t)src[index + 1] - start) * m;

	return start;
}

ISR(AUDIO_TIMER_OVF)
{
	if (audio_vario_mode == VARIO_BEEP)
	//pause start
	{
		//silent
		buzzer_set_vol(0);

		if (audio_vario_pause > 0)
		{
			audio_timer.SetTop(audio_vario_pause);
			audio_vario_mode = VARIO_PAUSE;
		}
		else
		{
			audio_off();
		}

		return;
	}

	if (audio_vario_mode == VARIO_PAUSE)
	//sound start
	{
		//vario in continous mode
		if (audio_vario_length == 0)
		{
			audio_off();
			return;
		}

		buzzer_set_vol(config.gui.vario_volume);

		if (audio_vario_prebeep_length > 0)
		{
			buzzer_set_freq(audio_vario_prebeep_frequency);
			audio_timer.SetTop(audio_vario_prebeep_length);
			audio_vario_mode = VARIO_PRE_BEEP;
		}
		else
		{
			buzzer_set_freq(audio_vario_freq);
			audio_timer.SetTop(audio_vario_length);
			audio_vario_mode = VARIO_BEEP;
		}

		return;
	}

	if (audio_vario_mode == VARIO_PRE_BEEP)
	//sound start after prebeep
	{
		//vario in continous mode
		if (audio_vario_length == 0)
		{
			audio_off();
			return;
		}

		buzzer_set_vol(config.gui.vario_volume);
		buzzer_set_freq(audio_vario_freq);
		audio_timer.SetTop(audio_vario_length);
		audio_vario_mode = VARIO_BEEP;

		return;
	}

}

void audio_vario_apply()
{
	switch (audio_vario_mode)
	{
		case(VARIO_OFF):
			//start the beeps
			if (audio_vario_length > 0 && audio_vario_pause > 0)
			{
				buzzer_set_vol(config.gui.vario_volume);
				audio_timer.SetValue(0);

				if (audio_vario_prebeep_length > 0)
				{
					buzzer_set_freq(audio_vario_prebeep_frequency);
					audio_timer.SetTop(audio_vario_prebeep_length);
					audio_vario_mode = VARIO_PRE_BEEP;
				}
				else
				{
					buzzer_set_freq(audio_vario_freq);
					audio_timer.SetTop(audio_vario_length);
					audio_vario_mode = VARIO_BEEP;
				}

				audio_timer.Start();

				break;
			}
			//continous tone
			else
			{
				buzzer_set_vol(config.gui.vario_volume);
				buzzer_set_freq(audio_vario_freq);

				audio_vario_mode = VARIO_CONT;
				break;
			}

		break;

		case(VARIO_BEEP):
			if (audio_vario_length == 0 || audio_vario_pause == 0)
			{
				audio_timer.Stop();
				buzzer_set_vol(config.gui.vario_volume);
				buzzer_set_freq(audio_vario_freq);

				audio_vario_mode = VARIO_CONT;
				break;
			}

			if (config.audio_profile.flags & AUDIO_FLUID)
				buzzer_set_freq(audio_vario_freq);
		break;

		case(VARIO_PAUSE):
			if (vario_force_change)
				audio_vario_mode = VARIO_OFF;
		break;

		case(VARIO_CONT):
			if (audio_vario_length > 0 && audio_vario_pause > 0)
			{
				buzzer_set_freq(audio_vario_freq);

				audio_timer.SetValue(0);
				audio_timer.SetTop(audio_vario_length);
				audio_timer.Start();

				audio_vario_mode = VARIO_BEEP;
				break;
			}

			buzzer_set_freq(audio_vario_freq);
		break;
	}
}


void audio_vario_step(float vario)
{
	if (config.gui.vario_mute || (config.gui.silent & (1 << active_page)))
	{
		audio_off();
		return;
	}

	//climb is float in m/s
	int16_t ivario = vario * 100;

	vario_force_change = (abs(ivario - vario_ivario_old) >= 10) ? true: false;
	vario_ivario_old = ivario;

	if ((ivario >= config.audio_profile.lift ||
		(ivario >= config.audio_profile.lift - config.audio_profile.weak && ((config.audio_profile.flags & AUDIO_WEAK_MASK) != AUDIO_WEAK_OFF)) ||
		 ivario <= config.audio_profile.sink) && (config.gui.vario_volume > 0))
	{
		//get frequency from the table
		uint16_t freq = get_near(vario, config.audio_profile.freq);
		if (audio_vario_freq != 0)
			audio_vario_freq += ((float)freq - audio_vario_freq) / AUDIO_LOW_PASS;
		else
			audio_vario_freq = freq;

		//if normal lift, weak or sink
		audio_vario_length = get_near(vario, config.audio_profile.length) * MS_TO_TICKS;
		audio_vario_pause = get_near(vario, config.audio_profile.pause) * MS_TO_TICKS;

		audio_vario_prebeep_length = 0;
		audio_vario_prebeep_frequency = 0;

		//weak mode
		if (ivario >= config.audio_profile.lift - config.audio_profile.weak && ivario < config.audio_profile.lift && ((config.audio_profile.flags & AUDIO_WEAK_MASK) != AUDIO_WEAK_OFF))
		{
			switch (config.audio_profile.flags & AUDIO_WEAK_MASK)
			{
				//add prebeep
				case (AUDIO_WEAK_BEEP):
					audio_vario_prebeep_length = config.audio_profile.prebeep_length * MS_TO_TICKS;
					audio_vario_prebeep_frequency = audio_vario_freq -  + config.audio_profile.prebeep_offset;
				break;

				case (AUDIO_WEAK_CONT):
					audio_vario_length = 0;
					audio_vario_pause = 0;

					int16_t lift_start_freq = config.audio_profile.weak_high_freq;

					audio_vario_freq = config.audio_profile.weak_low_freq;
					audio_vario_freq += ((lift_start_freq - config.audio_profile.weak_low_freq) * (ivario - (config.audio_profile.lift - config.audio_profile.weak))) / config.audio_profile.weak;
				break;

			}

		}

		//sink mode
		if (ivario <= config.audio_profile.sink)
		{
			switch (config.audio_profile.flags & AUDIO_SINK_MASK)
			{
				case(AUDIO_SINK_BEEP):
					audio_vario_prebeep_length = config.audio_profile.prebeep_length * MS_TO_TICKS;
					audio_vario_prebeep_frequency = audio_vario_freq + config.audio_profile.prebeep_offset;
				break;

				case(AUDIO_SINK_CONT):
					//if sink is continuous
					audio_vario_length = 0;
					audio_vario_pause = 0;
				break;

				case(AUDIO_SINK_OFF):
					audio_off();
					return;
				break;
			}
		}

		//update audio with new settings
		audio_vario_apply();

		return;
	}

	//no threshold was exceeded -> silent
	audio_off();
}

void audio_vario_reset()
{
	//so lowpass will not affect affect new beeps
	audio_vario_freq = 0;
	//next vario sound will go from OFF state
	audio_vario_mode = VARIO_OFF;

}
