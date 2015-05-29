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
void gps_change_baud();
void gps_stop();
void gps_parse(Usart *);
void gps_step();
void gps_setup();

bool gps_selftest();

#endif /* GPS_L80_H_ */
