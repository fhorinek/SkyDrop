/*
 * navigation.cpp
 *
 *  Created on: 16.02.2017
 *      Author: tilmann@bubecks.de
 */

#include "navigation.h"
#include "waypoint.h"
#include "fc.h"

//#include "../debug_on.h"

#include "../drivers/audio/sequencer.h"

MK_SEQ(next_wpt, ARR({750, 0, 1000, 0, 1000, 0}), ARR({250, 150, 250, 150, 250, 150}));
MK_SEQ(last_wpt, ARR({750, 0, 1000, 0, 1250, 0}), ARR({250, 150, 250, 150, 250, 150}));

#define FAI_EARTH_RADIUS 6371


void gps_get_kx_ky(float lat, float * kx, float * ky)
{
	float fcos = cos(to_radians(lat));
	float cos2 = 2. * fcos * fcos - 1.;
	float cos3 = 2. * fcos * cos2 - fcos;
	float cos4 = 2. * fcos * cos3 - cos2;
	float cos5 = 2. * fcos * cos4 - cos3;

    //multipliers for converting longitude and latitude
    //degrees into distance (http://1.usa.gov/1Wb1bv7)
	*kx = (111.41513 * fcos - 0.09455 * cos3 + 0.00012 * cos5);
    *ky = (111.13209 - 0.56605 * cos2 + 0.0012 * cos4);
}

void gps_destination(float lat1, float lon1, float angle, float distance_km,
					float * lat2, float * lon2)
{
	angle = to_radians(angle);
	float dx = sin(angle) * distance_km;
	float dy = cos(angle) * distance_km;

	float kx, ky;
	gps_get_kx_ky(lat1, &kx, &ky);

	*lon2 = lon1 + dx / kx;
	*lat2 = lat1 + dy / ky;
}

/**
 * Compute the distance between two GPS points in 2 dimensions
 * (without altitude). Latitude and longitude parameters must be given as fixed integers
 * multiplied with GPS_COORD_MULT.
 *
 * \param lat1 the latitude of the 1st GPS point
 * \param lon1 the longitude of the 1st GPS point
 * \param lat2 the latitude of the 2nd GPS point
 * \param lon2 the longitude of the 2nd GPS point
 * \param FAI use FAI sphere instead of WGS ellipsoid
 * \param bearing pointer to bearing (NULL if not used)
 *
 * \return the distance in cm.
 */
uint32_t gps_distance(int32_t lat1, int32_t lon1,
			 	 	  int32_t lat2, int32_t lon2, bool FAI, int16_t * bearing)
{
//	DEBUG("*gps_distance\n");


	float d_lon = (lon2 - lon1) / (float)GPS_COORD_MUL;
	float d_lat = (lat2 - lat1) / (float)GPS_COORD_MUL;

//	DEBUG("#d_lon=%0.10f\n", d_lon);
//	DEBUG("#d_lat=%0.10f\n", d_lat);
//
//	DEBUG("lat1=%li\n", lat1);
//	DEBUG("lon1=%li\n", lon1);
//	DEBUG("lat2=%li\n", lat2);
//	DEBUG("lon2=%li\n", lon2);

	uint32_t dist;

	if (FAI)
	{
//		DEBUG("f=1\n");

		d_lon = to_radians(d_lon / 2);
		d_lat = to_radians(d_lat / 2);

//		DEBUG("#d_lon=%0.10f\n", d_lon);
//		DEBUG("#d_lat=%0.10f\n", d_lat);

		float q = pow(sin(d_lat), 2) + pow(sin(d_lon), 2) * cos(to_radians(lat1 / (float)GPS_COORD_MUL)) * cos(to_radians(lat2 / (float)GPS_COORD_MUL));

//		DEBUG("#q=%0.10f\n", q);

		dist = 2 * FAI_EARTH_RADIUS * asin(sqrt(q)) * 100000.0;
	}
	else //WGS
	{
//		DEBUG("f=0\n");
//		DEBUG("#lat=%0.10f\n", (lat1 + lat2) / ((float)GPS_COORD_MUL * 2));

		float kx, ky;
		gps_get_kx_ky((lat1 + lat2) / ((float)GPS_COORD_MUL * 2), &kx, &ky);

//		DEBUG("#kx=%0.10f\n", kx);
//		DEBUG("#ky=%0.10f\n", ky);

        d_lon *= kx;
        d_lat *= ky;

//		DEBUG("#d_lon=%0.10f\n", d_lon);
//		DEBUG("#d_lat=%0.10f\n", d_lat);

        dist = sqrt(pow(d_lon, 2) + pow(d_lat, 2)) * 100000.0;
	}

    if (bearing)
    {
        if (d_lon  == 0 && d_lat == 0)
			*bearing = 0;
        else
        	*bearing = ((int16_t)to_degrees(atan2(d_lon, d_lat)) + 360) % 360;
//		DEBUG("a=%d\n", *bearing);
    }
//	DEBUG("d=%lu\n\n", dist);

    return dist;
}

void navigation_init()
{
	char full_path[64];

	//go to wait mode
	fc.task.opti_timer = TASK_OPTIMISE_WAIT;
	fc.task.opti_step = 6;

	if (config.tasks.name[0])
	{
		const char * task_root = PSTR("/TASKS");
		sprintf_P(full_path, PSTR("%S/%s"), task_root, config.tasks.name);

		if (storage_file_exist(full_path))
		{
			waypoint_task_open((char *)config.tasks.name);
		}
	}
}

/**
 * The GPS position has changed. Compute the distance to the previous
 * position and update the odometer accordingly.
 */
