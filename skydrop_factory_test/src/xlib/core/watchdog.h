/*
 * watchdog.h
 *
 *  Created on: 2.6.2015
 *      Author: horinek
 */

#ifndef XLIB_CORE_WATCHDOG_H_
#define XLIB_CORE_WATCHDOG_H_

#include "../common.h"

typedef enum {
	wdt_8ms = 	1 << 2,
	wdt_16ms = 	2 << 2,
	wdt_32ms = 	3 << 2,
	wdt_64ms = 	4 << 2,
	wdt_128ms = 5 << 2,
	wdt_256ms = 6 << 2,
	wdt_512ms = 7 << 2,
	wdt_1s = 	8 << 2,
	wdt_2s = 	9 << 2,
	wdt_4s = 	10 << 2,
	wdt_8s = 	11 << 2,
} wdt_timeout;


void wdt_init(wdt_timeout to);
void wdt_deinit();


#endif /* XLIB_CORE_WATCHDOG_H_ */
