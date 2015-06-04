#include "audio.h"
#include "../drivers/buzzer.h"
#include "fc.h"

Timer audio_timer;

volatile uint16_t next_tone = 0;
volatile uint16_t next_length = 0;
volatile uint16_t next_pause = 0;

volatile bool delay_on = false;

#define PERIOD_SOUND		0
#define PERIOD_PAUSE		1
volatile uint8_t audio_period = PERIOD_SOUND;

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

	m = round(m * 10) / 10.0;

	int16_t start = src[index];

	start = start + (float)((int16_t)src[index + 1] - start) * m;

	return start;
}

ISR(AUDIO_TIMER_OVF)
{
	if (audio_period == PERIOD_SOUND)
	//pause start
	{
		buzzer_set_vol(0);

		if (next_pause == 0)
		{
			audio_timer.Stop();
			delay_on = false;

			return;
		}

		audio_timer.SetTop(next_pause);

		audio_period = PERIOD_PAUSE;
	}
	else
	//sound start
	{
		if (next_tone > 0)
		{
			buzzer_set_freq(next_tone);
			buzzer_set_vol(fc.audio_volume);
		}

		if (next_length == 0)
		{
			audio_timer.Stop();
			delay_on = false;

			return;
		}

		audio_timer.SetTop(next_length);

		audio_period = PERIOD_SOUND;
	}
}

void audio_set_tone(uint16_t tone)
{
	if (tone == 0)
	{
		next_tone = 0;

		//buzzer is running continuously turn off sound
		if (!delay_on)
		{
			//disable sound output
			buzzer_set_vol(0);
		}
	}
	else
	{
		next_tone = tone;

		//buzzer is running continuously update freq now
		if (delay_on == false)
		{
			buzzer_set_freq(next_tone);
			buzzer_set_vol(fc.audio_volume);
		}

		//fluid update is enabled
		if (fc.audio_fluid && audio_period == PERIOD_SOUND)
			buzzer_set_freq(next_tone);
	}
}

void audio_set_delay(uint16_t length, uint16_t pause)
{
	//Continuous sound (sink)
	if (pause == 0 || length == 0)
	{
		next_length = 0;
		next_pause = 0;

	}
	else
	//with pauses (lift)
	{
		//convert from Hz and ms
		next_length = 31 * length;
		next_pause = 31 * pause;

		//if previous sound was continuous (audio_timer is not working)
		if (delay_on == false)
		{
			//restart timer counter
			audio_timer.SetValue(1);

			//set new tone + enable sound
			buzzer_set_freq(next_tone);
			buzzer_set_vol(fc.audio_volume);

			audio_timer.SetTop(next_length);

			//start timer
			audio_timer.Start();

			//set the period state state
			audio_period = PERIOD_SOUND;
		}

		//we have pauses enabled
		delay_on = true;
	}
}

void audio_init()
{
	AUDIO_TIMER_PWR_ON;
	audio_timer.Init(AUDIO_TIMER, timer_div1024);
	audio_timer.EnableInterrupts(timer_overflow);
}

void audio_step(float vario)
{
	uint16_t freq;
	uint16_t length;
	uint16_t pause;

	//GET fresh values from table
	// - climb is float in m/s
	if ((vario * 100 >= fc.audio_lift || vario * 100 <= fc.audio_sink) && (fc.audio_volume > 0))
	{
		if (vario * 100 >= fc.audio_lift)
		{
			//get frequency from the table
			freq = get_near(vario, fc.buzzer_freq);
			length = get_near(vario, fc.buzzer_length);
			pause = get_near(vario, fc.buzzer_pause);
		}
		else //XXX: this is hack we need to fix this, so beeps can be used in sink too
		{
			//get frequency from the table
			freq = get_near(vario, fc.buzzer_freq);
			length = 0;
			pause = 0;
		}

	}
	else
	//no threshold was exceeded -> silent
	{
		freq = 0;
		length = 0;
		pause = 0;
	}

	//update audio with new settings
	audio_set_tone(freq);
	audio_set_delay(length, pause);
}
