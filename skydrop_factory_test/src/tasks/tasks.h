/*
 * tasks.h
 *
 *  Created on: 24.7.2014
 *      Author: horinek
 */

#ifndef TASKS_H_
#define TASKS_H_

#include "../skydrop.h"

#include "task_usb/task_usb.h"
#include "task_powerdown.h"
#include "task_active.h"
#include "task_update.h"

#define TASK_POWERDOWN	0
#define TASK_USB		1
#define TASK_ACTIVE		2
#define TASK_UPDATE		3

#define NO_TASK			0xFF

#define TASK_IRQ_BUTTON_L			0
#define TASK_IRQ_BUTTON_M			1
#define TASK_IRQ_BUTTON_R			2
#define TASK_IRQ_BAT				3
#define TASK_IRQ_USB				4
#define TASK_IRQ_MOUNT_ERROR		5


class SleepLock
{
private:
	volatile bool active;
public:
	SleepLock();
	void Lock();
	void Unlock();
	bool Active();

};

void task_timer_setup(bool full_speed = true);
void task_timer_stop();

void task_init();
void task_set(uint8_t task);
void task_rgui();

uint32_t task_get_ms_tick_once();
uint32_t task_get_ms_tick();

void task_loop();
void task_system_loop();
void task_sleep();
void task_irqh(uint8_t type, uint8_t * buff);

extern volatile uint8_t actual_task;

#endif /* TASKS_H_ */
