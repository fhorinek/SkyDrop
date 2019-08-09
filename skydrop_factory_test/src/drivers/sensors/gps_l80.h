/*
 * gps_l80.h
 *
 *  Created on: 23.1.2015
 *      Author: horinek
 */

#ifndef GPS_L80_H_
#define GPS_L80_H_

#include "../../common.h"

// The product specifications of the sensor says, that GPS position has a horizontal accuracy of 2.5m.
// We understand that for a HDOP of 1.0
#define L80_HACCURACY 2.5

// This is the vertical accuracy in m. It is not found anywhere but estimated by tilmann-at-bubecks.de
#define L80_VACCURACY 30.0

void gps_init();
void gps_change_uart_baudrate();
void gps_start();
void gps_stop();
void gps_parse(Usart *);
void gps_step();
void gps_set_baudrate();

bool gps_selftest();

void gps_normal();
void gps_detail();

#endif /* GPS_L80_H_ */
