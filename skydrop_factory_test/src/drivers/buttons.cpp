/*
 * buttons.cc
 *
 *  Created on: 27.11.2014
 *      Author: horinek
 */

#include "buttons.h"
#include "../tasks/tasks.h"
#include "../fc/conf.h"

uint8_t buttons_state[] = {BS_IDLE, BS_IDLE, BS_IDLE};
uint32_t buttons_counter[] = {0, 0, 0};

SleepLock button_lock;

ISR(SWITCH_INT)
{
//	buttons_step();
}

void buttons_init()
{
	GpioSetPull(SWITCH1, gpio_pull_up);
	GpioSetInvert(SWITCH1, ON);

	GpioSetPull(SWITCH2, gpio_pull_up);
	GpioSetInvert(SWITCH2, ON);

	GpioSetPull(SWITCH3, gpio_pull_up);
	GpioSetInvert(SWITCH3, ON);

	GpioSetInterrupt(SWITCH1, gpio_interrupt0, gpio_bothedges);
	GpioSetInterrupt(SWITCH2, gpio_interrupt0, gpio_bothedges);
	GpioSetInterrupt(SWITCH3, gpio_interrupt0, gpio_bothedges);
}

void buttons_deinit()
{
	GpioSetPull(SWITCH1, gpio_totem);
//	Middle need to be active to wake up the device
//	GpioSetPull(SWITCH2, gpio_totem);
	GpioSetPull(SWITCH3, gpio_totem);
}


//#define task_irqh(a,b)

void button_handle(uint8_t index, uint8_t state)
{
	uint32_t time = task_get_ms_tick();
	uint8_t res;

	if (state == 1) //button is pressed
	{
		switch(buttons_state[index])
		{
			case(BS_IDLE):
				res = BE_HOLD;
				task_irqh(TASK_IRQ_BUTTON_L + index, &res);
				buttons_state[index] = BS_1DOWN;
				buttons_counter[index] = time;
			break;

			case(BS_1DOWN):
				if (time - buttons_counter[index] > BUTTON_LONG)
				{
					res = BE_LONG;
					task_irqh(TASK_IRQ_BUTTON_L + index, &res);
					buttons_state[index] = BS_PRESSED;
				}
			break;

			case(BS_WAIT):
				if (time - buttons_counter[index] < BUTTON_WAIT)
				{
					res = BE_DBL_CLICK;
					task_irqh(TASK_IRQ_BUTTON_L + index, &res);
					buttons_state[index] = BS_2DOWN;
				}
				else
				{
					res = BE_HOLD;
					task_irqh(TASK_IRQ_BUTTON_L + index, &res);
					buttons_state[index] = BS_1DOWN;
					buttons_counter[index] = time;
				}

			break;
		}

	}
	else // button is released
	{
		switch(buttons_state[index])
		{
			case(BS_RESET):
				buttons_state[index] = BS_IDLE;
			break;

			case(BS_1DOWN):
				{
					res = BE_CLICK;
					task_irqh(TASK_IRQ_BUTTON_L + index, &res);

					buttons_state[index] = BS_WAIT;
					buttons_counter[index] = time;
				}
			break;

			case(BS_WAIT):
				if (time  - buttons_counter[index] > BUTTON_WAIT)
					buttons_state[index] = BS_IDLE;
			break;

			case(BS_2DOWN):
				buttons_state[index] = BS_IDLE;
			break;

			case(BS_PRESSED):
				res = BE_RELEASED;
				task_irqh(TASK_IRQ_BUTTON_L + index, &res);
				buttons_state[index] = BS_IDLE;
			break;
		}

	}
}

void buttons_step()
{
	if (config.gui.disp_flags & CFG_DISP_FLIP)
	{
		button_handle(2, GpioRead(SWITCH1));
		button_handle(0, GpioRead(SWITCH3));
	}
	else
	{
		button_handle(0, GpioRead(SWITCH1));
		button_handle(2, GpioRead(SWITCH3));
	}
	button_handle(1, GpioRead(SWITCH2));

	if (buttons_state[0] > BS_IDLE || buttons_state[1] > BS_IDLE || buttons_state[2] > BS_IDLE)
		button_lock.Lock();
	else
		button_lock.Unlock();
}

bool buttons_read(uint8_t index)
{
	return (buttons_state[index] > BS_DEBOUNCE);
}

bool button_hold(uint8_t index)
{
	return (buttons_state[index] == BS_PRESSED);
}

bool button_in_reset(uint8_t index)
{
	return (buttons_state[index] == BS_RESET);
}

void buttons_reset()
{
	buttons_state[0] = BS_RESET;
	buttons_state[1] = BS_RESET;
	buttons_state[2] = BS_RESET;
}
