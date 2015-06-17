/*
 * gps_l80.h
 *
 *  Created on: 23.1.2015
 *      Author: horinek
 */

#ifndef GPS_L80_H_
#define GPS_L80_H_

#include "../../common.h"

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
