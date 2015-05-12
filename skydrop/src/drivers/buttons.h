/*
 * buttons.h
 *
 *  Created on: 27.11.2014
 *      Author: horinek
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_

#include "../common.h"
#include "../drivers/uart.h"

#define B_LEFT		0
#define B_MIDDLE	1
#define B_RIGHT		2

#define BS_IDLE		0
#define BS_DEBOUNCE	1
#define BS_1DOWN	2
#define BS_WAIT		3
#define BS_2DOWN	4
#define BS_PRESSED	5

//buttons events NEVER change the indexes
#define BE_HOLD			0	//button was debounced and it is hold
#define BE_RELEASED		1	//button was released
#define BE_CLICK		2	//button was released and made single click
#define BE_DBL_CLICK	3	//button was released after second click
#define BE_LONG			4	//button is held a long time

void buttons_init();
void buttons_step();
bool buttons_read(uint8_t index);

#endif /* BUTTONS_H_ */
