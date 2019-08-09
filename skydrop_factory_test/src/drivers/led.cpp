#include "led.h"

Timer led_timer1;
Timer led_timer2;

volatile uint16_t lcd_bckl_actual = 0;
volatile uint16_t lcd_bckl_wanted = 0;

#define led_timer_top 255

#define MS_TO_TICKS(A)	(A / 2.048)

#define LED_NOTIFY_DURATION		MS_TO_TICKS(50)
#define LED_NOTIFY_PAUSE		MS_TO_TICKS(500)
#define LED_NOTIFY_LOOP			MS_TO_TICKS(2000)

//odd numbers are pause states
#define LED_TASK_BAT_LOW	0
#define LED_TASK_NO_GPS		2
#define LED_TASK_BT_NC		4
#define LED_TASK_BT_OK		6
#define LED_TASK_IDLE		8

uint16_t led_task_cnt = 0;
uint8_t led_task_state = LED_TASK_IDLE;

ISR(LED_TIMER1_OVF)
{

	if (led_task_cnt > 0)
	{
		led_task_cnt--;
		return;
	}

	switch (led_task_state)
	{
		case(LED_TASK_BAT_LOW):
			if (battery_per < 20)
			{
				led_task_cnt = LED_NOTIFY_DURATION;
				led_set(0xFF, 0, 0);
				break;
			}
			led_task_state = LED_TASK_NO_GPS;

		case(LED_TASK_NO_GPS):
			if (config.connectivity.use_gps && !fc.gps_data.valid)
			{
				led_task_cnt = LED_NOTIFY_DURATION;
				led_set(0xFF, 0xFF, 0);
				break;
			}
			led_task_state = LED_TASK_BT_NC;

		case(LED_TASK_BT_NC):
			if (bt_ready() && !bt_device_active())
			{
				led_task_cnt = LED_NOTIFY_DURATION;
				led_set(0, 0xFF, 0xFF / 2);
				break;
			}
			led_task_state = LED_TASK_BT_OK;

		case(LED_TASK_BT_OK):
			if (bt_ready() && bt_device_active())
			{
				led_task_cnt = LED_NOTIFY_DURATION;
				led_set(0, 0, 0xFF);
				break;
			}
			led_task_state = LED_TASK_IDLE;

		case(LED_TASK_IDLE):
			led_task_cnt = LED_NOTIFY_LOOP;
			led_task_state = 0;
		return;

		//pause between alarms
		default:
			led_set(0, 0, 0);
			led_task_cnt = LED_NOTIFY_PAUSE;
	}

	led_task_state++;
}

//LCD bcklight control
ISR(LED_TIMER2_OVF)
{
	if (lcd_bckl_actual != lcd_bckl_wanted)
	{
		if (lcd_bckl_actual < lcd_bckl_wanted)
			lcd_bckl_actual++;
		else
			lcd_bckl_actual--;


		led_timer2.SetCompare(timer_B, lcd_bckl_actual / 8);
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
	led_timer1.Init(LED_TIMER1, timer_div256);
			    		    // GREEN | RED
	led_timer1.EnableOutputs(timer_A | timer_B);
	led_timer1.SetTop(led_timer_top);
	led_timer1.SetMode(timer_pwm);

	led_timer2.Init(LED_TIMER2, timer_div256);
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

void lcd_bckl(uint8_t val)
{
	lcd_bckl_wanted = val * 8;

	if (lcd_bckl_wanted != lcd_bckl_actual)
		led_timer2.EnableInterrupts(timer_overflow);
}

void led_notify_enable()
{
	led_timer1.EnableInterrupts(timer_overflow);
}

void led_notify_disable()
{
	led_timer1.DisableInterrupts(timer_overflow);
}
