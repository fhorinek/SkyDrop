#include "tasks.h"
#include "../xlib/core/watchdog.h"
#include "../fc/conf.h"


void (* task_init_array[])() =
{task_powerdown_init, task_usb_init, task_active_init};

void (* task_stop_array[])() =
{task_powerdown_stop, task_usb_stop, task_active_stop};

void (* task_loop_array[])() =
{task_powerdown_loop, task_usb_loop, task_active_loop};

void (* task_irqh_array[])(uint8_t type, uint8_t * buff) =
{task_powerdown_irqh, task_usb_irqh, task_active_irqh};

//task variables
Timer task_timer;
volatile uint32_t task_timer_high;
volatile uint8_t task_sleep_lock = 0;

volatile uint8_t actual_task = NO_TASK;
volatile uint8_t new_task = TASK_POWERDOWN;

uint8_t usb_state;

SleepLock powerdown_lock;

SleepLock::SleepLock()
{
	this->active = false;
}

void SleepLock::Lock()
{
	if (this->active == false)
	{
		this->active = true;
		task_sleep_lock++;
	}
}

void SleepLock::Unlock()
{
	if (this->active == true)
	{
		this->active = false;
		task_sleep_lock--;
	}
}

bool SleepLock::Active()
{
	return this->active;
}

ISR(TASK_TIMER_OVF)
{
	task_timer_high++;
}

ISR(USB_CONNECTED_IRQ)
{
	//dummy
	//just wake up the device
	//usb_in is checked in main loop
}

uint32_t task_get_ms_tick()
{
	uint32_t res = (task_timer_high * 512ul) + (uint32_t)(task_timer.GetValue() / 125);

	return res;
}

void task_timer_setup(bool full_speed)
{
	TASK_TIMER_PWR_ON;

	if (full_speed)
		task_timer.Init(TASK_TIMER, timer_div256);
	else
		task_timer.Init(TASK_TIMER, timer_div4);

	task_timer.Stop();
	task_timer.EnableInterrupts(timer_overflow);
	task_timer.SetValue(0);
	task_timer.SetTop(63999); //125 == 1ms
	task_timer_high = 0;

	task_timer.Start();

}

void task_timer_stop()
{
	task_timer.Stop();

	TASK_TIMER_PWR_OFF;
}

void task_init()
{
	task_timer_setup();
	USB_CONNECTED_IRQ_ON;

	powerdown_lock.Unlock();

	//if ftest is not done
	if (!cfg_factory_passed())
		task_set(TASK_ACTIVE);

	//if is USB connected go directly to USB task
	if ((usb_state = USB_CONNECTED))
		task_set(TASK_USB);

	wdt_init(wdt_2s);
}

void task_set(uint8_t task)
{
	new_task = task;
}

void task_loop()
{
	wdt_reset();
	if (actual_task != NO_TASK)
		task_loop_array[actual_task]();
}

uint64_t loop_start = 0;

void task_system_loop()
{
	wdt_reset();

	//task switching outside interrupt
	if (new_task != actual_task)
	{
		DEBUG("Switching task %d to %d\n", actual_task, new_task);
		if (actual_task != NO_TASK)
		{
			task_stop_array[actual_task]();

			//XXX: this will guarantee that task switched from the powerdown task will be vanilla
			if (new_task == TASK_POWERDOWN)
				SystemReset();
		}

		actual_task = new_task;

		task_init_array[actual_task]();
	}

	//check USB and send IRQ
	if (usb_state != USB_CONNECTED)
	{
		usb_state = USB_CONNECTED;
		task_irqh(TASK_IRQ_USB, &usb_state);
	}

	buttons_step();
	if (powerdown_lock.Active() == false)
	{
		battery_step();
	}
}

void task_sleep()
{
//	IO1_HIGH;
	if (task_sleep_lock == 0)
	{
		SystemPowerIdle();
	}
//	IO1_LOW;
}

void task_irqh(uint8_t type, uint8_t * buff)
{
	if (actual_task != new_task)
	{
		DEBUG("IGNORING IRQ\n");
	}
	else
	{
		task_irqh_array[actual_task](type, buff);
	}
}

