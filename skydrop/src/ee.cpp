/*
 * ee.cpp
 *
 *  Created on: 6. 4. 2020
 *      Author: horinek
 */

#include "ee.h"

void ee_update_block(void * ram_ptr, void * ee_ptr, uint16_t size)
{
	eeprom_busy_wait();
	eeprom_update_block(ram_ptr, ee_ptr, size);
}

void ee_read_block(void * ram_ptr, void * ee_ptr, uint16_t size)
{
	eeprom_busy_wait();
	eeprom_read_block(ram_ptr, ee_ptr, size);
}

