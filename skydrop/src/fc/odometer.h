/*
 * odometer.h
 *
 *  Created on: 16.02.2017
 *      Author: tilmann@bubecks.de
 */

#ifndef FC_ODOMETER_H_
#define FC_ODOMETER_H_

#include <stdint.h>
#include <math.h>

/**
 * Latitude and longitude are multiplied with GPS_MULT to get fixed point integers.
 * E.g. 48.5 will be 48500000.
 */
#define GPS_MULT			10000000ul

/**
 * Convert an angle given in radians to degree.
 */
inline double to_degrees(double radians) {
    return radians * (180.0 / M_PI);
}

/**
 * Returns the bearing from lat1/lon1 to lat2/lon2. All parameters
 * must be given as fixed integers multiplied with GPS_MULT.
 *
 * \param lat1 the latitude of the 1st GPS point
 * \param lon1 the longitude of the 1st GPS point
 * \param lat2 the latitude of the 2nd GPS point
 * \param lon2 the longitude of the 2nd GPS point
 *
 * \return the bearing in degrees (0-359, where 0 is north, 90 is east, ...).
 */
int16_t gps_bearing(int32_t lat1, int32_t lon1,
		    		int32_t lat2, int32_t lon2);

/**
 * Compute the distance between two GPS points in 2 dimensions
 * (without altitude). Latitude and longitude parameters must be given as fixed integers
 * multiplied with GPS_MULT.
 *
 * \param lat1 the latitude of the 1st GPS point
 * \param lon1 the longitude of the 1st GPS point
 * \param lat2 the latitude of the 2nd GPS point
 * \param lon2 the longitude of the 2nd GPS point
 *
 * \return the distance in cm.
 */
uint32_t gps_distance_2d(int32_t lat1, int32_t lon1,
			 	 	 	 int32_t lat2, int32_t lon2);

/**
 * Compute the distance between two GPS points in 3 dimensions
 * (including altitude). Latitude and longitude parameters must be
 * given as fixed integers multiplied with GPS_MULT.
 *
 * \param lat1 the latitude of the 1st GPS point
 * \param lon1 the longitude of the 1st GPS point
 * \param alt1 the altitude of the 1st GPS point (in m)
 * \param lat2 the latitude of the 2nd GPS point
 * \param lon2 the longitude of the 2nd GPS point
 * \param alt2 the altitude of the 2nd GPS point (in m)
 *
 * \return the distance in cm.
 */
uint32_t gps_distance_3d(int32_t lat1, int32_t lon1, double alt1,
			 	 	 	 int32_t lat2, int32_t lon2, double alt2);

/**
 * The GPS position has changed. Compute the distance to the previous
 * position and update the odometer accordingly.
 */
void odometer_step();


#endif /* FC_ODOMETER_H_ */
