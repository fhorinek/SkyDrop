/*
 * airspace.h
 *
 *  Created on: 27.11.2018
 *      Author: tilmann@bubecks.de
 */

#ifndef FC_AIRSPACE_H
#define FC_AIRSPACE_H

#include "../common.h"

void airspace_init();
void airspace_read_pos(int32_t latitude, int32_t longtitude);
void airspace_is_forbidden();
void airspace_get_angle();
void airspace_get_distance();
void airspace_step();

#define AIRSPACE_INVALID 			999
#define AIRSPACE_LONGEST_DISTANCE 	(0xFF * 64)

#define AIR_AGL_FLAG 				0x80
#define AIR_250ft 					250
#define AIR_250ft_to_m 				76.196281622

uint16_t airspace_convert_alt_ft(uint8_t raw_alt);
uint16_t airspace_convert_alt_m(uint8_t raw_alt);

#endif /* FC_AIRSPACE_H */
