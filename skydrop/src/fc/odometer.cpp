/*
 * odometer.cpp
 *
 *  Created on: 16.02.2017
 *      Author: tilmann@bubecks.de
 */

#include "odometer.h"
#include "waypoint.h"
#include "fc.h"

//#include "../debug_on.h"

#include "../drivers/audio/sequencer.h"

MK_SEQ(next_wpt, ARR({750, 0, 1000, 0, 1000, 0}), ARR({250, 150, 250, 150, 250, 150}));
MK_SEQ(last_wpt, ARR({750, 0, 1000, 0, 1250, 0}), ARR({250, 150, 250, 150, 250, 150}));

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
					int32_t lat2, int32_t lon2)
{
	double dX = ((double)lon1 - lon2) / GPS_MULT;
	double dY = ((double)lat1 - lat2) / GPS_MULT;
	return ((int16_t)to_degrees(atan2(dX, dY)) + 360) % 360;
}

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
			 	 	 	 int32_t lat2, int32_t lon2)
{
	double dx, dy;   // in cm
	double lat;

	// Compute the average lat of lat1 and lat2 to get the width of a
	// 1 degree cell at that position of the earth:
	lat = (lat1 + lat2) / 2 / GPS_MULT * (M_PI / 180.0);

	// 111.3 km (in cm) is the width of 1 degree
	// disp.get_cos() is not usable here, because we deal with fractions of a degree.
	dx = cos(lat) * 11130000 * abs(lon1 - lon2) / GPS_MULT;
	dy = 1.0      * 11130000 * abs(lat1 - lat2) / GPS_MULT;

	return (uint32_t)sqrt(dx * dx + dy * dy);
}

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
			 	 	 	 int32_t lat2, int32_t lon2, double alt2)
{
	uint32_t dx, dy;
	float da;

	dx = gps_distance_2d(lat1, lon1, lat1, lon2);
	dy = gps_distance_2d(lat1, lon1, lat2, lon1);
	da = abs(alt1 - alt2) * 100;                 // convert from m to cm

	return (uint32_t)sqrt((double)dx * dx + (double)dy * dy + da * da);
}

/**
 * The GPS position has changed. Compute the distance to the previous
 * position and update the odometer accordingly.
 */
void odometer_step()
{
	#define NO_LAT_DATA  ((int32_t)2147483647)

	static int32_t last_lat = NO_LAT_DATA;
	static int32_t last_lon;
	static float last_alt;

	if (fc.gps_data.new_sample & FC_GPS_NEW_SAMPLE_ODO)
		fc.gps_data.new_sample &= ~FC_GPS_NEW_SAMPLE_ODO;
	else
		return;

	if (fc.flight.home_valid)
	{
		fc.flight.home_bearing = gps_bearing(config.home.lat, config.home.lon, fc.gps_data.latitude, fc.gps_data.longtitude );
		fc.flight.home_distance = gps_distance_2d(fc.gps_data.latitude, fc.gps_data.longtitude, config.home.lat, config.home.lon) / 100000.0;   // cm to km
	}

	if ( fc.flight.waypoint_no != 0 )
	{
//		DEBUG("lat1 %ld\n", fc.flight.next_waypoint.lat);
//		DEBUG("lon1 %ld\n", fc.flight.next_waypoint.lon);
//		DEBUG("lat2 %ld\n", fc.gps_data.latitude);
//		DEBUG("lon2 %ld\n", fc.gps_data.longtitude);


		fc.flight.next_waypoint.bearing = gps_bearing(fc.flight.next_waypoint.lat, fc.flight.next_waypoint.lon, fc.gps_data.latitude, fc.gps_data.longtitude );
		fc.flight.next_waypoint.distance = gps_distance_2d(fc.gps_data.latitude, fc.gps_data.longtitude, fc.flight.next_waypoint.lat, fc.flight.next_waypoint.lon) / 100.0;   // cm to m
		fc.flight.next_waypoint.distance -= fc.flight.next_waypoint.radius_m;

//		DEBUG("ber %d\n", fc.flight.next_waypoint.bearing);
//		DEBUG("dis %0.2f\n", fc.flight.next_waypoint.distance);


		if ( fc.flight.next_waypoint.distance <= 0 )
		{

			// We reached the waypoint. Go to next.
			if (!waypoint_goto_next() )
			{
				fc.flight.waypoint_no = 0;
				fc.flight.next_waypoint.bearing = 0;
				fc.flight.next_waypoint.distance = 0;
				fc.flight.next_waypoint.name[0] = 0;
				gui_showmessage_P(PSTR("Navigation\nfinished."));

				seq_start(&last_wpt, config.gui.alert_volume);
			}
			else
			{
				seq_start(&next_wpt, config.gui.alert_volume);
			}
		}

		fc.flight.next_waypoint.distance /= 1000;             // m to km.
	}

	// Do we already have a previous GPS point?
	if (last_lat != NO_LAT_DATA)
	{
		uint32_t v = gps_distance_3d(last_lat, last_lon, last_alt, fc.gps_data.latitude, fc.gps_data.longtitude, fc.gps_data.altitude);

		//calculated speed in knots
		uint16_t calc_speed = (v * FC_MPS_TO_KNOTS) / 100;

		//do not add when gps speed is < 1 km/h
		//do not add when difference between calculated speed and gps speed is > 10 km/h
		if (abs(calc_speed - fc.gps_data.ground_speed) < FC_ODO_MAX_SPEED_DIFF && fc.gps_data.ground_speed > FC_ODO_MIN_SPEED)
			fc.odometer += v;
	}

	// Save the current GPS position for the next step
	last_lat = fc.gps_data.latitude;
	last_lon = fc.gps_data.longtitude;
	last_alt = fc.gps_data.altitude;
}