void navigation_step()
{
	#define NO_LAT_DATA  ((int32_t)2147483647)

	static int32_t last_lat = NO_LAT_DATA;
	static int32_t last_lon;

	if (fc.gps_data.new_sample & FC_GPS_NEW_SAMPLE_ODO)
	{
		fc.gps_data.new_sample &= ~FC_GPS_NEW_SAMPLE_ODO;
	}
	else
	{
		if (waypoint_task_active() && (fc.task.head.flags & CFG_TASK_FLAGS_OPTIMIZE))
			waypoint_task_optimise_step();
		return;
	}

	if (fc.flight.home_valid)
	{
		bool use_fai = config.connectivity.gps_format_flags & GPS_EARTH_MODEL_FAI;

		fc.flight.home_distance = gps_distance(fc.gps_data.latitude, fc.gps_data.longtitude,
											   config.home.lat, config.home.lon,
											   use_fai, (int16_t *)&fc.flight.home_bearing) / 100000.0;   // cm to km
	}

	if (waypoint_task_active())
	{
//		DEBUG("lat1 %ld\n", fc.task.next_waypoint.lat);
//		DEBUG("lon1 %ld\n", fc.task.next_waypoint.lon);
//		DEBUG("lat2 %ld\n", fc.gps_data.latitude);
//		DEBUG("lon2 %ld\n", fc.gps_data.longtitude);

		bool use_fai = fc.task.head.flags & CFG_TASK_FLAGS_FAI_SPHERE;

		int32_t center_dist;
		int32_t wpt_dist;

		//use optimal points if enabled and ready
		bool use_optimal = fc.task.head.flags & CFG_TASK_FLAGS_OPTIMIZE && !(fc.task.next_waypoint.twpt.opti_latitude == 0 && fc.task.next_waypoint.twpt.opti_longtitude == 0);

		//unless the task is set to exit and you are outside and only on first wp
		if (waypoint_task_mode() == TASK_MODE_PREPARE && !fc.task.inside_before_start && !(fc.task.head.flags & CFG_TASK_FLAGS_START_ENTER) && fc.task.waypoint_index == 1)
		{
			use_optimal = false;
		}

		if (use_optimal)
		{
			//get optimal points
			wpt_dist = gps_distance(fc.gps_data.latitude, fc.gps_data.longtitude,
					fc.task.next_waypoint.twpt.opti_latitude, fc.task.next_waypoint.twpt.opti_longtitude,
					use_fai, (int16_t *)&fc.task.next_waypoint.bearing) / 100;    // cm to m

			center_dist = gps_distance(fc.gps_data.latitude, fc.gps_data.longtitude,
					fc.task.next_waypoint.twpt.wpt.latitude, fc.task.next_waypoint.twpt.wpt.longtitude,
					use_fai, NULL) / 100;    // cm to m

			DEBUG("n %lu %lu\n", fc.task.next_waypoint.twpt.opti_latitude, fc.task.next_waypoint.twpt.opti_longtitude);
		}
		else
		{
			//get center points
			wpt_dist = gps_distance(fc.gps_data.latitude, fc.gps_data.longtitude,
					fc.task.next_waypoint.twpt.wpt.latitude, fc.task.next_waypoint.twpt.wpt.longtitude,
					use_fai, (int16_t *)&fc.task.next_waypoint.bearing) / 100;    // cm to m

			center_dist = wpt_dist;

			DEBUG("n %lu %lu\n", fc.task.next_waypoint.twpt.wpt.latitude, fc.task.next_waypoint.twpt.wpt.longtitude);
		}

		center_dist -= fc.task.next_waypoint.twpt.radius_m;

		if (center_dist <= 0 && fc.flight.state == FLIGHT_FLIGHT)
		{

			if (waypoint_task_mode() == TASK_MODE_PREPARE)
			{
				//before start
				fc.task.inside_before_start = true;
			}
			else
			{
				//active or after deadline

				//if task start is enter and you are inside
				if (fc.task.head.flags & CFG_TASK_FLAGS_START_ENTER && fc.task.inside_before_start)
				{
					//need to exit the cylinder first

				}
				else
				{
					if (waypoint_goto_next())
					{
						seq_start(&next_wpt, config.gui.alert_volume);
						waypoint_show();
					}
					else
					{
						fc.task.waypoint_index = fc.task.waypoint_count;
						fc.task.next_waypoint.bearing = 0;
						fc.task.next_waypoint.distance = 0;
						gui_showmessage_P(PSTR("Navigation\nfinished."));

						seq_start(&last_wpt, config.gui.alert_volume);
					}
				}
			}
		}
		else
		{
			//outside wpt
			fc.task.inside_before_start = false;
		}

		fc.task.next_waypoint.distance = wpt_dist / 1000.0;             // m to km.
	}

	// Do we already have a previous GPS point?
	if (last_lat != NO_LAT_DATA)
	{
		bool use_fai = config.connectivity.gps_format_flags & GPS_EARTH_MODEL_FAI;
		uint32_t v = gps_distance(last_lat, last_lon, fc.gps_data.latitude, fc.gps_data.longtitude, use_fai);

		//calculated speed in knots. The distance "v" is in centimeter and we get a GPS sample every second, so this is meter per second.
		float calc_speed = v / 100.0 * FC_MPS_TO_KNOTS;

		//do not add when gps speed is < 1 km/h
		//do not add when difference between calculated speed and gps speed is > 10 km/h
		if (fabs(calc_speed - fc.gps_data.ground_speed) < FC_ODO_MAX_SPEED_DIFF && fc.gps_data.ground_speed > FC_ODO_MIN_SPEED)
			fc.odometer += v;
	}

	// Save the current GPS position for the next step
	last_lat = fc.gps_data.latitude;
	last_lon = fc.gps_data.longtitude;
}
