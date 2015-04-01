#include "led.h"

Timer led_timer1;
Timer led_timer2;


volatile uint8_t led_anim_mode = LED_NO_ANIM;
volatile uint32_t led_anim_progress = 0;
volatile uint32_t led_anim_top = 0;

volatile uint8_t lcd_bckl_actual = 0;
volatile uint8_t lcd_bckl_wanted = 0;

#define led_timer_top 0xFF

ISR(LED_TIMER1_OVF)
{
	uint16_t val = 0, r = 0, g = 0, b = 0;

	if (led_anim_progress < led_anim_top / 10) //10 %
	{
		val = ((1000 * led_anim_progress) / led_anim_top) * led_timer_top / 100;
		r = (led_anim_mode == LED_BREATHR) ? val:0;
		g = (led_anim_mode == LED_BREATHG) ? val:0;
	}
	else if (led_anim_progress < led_anim_top / 5) //20%
	{
		val = ((1000 * (led_anim_progress - led_anim_top / 10)) / led_anim_top) * led_timer_top / 100;
		r = (led_anim_mode == LED_BREATHR) ? (led_timer_top - val):0;
		g = (led_anim_mode == LED_BREATHG) ? (led_timer_top - val):0;
	}

	led_set(r, g, b);

	led_anim_progress++;
	if (led_anim_progress > led_anim_top)
		led_anim_progress = 0;
}

ISR(LED_TIMER2_OVF)
{
	if (lcd_bckl_actual != lcd_bckl_wanted)
	{
		if (lcd_bckl_actual < lcd_bckl_wanted)
			lcd_bckl_actual++;
		else
			lcd_bckl_actual--;


		led_timer2.SetCompare(timer_B, lcd_bckl_actual);
		bat_en_high(BAT_EN_LCD);
	}
	else
	{
		led_timer2.DisableInterrupts(timer_overflow);
		if (lcd_bckl_actual == 0)
			bat_en_low(BAT_EN_LCD);
	}
}

void led_init()
{
	//enable timer
	LED_TIMER1_PWR_ON;
	LED_TIMER2_PWR_ON;

	//enable gpio
	GpioSetDirection(LEDR, OUTPUT);
	GpioSetDirection(LEDG, OUTPUT);
	GpioSetDirection(LEDB, OUTPUT);
	GpioSetDirection(BCKL, OUTPUT);

	GpioSetInvert(LEDR, ON);
	GpioSetInvert(LEDG, ON);
	GpioSetInvert(LEDB, ON);
	GpioSetInvert(BCKL, OFF);

	GpioWrite(LEDR, LOW);
	GpioWrite(LEDG, LOW);
	GpioWrite(LEDB, LOW);
	GpioWrite(BCKL, LOW);

	//init timer
	led_timer1.Init(LED_TIMER1, timer_div1024);
			    		    // GREEN | RED
	led_timer1.EnableOutputs(timer_A | timer_B);
	led_timer1.SetTop(led_timer_top);
	led_timer1.SetMode(timer_pwm);

	led_timer2.Init(LED_TIMER2, timer_div1024);
			    	          //BLUE | BCKL
	led_timer2.EnableOutputs(timer_A | timer_B);
	led_timer2.SetTop(led_timer_top);
	led_timer2.SetMode(timer_pwm);


	//start timer
	led_set(0, 0, 0);
	led_timer1.Start();
	led_timer2.Start();
}

void led_set(uint16_t red, uint16_t green, uint16_t blue)
{
	if (red == 0 && green == 0 && blue == 0)
		bat_en_low(BAT_EN_LED);
	else
		bat_en_high(BAT_EN_LED);

	led_timer1.SetCompare(timer_B, red);
	led_timer1.SetCompare(timer_A, green);
	led_timer2.SetCompare(timer_A, blue);
}

void led_anim(uint8_t mode)
{
	led_anim(mode, ANIM_DEFAULT_TOP);
}

void led_anim(uint8_t mode, uint16_t top)
{
	led_anim_mode = mode;

	if (mode == LED_NO_ANIM)
	{
		led_set(0, 0, 0);
		led_timer1.DisableInterrupts(timer_overflow);
		return;
	}

	led_anim_top = top;
	led_anim_progress = 0;
	led_timer1.EnableInterrupts(timer_overflow);
}

void lcd_bckl(uint8_t val)
{
	lcd_bckl_wanted = val;

	if (lcd_bckl_wanted != lcd_bckl_actual)
		led_timer2.EnableInterrupts(timer_overflow);
}
