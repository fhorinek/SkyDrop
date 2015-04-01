/*
 * task_powerdown.h
 *
 *  Created on: 6.11.2014
 *      Author: horinek
 */

#ifndef TASK_POWERDOWN_H_
#define TASK_POWERDOWN_H_

#include "tasks.h"

void task_powerdown_init();
void task_powerdown_stop();
void task_powerdown_loop();
void task_powerdown_irqh(uint8_t type, uint8_t * buff);

#endif /* TASK_POWERDOWN_H_ */
