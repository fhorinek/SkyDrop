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

#define AIRSPACE_INVALID 999
#define AIRSPACE_LONGEST_DISTANCE (255*64)

#endif /* FC_AIRSPACE_H */
