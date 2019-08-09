/*
 * task_gui.h
 *
 *  Created on: 20.1.2015
 *      Author: horinek
 */

#ifndef TASK_ACTIVE_H_
#define TASK_ACTIVE_H_

#include "tasks.h"

void task_active_init();
void task_active_stop();
void task_active_loop();
void task_active_irqh(uint8_t type, uint8_t * buff);

void task_special_files_handle();

#endif /* TASK_ACTIVE_H_ */
