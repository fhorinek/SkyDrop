#include "splash.h"

#include "../fc/audio.h"

uint32_t splash_timer = 0;
uint8_t splash_cnt = 0;
uint8_t splash_mode;
#define SPLASH_TIMEOUT	500
#define SPLASH_ANIM_TOP	35

MK_BEEP(beep_on, ARR({261, 329, 392, 523}), ARR({250, 250, 250, 250}));
MK_BEEP(beep_off, ARR({523, 392, 261}), ARR({250, 250, 250}));

void gui_splash_set_mode(uint8_t mode)
{
	splash_mode = mode;
}

void gui_splash_init()
{
	if (splash_mode == SPLASH_ON)
	{
		gui_trigger_backlight();
		splash_cnt = 0;
//		audio_beep_start(&beep_on);
	}

	if (splash_mode == SPLASH_OFF)
	{
		lcd_bckl(0);
		splash_cnt = SPLASH_ANIM_TOP;
//		audio_beep_start(&beep_off);
	}
}

void gui_splash_stop() {}

void gui_splash_loop()
{
	//border
	disp.DrawLine(24, 16, 33,  7, 1);
	disp.DrawLine(34,  7, 49,  7, 1);
	disp.DrawLine(50,  7, 59, 16, 1);
	disp.DrawLine(59, 17, 51, 25, 1);
	disp.DrawLine(50, 25, 42, 17, 1);
	disp.DrawLine(41, 17, 33, 25, 1);
	disp.DrawLine(32, 25, 24, 17, 1);

	disp.DrawLine(42, 26, 50, 34, 1);
	disp.DrawLine(50, 35, 42, 43, 1);
	disp.DrawLine(41, 43, 33, 35, 1);
	disp.DrawLine(33, 34, 41, 26, 1);

	for (uint8_t i = 0; i < splash_cnt && i < 8; i++)
		disp.DrawLine(41 - i, 42 - i, 42 + i, 42 - i, 1);

	for (uint8_t i = 0; i < splash_cnt - 8 && i < 8; i++)
		disp.DrawLine(34 + i, 34 - i, 49 - i, 34 - i, 1);


	for (uint8_t i = 0; i + 8 < splash_cnt && i < 8; i++)
	{
		disp.DrawLine(25 + i, i + 17, 40 - i, i + 17, 1);
		disp.DrawLine(25 + 18 + i, i + 17, 40 + 18 - i, i + 17, 1);
	}

	for (uint8_t i = 0; i < splash_cnt && i < 9; i++)
		disp.DrawLine(33 - i, i + 8, 50 + i, i + 8, 1);

	disp.LoadFont(F_TEXT_S);
	uint8_t t_h = disp.GetTextHeight();
	disp.GotoXY(0, GUI_DISP_HEIGHT - t_h);
	fprintf_P(lcd_out, PSTR("build %04d"), BUILD_NUMBER);


	if (splash_mode == SPLASH_ON)
	{
		if (splash_cnt < SPLASH_ANIM_TOP)
		{
			splash_cnt++;
			splash_timer = task_get_ms_tick() + SPLASH_TIMEOUT;
		}
		else
		{
			if (splash_timer < task_get_ms_tick())
				gui_switch_task(GUI_PAGES);
		}
		gui_trigger_backlight();
	}

	if (splash_mode == SPLASH_OFF)
	{
		if (splash_cnt > 0)
		{
			splash_cnt--;
			splash_timer = task_get_ms_tick() + SPLASH_TIMEOUT;
		}
		else
		{
			if (splash_timer < task_get_ms_tick())
				task_set(TASK_POWERDOWN);
		}
	}
}

void gui_splash_irqh(uint8_t type, uint8_t * buff)
{

}


