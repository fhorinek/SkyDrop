/*
 * task_update.h
 *
 *  Created on: 26.8.2015
 *      Author: horinek
 */

#ifndef TASK_UPDATE_H_
#define TASK_UPDATE_H_

#include "tasks.h"

#define UPDATE_IDLE			0
#define UPDATE_CHECK_EE		1
#define UPDATE_UNPACK		2
#define UPDATE_CHECK_FW		3
#define UPDATE_EE			4
#define UPDATE_FAIL			5

extern uint8_t update_state;

struct update_header
{
	uint32_t build_number;
	uint32_t file_size;
	uint32_t eeprom_size;
	uint8_t eeprom_crc;
	uint32_t flash_size;
	uint8_t flash_crc;
};

void task_update_init();
void task_update_stop();
void task_update_loop();
void task_update_irqh(uint8_t type, uint8_t * buff);

#endif /* TASK_UPDATE_H_ */
