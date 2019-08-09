#include "audio.h"
#include "../../fc/fc.h"

#include "buzzer.h"
#include "sequencer.h"

Timer audio_timer;


//demo
volatile bool audio_demo = false;
volatile float audio_demo_val = 0;

void audio_init()
{
	AUDIO_TIMER_PWR_ON;
	audio_timer.Init(AUDIO_TIMER, timer_div1024);
	audio_timer.EnableInterrupts(timer_overflow);
}

void audio_step()
{
	//sound effect is high priority
	if (seq_enabled)
	{
		seq_loop();
		return;
	}


}

void audio_off()
{
	//stop unused timer
	audio_timer.Stop();
	//silence!
	buzzer_set_vol(0);
}


