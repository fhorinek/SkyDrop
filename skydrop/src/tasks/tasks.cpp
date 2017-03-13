#include "tasks.h"
#include "../xlib/core/watchdog.h"
#include "../fc/conf.h"


void (* task_init_array[])() =
{task_powerdown_init, task_usb_init, task_active_init, task_update_init};

void (* task_stop_array[])() =
{task_powerdown_stop, task_usb_stop, task_active_stop, task_update_stop};

void (* task_loop_array[])() =
{task_powerdown_loop, task_usb_loop, task_active_loop, task_update_loop};

void (* task_irqh_array[])(uint8_t type, uint8_t * buff) =
{task_powerdown_irqh, task_usb_irqh, task_active_irqh, task_update_irqh};

//task variables
Timer task_timer;
volatile uint32_t task_timer_high;
volatile uint8_t task_sleep_lock = 0;

volatile uint32_t fine_timer_high;

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

ISR(TASK_TIMER_CMPA)
{
	if (SP < debug_min_stack_pointer)
		debug_min_stack_pointer = SP;

	if (debug_max_heap_pointer < (uint16_t)freeRam())
		debug_max_heap_pointer = freeRam();

	task_timer_high += 512ul;
	fine_timer_high += 64000ul;

	uint16_t val = task_timer.GetValue();
	task_timer.SetValue(val - 63999);
}

ISR(USB_CONNECTED_IRQ)
{
	//dummy ISR
	//just wake up the device
	//usb_in is checked in main loop
}

uint32_t old_tick = 0;

uint32_t task_get_ms_tick_once()
{
	uint32_t res;

	cli();
	res = (task_timer_high) + (uint32_t)(task_timer.GetValue() / 125);
	sei();

	if (res < old_tick)
	{
		assert(0);
		DEBUG(" res=%lu\n", res);
		DEBUG(" old=%lu\n", old_tick);
	}

	old_tick = res;

	return res;
}

uint32_t task_get_ms_tick()
{
	return old_tick;
}


uint32_t fine_timer_get()
{
	uint32_t res;

	cli();
	res = (fine_timer_high) + (uint32_t)(task_timer.GetValue());
	sei();

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
	task_timer.SetInterruptPriority(HIGH);
	task_timer.EnableInterrupts(timer_compareA);
	task_timer.SetValue(0);
	task_timer.SetCompare(timer_A, 63999); //125 == 1ms
	task_timer_high = 0;
	old_tick = 0;

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

	ewdt_init();
}

void task_set(uint8_t task)
{
	new_task = task;
}

void task_loop()
{
	ewdt_reset();
	if (actual_task != NO_TASK)
		task_loop_array[actual_task]();
}

uint64_t loop_start = 0;

void task_system_loop()
{
	ewdt_reset();

	//task switching outside interrupt
	if (new_task != actual_task)
	{
		DEBUG("Switching task %d to %d\n", actual_task, new_task);
		if (actual_task != NO_TASK)
		{
			task_stop_array[actual_task]();

			//XXX: this will guarantee that task switched from the powerdown task will be vanilla
			if (new_task == TASK_POWERDOWN)
			{
				SystemReset();
			}

			//XXX: usb is bit unstable when it is switched from another task, this is hack
			if (new_task == TASK_USB && actual_task != NO_TASK)
			{
				SystemReset();
			}
		}

		actual_task = new_task;

		task_init_array[actual_task]();
	}

	//check USB and send IRQ
	if (usb_state != USB_CONNECTED)
	{
		usb_state = USB_CONNECTED;

		battery_force_update();
		task_irqh(TASK_IRQ_USB, &usb_state);
	}

	buttons_step();
	if (powerdown_lock.Active() == false)
	{
		if (battery_step())
			task_irqh(TASK_IRQ_BAT, NULL);
	}
}

uint16_t wake_ups = 0;
uint32_t wake_next = 0;

uint32_t fine_last = 0;
uint32_t fine_acc = 0;

void task_sleep()
{
	fine_acc += fine_timer_get() - fine_last;

	if (task_sleep_lock == 0)
	{
		SystemPowerIdle();
	}

	task_get_ms_tick_once();
	fine_last = fine_timer_get();
	wake_ups++;

	if (wake_next < task_get_ms_tick())
	{
		//uint8_t usage = fine_acc / 6250;
//		DEBUG("CPU: %3u%% (%u irq)\n", usage, wake_ups / 5);

		wake_ups = 0;
		fine_acc = 0;

		wake_next = task_get_ms_tick() + 5000;
	}
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

