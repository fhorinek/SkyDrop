/*
 * odometer.h
 *
 *  Created on: 16.02.2017
 *      Author: tilmann@bubecks.de
 */

#ifndef FC_NAVIGATION_H_
#define FC_NAVIGATION_H_

#include "../common.h"


uint32_t gps_distance(int32_t lat1, int32_t lon1, int32_t lat2, int32_t lon2, bool FAI, int16_t * bearing = NULL);
void gps_destination(float lat1, float lon1, float angle, float distance_km, float * lat2, float * lon2);

void navigation_init();

/**
 * The GPS position has changed. Compute the distance to the previous
 * position and update the odometer accordingly.
 */

void navigation_step();


#endif /* FC_NAVIGATION_H_ */
