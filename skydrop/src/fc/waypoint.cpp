/*
 * wpt.c
 *
 *  Created on: 22.06.2018
 *      Author: tilmann@bubecks.de
 */

#include "waypoint.h"

#include "../debug_on.h"

void waypoint_show()
{
	char message[50];

	sprintf_P(message, PSTR("Next WPT %d/%d\n\"%s\""),
			fc.task.waypoint_index, fc.task.waypoint_count - 1,
			fc.task.next_waypoint.twpt.wpt.name);
	gui_showmessage(message);
}

/**
 * Go to the next waypoint in the list of waypoints.
 *
 * @return true if next waypoint is available, false otherwise
 */
bool waypoint_goto_next()
{
	if (fc.task.waypoint_index < fc.task.waypoint_count - 1)
	{
		fc.task.waypoint_index++;
		waypoint_task_get_wpt(fc.task.waypoint_index, (task_waypoint_t *)&fc.task.next_waypoint.twpt);
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * Go to the previous waypoint in the list of waypoints.
 *
 * @return true if previous waypoint is available, false otherwise
 */
bool waypoint_goto_prev()
{
	if (fc.task.waypoint_index > 1)
	{
		fc.task.waypoint_index--;
		waypoint_task_get_wpt(fc.task.waypoint_index, (task_waypoint_t *)&fc.task.next_waypoint.twpt);
		return true;
	}
	else
	{
		return false;
	}
}

void waypoint_list_cache(char * name)
{
	FIL source_file;
	FIL cache_file;
	waypoint_cache_file_t info;

	char line[100];
	uint16_t br_bw;

	//create cache directory
	strcpy_P(line, PSTR("/WPT/_CACHE"));
	f_mkdir(line);

	//get info about source file
	sprintf_P(line, PSTR("/WPT/%s"), name);
	FILINFO fno;
	f_stat(line, &fno);

	//open cache file first and check if it is up-to-date
	sprintf_P(line, PSTR("/WPT/_CACHE/%s"), name);
	if (f_open(&cache_file, line, FA_READ) == FR_OK)
	{
		uint32_t index = f_size(&cache_file) - sizeof(info);
		f_lseek(&cache_file, index);
		f_read(&cache_file, &info, sizeof(info), &br_bw);
		f_close(&cache_file);

		bool valid = true;

		if (fno.fdate != info.date)
			valid = false;
		if (fno.ftime != info.time)
			valid = false;
		if (fno.fsize != info.size)
			valid = false;
		if (info.point_count != (f_size(&cache_file) - sizeof(info)) / sizeof(waypoint_cache_t))
			valid = false;

		//if cache file is valid, skip re-generation
		if (valid)
		{
			DEBUG("Cache is up-to-date\n");
			return;
		}
	}
	//open cache
	f_open(&cache_file, line, FA_WRITE | FA_CREATE_ALWAYS);

	//open source
	sprintf_P(line, PSTR("/WPT/%s"), name);
	f_open(&source_file, line, FA_READ);

	bool skip_first = true;
	uint16_t number_of_waypoints = 0;

	while(true)
	{
		waypoint_cache_t wpt;
		char * ptr_start;
		char * ptr_end;
		uint8_t len;

		uint32_t loc_deg;
		uint32_t loc_min;

		if (f_gets(line, sizeof(line), &source_file) == NULL)
			break;

		DEBUG("index: %u\n", number_of_waypoints);
		DEBUG("line: %s", line);

		if (skip_first)
		{
			skip_first = false;
			continue;
		}

		//-----Related Tasks----- start
		if (line[0] == '-')
			break;

		//read WPT info
		//name
		ptr_start = line + 1;
		ptr_end = index(ptr_start, '"');
		len = min(sizeof(wpt.name), (uint8_t)(ptr_end - ptr_start));
		memset(wpt.name, 0, sizeof(wpt.name));
		memcpy(wpt.name, ptr_start, len);

		//code
		ptr_start = find_comma(ptr_end) + 1;
		ptr_end = index(ptr_start, '"');
		len = min(sizeof(wpt.code), (uint8_t)(ptr_end - ptr_start));
		memset(wpt.code, 0, sizeof(wpt.code));
		memcpy(wpt.code, line + 1, len);

		ptr_start = find_comma(ptr_start);
		ptr_start = find_comma(ptr_start);

		//Latitude
		loc_deg = atoi_n(ptr_start, 2);
		loc_min = atoi_n(ptr_start + 2, 5);

		wpt.latitude = (loc_min * 1000ul) / 6;
		wpt.latitude = loc_deg * 10000000ul + wpt.latitude;
		if ((*(ptr_start + 8)) == 'S')
			wpt.latitude *= -1;

		//Longitude
		ptr_start = find_comma(ptr_start);
		loc_deg = atoi_n(ptr_start, 3);
		loc_min = atoi_n(ptr_start + 3, 5);

		wpt.longtitude = (loc_min * 1000ul) / 6;
		wpt.longtitude = loc_deg * 10000000ul + wpt.longtitude;
		if ((*(ptr_start + 9)) == 'W')
			wpt.longtitude *= -1;

		//Elevation
		ptr_start = find_comma(ptr_start);

		if (*(find_comma(ptr_start) - 2) != 'm')
			wpt.elevation = atoi_f(ptr_start) * FC_METER_TO_FEET;
		else
			wpt.elevation = atoi_f(ptr_start);

		//Waypoint style
		ptr_start = find_comma(ptr_start);
		if ((*ptr_start) != ',' && (*ptr_start) != '0' && (*ptr_start) != '1')
			continue;

//		DEBUG("name '%s'\n", wpt.name);
//		DEBUG("code '%s'\n", wpt.code);
//		DEBUG("lat  %ld\n", wpt.latitude);
//		DEBUG("long %ld\n", wpt.longtitude);
//		DEBUG("ele  %d\n\n", wpt.elevation);

		f_write(&cache_file, &wpt, sizeof(wpt), &br_bw);

		number_of_waypoints++;
	}

	f_close(&source_file);

	info.size = fno.fsize;
	info.date = fno.fdate;
	info.time = fno.ftime;
	info.point_count = number_of_waypoints;

//	DEBUG("size: %lu\n", info.size);
//	DEBUG("date: %u\n", info.date);
//	DEBUG("time: %u\n", info.time);
//	DEBUG("point_count: %u\n", info.point_count);

	f_write(&cache_file, &info, sizeof(info), &br_bw);
	f_close(&cache_file);
}

char waypoint_list_name[13];
uint16_t waypoint_list_count;

void waypoint_list_open(char * name)
{
	char line[32];
	FIL handle;

	strcpy(waypoint_list_name, name);

	waypoint_list_cache(name);

	const char * wpt_root = PSTR("/WPT/_CACHE");
	sprintf_P(line, PSTR("%S/%s"), wpt_root, waypoint_list_name);

	f_open(&handle, line, FA_READ);
	waypoint_list_count = (f_size(&handle) - sizeof(waypoint_cache_file_t)) / sizeof(waypoint_cache_t);
	f_close(&handle);

//	DEBUG("waypoint_list_open %s\n", line);
//	DEBUG("waypoint_list_count %u\n", waypoint_list_count);
}

void waypoint_list_close()
{
	waypoint_list_name[0] = 0;
}

void waypoint_list_get_wpt(uint8_t windex, waypoint_cache_t * wpt)
{
	FIL handle;
	char line[32];
	uint16_t br;

	const char * wpt_root = PSTR("/WPT/_CACHE");
	sprintf_P(line, PSTR("%S/%s"), wpt_root, waypoint_list_name);
	f_open(&handle, line, FA_READ);

	uint32_t index = windex * sizeof(waypoint_cache_t);
	f_lseek(&handle, index);
	f_read(&handle, wpt, sizeof(waypoint_cache_t), &br);
	f_close(&handle);
}

void waypoint_task_open_handle(FIL * handle, uint8_t mode, bool use_tmp = false)
{
	char line[32];
	const char * task_root = PSTR("/TASKS");
	if (!use_tmp)
		sprintf_P(line, PSTR("%S/%s"), task_root, fc.task.name);
	else
		sprintf_P(line, PSTR("%S/_TMP"), task_root);

	if (handle == NULL)
		f_unlink(line);
	else
		f_open(handle, line, mode);
}

void waypoint_task_remove()
{
	waypoint_task_open_handle(NULL, NULL);
	fc.task.active = false;

	fc.task.waypoint_count -= 1;
}

void waypoint_task_close()
{
	fc.task.active = false;
	config.tasks.name[0] = 0;

	eeprom_busy_wait();
	eeprom_update_block((void *)config.tasks.name, config_ee.tasks.name, 1);
}

void waypoint_task_open(char * name)
{
	char line[32];
	FIL handle;

	const char * task_root = PSTR("/TASKS");

	//create task directory
	strcpy_P(line, task_root);
	f_mkdir(line);

	if (strlen(name) == 0)
	{
		uint16_t n = 0;
		FILINFO fno;

		do
		{
			sprintf_P(line, PSTR("%S/TASK%03u"), task_root, ++n);
		} while(f_stat(line, &fno) != FR_NO_FILE);

		sprintf_P((char *)fc.task.name, PSTR("TASK%03u"), n);
	}
	else
	{
		strcpy((char *)fc.task.name, name);
	}

	//store name to EE
	strcpy((char *)config.tasks.name, (char *)fc.task.name);
	eeprom_busy_wait();
	eeprom_update_block((void *)config.tasks.name, config_ee.tasks.name, sizeof(config_ee.tasks.name));

	waypoint_task_open_handle(&handle, FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
	f_lseek(&handle, 0);

	uint16_t bw_br;

	if (f_size(&handle) == 0)
	{
		//default header data
		fc.task.head.flags = CFG_TASK_FLAGS_START_EXIT;

		fc.task.head.start_hour = CFG_TASK_HOUR_DISABLED;
		fc.task.head.start_min = 0;

		fc.task.head.deadline_hour = CFG_TASK_HOUR_DISABLED;
		fc.task.head.deadline_min = 0;

		fc.task.head.center_dist_m = 0;
		fc.task.head.opti_dist_m = 0;
		fc.task.head.optimised = false;

		f_write(&handle, (void *)&fc.task.head, sizeof(task_header_t), &bw_br);
	}
	else
	{
		f_read(&handle, (void *)&fc.task.head, sizeof(task_header_t), &bw_br);
	}

	fc.task.waypoint_index = 1;
	fc.task.waypoint_count = (f_size(&handle) - sizeof(task_header_t)) / sizeof(task_waypoint_t);
	f_close(&handle);

	if (fc.task.waypoint_count > 1)
		waypoint_task_get_wpt(fc.task.waypoint_index, (task_waypoint_t *)&fc.task.next_waypoint.twpt);

	fc.task.active = true;
}

void waypoint_task_modify_head(task_header_t * head, bool use_temp)
{
	FIL handle;
	uint16_t bw;

	waypoint_task_open_handle(&handle, FA_WRITE | FA_OPEN_ALWAYS, use_temp);
	f_lseek(&handle, 0);
	f_write(&handle, head, sizeof(task_header_t), &bw);
	f_close(&handle);
}

void waypoint_task_get_wpt(uint8_t windex, task_waypoint_t * twpt, bool use_tmp)
{
	FIL handle;
	uint16_t br;

	waypoint_task_open_handle(&handle, FA_READ, use_tmp);

	uint32_t index = sizeof(task_header_t) + windex * sizeof(task_waypoint_t);
	f_lseek(&handle, index);
	f_read(&handle, twpt, sizeof(task_waypoint_t), &br);
	f_close(&handle);
}

void waypoint_task_add_wpt(waypoint_cache_t * wpt)
{
	FIL handle;
	uint16_t bw;

	task_waypoint_t twpt;
	memcpy(&twpt.wpt, wpt, sizeof(waypoint_cache_t));
	twpt.dist_m = 0;
	twpt.radius_m = TASK_WAYPOINT_DEFAULT_RADIUS;

	waypoint_task_open_handle(&handle, FA_WRITE);
	f_lseek(&handle, f_size(&handle));
	f_write(&handle, &twpt, sizeof(twpt), &bw);
	f_close(&handle);

	fc.task.waypoint_count += 1;
	fc.task.head.optimised = false;
}

void waypoint_task_switch_wpt(uint8_t windex_a, uint8_t windex_b)
{
	task_waypoint_t twpt_a, twpt_b;

	//sanity check
	if (windex_a >= fc.task.waypoint_count || windex_b >= fc.task.waypoint_count || windex_a == windex_b)
		return;

	waypoint_task_get_wpt(windex_a, &twpt_a);
	waypoint_task_get_wpt(windex_b, &twpt_b);

	waypoint_task_modify_wpt(windex_a, &twpt_b);
	waypoint_task_modify_wpt(windex_b, &twpt_a);
}

void waypoint_task_modify_wpt(uint8_t windex, task_waypoint_t * twpt, bool use_tmp)
{
	FIL handle;
	uint16_t bw;

	waypoint_task_open_handle(&handle, FA_WRITE, use_tmp);
	uint32_t index = sizeof(task_header_t) + windex * sizeof(task_waypoint_t);
	f_lseek(&handle, index);
	f_write(&handle, twpt, sizeof(task_waypoint_t), &bw);
	f_close(&handle);

	fc.task.head.optimised = false;
}

void waypoint_task_remove_wpt(uint8_t windex)
{
	task_waypoint_t twpt;

	//sanity check
	if (windex >= fc.task.waypoint_count)
		return;

	for (uint8_t i = windex; i < fc.task.waypoint_count - 1; i++)
	{
		waypoint_task_get_wpt(i + 1, &twpt);
		waypoint_task_modify_wpt(i, &twpt);
	}

	FIL handle;

	fc.task.waypoint_count -= 1;

	waypoint_task_open_handle(&handle, FA_WRITE);
	uint32_t index = sizeof(task_header_t) + fc.task.waypoint_count * sizeof(task_waypoint_t);
	f_lseek(&handle, index);
	f_truncate(&handle);
	f_close(&handle);

}

void waypoint_task_calc_distance()
{
	//sanity check
	if (fc.task.waypoint_count == 0)
		return;

	task_waypoint_t twpt;

	waypoint_task_get_wpt(0, &twpt);

	int32_t lat, lon;
	lat = twpt.wpt.latitude;
	lon = twpt.wpt.longtitude;

	fc.task.head.center_dist_m = 0;
	fc.task.head.opti_dist_m = 0;
	fc.task.head.optimised = false;

	for (uint8_t i = 1; i < fc.task.waypoint_count; i++)
	{
		waypoint_task_get_wpt(i, &twpt);

		bool use_fai = fc.task.head.flags & CFG_TASK_FLAGS_FAI_SPHERE;
		twpt.dist_m = gps_distance(lat, lon, twpt.wpt.latitude, twpt.wpt.longtitude, use_fai);
		fc.task.head.center_dist_m += twpt.dist_m;

		lat = twpt.wpt.latitude;
		lon = twpt.wpt.longtitude;

		waypoint_task_modify_wpt(i, &twpt);
	}

	waypoint_task_modify_head((task_header_t *)&fc.task.head);
}

void waypoint_task_opti_angle(opti_waypoint_cache_t * twpt, float angle)
{
	float lon1 = twpt->longtitude / (float)GPS_COORD_MUL;
	float lat1 = twpt->latitude / (float)GPS_COORD_MUL;

	float lon2, lat2;

	gps_destination(lat1, lon1, angle, twpt->radius_m / 1000.0, &lat2, &lon2);

	twpt->opti_longtitude = lon2 * (float)GPS_COORD_MUL;
	twpt->opti_latitude = lat2 * (float)GPS_COORD_MUL;
}

uint32_t waypoint_task_opti_distance(opti_waypoint_cache_t * twpt1, opti_waypoint_cache_t * twpt2)
{
	int32_t lat1 = twpt1->opti_latitude;
	int32_t lon1 = twpt1->opti_longtitude;

	int32_t lat2 = twpt2->opti_latitude;
	int32_t lon2 = twpt2->opti_longtitude;

	bool use_fai = fc.task.head.flags & CFG_TASK_FLAGS_FAI_SPHERE;
	return gps_distance(lat1, lon1, lat2, lon2, use_fai, NULL);
}

#define TASK_OPTI_INVALID	0xFF

void waypoint_task_optimise_init()
{
	//create temporary copy of the actual task
	waypoint_task_modify_head((task_header_t*)&fc.task.head, true);

	//duplicate the waypoints
	for (uint8_t i = 0; i < fc.task.waypoint_count; i++)
	{
		task_waypoint_t actual_wpt;
		waypoint_task_get_wpt(i, &actual_wpt);

		actual_wpt.opti_latitude = actual_wpt.wpt.latitude;
		actual_wpt.opti_longtitude = actual_wpt.wpt.longtitude;
		actual_wpt.opti_angle = 500;
		actual_wpt.opti_dist_m = 0xFFFFFFFF;

		DEBUG("c %u %ld %ld %u\n", i, actual_wpt.wpt.latitude, actual_wpt.wpt.longtitude, actual_wpt.radius_m);

		waypoint_task_modify_wpt(i, &actual_wpt, true);
	}

	//reset cache
	for (uint8_t i = 0; i < TASK_OPTI_CACHE_SIZE; i++)
		fc.task.opti_cache[i].index = TASK_OPTI_INVALID;

	fc.task.opti_wp_index = 1;
	fc.task.opti_step = 1;
	fc.task.opti_sweep_step = 0;
	fc.task.opti_itter_improved = false;
}

//return true if need next wpt
bool waypoint_task_optimise_sweep(opti_waypoint_cache_t * prev_wpt, opti_waypoint_cache_t * actual_wpt, opti_waypoint_cache_t * next_wpt)
{
	float start, end, step;
	uint8_t opti_steps;

	switch (fc.task.opti_step)
	{
		case(1):

			start = 0;
			end = 360;
			step = 20;
			opti_steps = 18;
			break;
		case(2):
			start = actual_wpt->opti_angle - 10;
			end = actual_wpt->opti_angle + 10;
			step = 1;
			opti_steps = 10;
			break;
		case(3):
			start = actual_wpt->opti_angle - 0.5;
			end = actual_wpt->opti_angle + 0.5;
			step = 0.1;
			opti_steps = 10;
			break;
		case(4):
		default:
			start = actual_wpt->opti_angle - 0.05;
			end = actual_wpt->opti_angle + 0.05;
			step = 0.01;
			opti_steps = 10;
			break;
	}

	uint32_t min_dist;

	min_dist = actual_wpt->opti_dist_m;
	if (fc.task.opti_step == 1)
		min_dist &= 0x7FFFFFFF;

	if (fc.task.opti_step == 2 && actual_wpt->opti_dist_m & 0x80000000)
		min_dist = 0xFFFFFFFF;


	float min_angle = actual_wpt->opti_angle;

	DEBUG("--i min %lu\n", min_dist);

	float sweep_delta = (end - start) / opti_steps;
	start = start + sweep_delta * fc.task.opti_sweep_step;
	end = start + sweep_delta;

	uint32_t first = 0;

	for (float angle = start; angle < end; angle += step)
	{
		DEBUG("--a %u %u %0.2f\n", fc.task.opti_step, actual_wpt->index, angle);

		//recalculate opti lan/lot based on angle
		waypoint_task_opti_angle(actual_wpt, angle);

		first = waypoint_task_opti_distance(prev_wpt, actual_wpt);
		uint32_t dist = first;

		if (fc.task.opti_step > 1)
			dist += waypoint_task_opti_distance(actual_wpt, next_wpt);

		if (min_dist > dist)
		{
			min_dist = dist;
			min_angle = angle;

			DEBUG("w %u %ld %ld %0.2f %lu\n", actual_wpt->index, actual_wpt->opti_latitude, actual_wpt->opti_longtitude, angle, dist);
		}
	}

	//if change between last and best angle is larger than step
	bool improved = (abs(actual_wpt->opti_angle - min_angle) > 0);
	//bool improved = (abs(actual_wpt->opti_angle - min_angle) > step);

	//set best opti angle
	if (improved)
	{
		actual_wpt->opti_angle = min_angle;
		actual_wpt->opti_dist_m = min_dist | ((fc.task.opti_step == 1) ? 0x80000000 : 0);

		fc.task.opti_itter_improved = true;
		DEBUG("--IMPROVED\n");
	}

	//calculate opti lat/lon
	waypoint_task_opti_angle(actual_wpt, actual_wpt->opti_angle);

	//increment the sweep step
	fc.task.opti_sweep_step = (fc.task.opti_sweep_step + 1) % opti_steps;

	//next iteration starts?
	return (fc.task.opti_sweep_step == 0);
}

void waypoint_task_optimise_store_cache_wpt(uint8_t cache_index)
{
	//store old data if used
	if (fc.task.opti_cache[cache_index].index != TASK_OPTI_INVALID)
	{
		task_waypoint_t tmp;

		//load original wpt
		waypoint_task_get_wpt(fc.task.opti_cache[cache_index].index, &tmp, true);

		tmp.opti_angle = fc.task.opti_cache[cache_index].opti_angle;
		tmp.opti_latitude = fc.task.opti_cache[cache_index].opti_latitude;
		tmp.opti_longtitude = fc.task.opti_cache[cache_index].opti_longtitude;
		tmp.opti_dist_m = fc.task.opti_cache[cache_index].opti_dist_m;

//		DEBUG("Storing to SD %u\n", fc.task.opti_cache[cache_index].index);
//		DEBUG("opti_angle %0.2f\n", tmp.opti_angle);
//		DEBUG("opti_latitude %ld\n", tmp.opti_latitude);
//		DEBUG("opti_longtitude %ld\n", tmp.opti_longtitude);
//		DEBUG("opti_dist_m %lu\n", tmp.opti_dist_m);
//		DEBUG("\n");

		waypoint_task_modify_wpt(fc.task.opti_cache[cache_index].index, &tmp, true);
	}

	fc.task.opti_cache[cache_index].index = TASK_OPTI_INVALID;
}

//return true if found in cache
bool waypoint_task_optimise_get_cache_wpt(uint8_t index, opti_waypoint_cache_t * * wpt)
{
	for (uint8_t i = 0; i < TASK_OPTI_CACHE_SIZE; i++)
	{
		if (index == fc.task.opti_cache[i].index)
		{
			*wpt = (opti_waypoint_cache_t *)&fc.task.opti_cache[i];

			return true;
		}
	}

	//not found in cache load new
	uint8_t new_index = index % TASK_OPTI_CACHE_SIZE;

	//store old data if used
	waypoint_task_optimise_store_cache_wpt(new_index);

	task_waypoint_t tmp;

	//load new data
	waypoint_task_get_wpt(index, &tmp, true);

	fc.task.opti_cache[new_index].index = index;
	fc.task.opti_cache[new_index].latitude = tmp.wpt.latitude;
	fc.task.opti_cache[new_index].longtitude = tmp.wpt.longtitude;
	fc.task.opti_cache[new_index].radius_m = tmp.radius_m;

	fc.task.opti_cache[new_index].opti_angle = tmp.opti_angle;
	fc.task.opti_cache[new_index].opti_latitude = tmp.opti_latitude;
	fc.task.opti_cache[new_index].opti_longtitude = tmp.opti_longtitude;
	fc.task.opti_cache[new_index].opti_dist_m = tmp.opti_dist_m;

	DEBUG("Loading from SD %u\n", index);
//	DEBUG("opti_angle %0.2f\n", tmp.opti_angle);
//	DEBUG("opti_latitude %ld\n", tmp.opti_latitude);
//	DEBUG("opti_longtitude %ld\n", tmp.opti_longtitude);
	DEBUG("opti_dist_m %lu\n", tmp.opti_dist_m);
//	DEBUG("\n");

	*wpt = (opti_waypoint_cache_t *)&fc.task.opti_cache[new_index];

	return false;
}

void waypoint_task_optimise_itter()
{
	opti_waypoint_cache_t * prev_wpt, * actual_wpt, * next_wpt;

	//get waypoint cache, if not in cache load from sd and came back to next iteration
	//this way it will not block the user interface
	if (!waypoint_task_optimise_get_cache_wpt(fc.task.opti_wp_index - 1, &prev_wpt))
		return;
	if (!waypoint_task_optimise_get_cache_wpt(fc.task.opti_wp_index + 0, &actual_wpt))
		return;
	if (!waypoint_task_optimise_get_cache_wpt(fc.task.opti_wp_index + 1, &next_wpt))
		return;

	//get most optimal angle, return true if need another wpt
	if (waypoint_task_optimise_sweep(prev_wpt, actual_wpt, next_wpt))
	{
		fc.task.opti_wp_index++;
	}

	if (fc.task.opti_wp_index > fc.task.waypoint_count - 2)
	{
		if (!fc.task.opti_itter_improved)
		{
			fc.task.opti_step++;
		}

		fc.task.opti_itter_improved = false;
		fc.task.opti_wp_index = 1;

	}

	//DEBUG("----wp %u time: %lu\n", fc.task.opti_wp_index, task_get_ms_tick_once() - itter_time);
}

void waypoint_task_optimise_finalise()
{
	char pfrom[32];
	char pto[32];

	DEBUG("--waypoint_task_optimise_finalise()\n");

	//save the cache to SD
	for (uint8_t i = 0; i < TASK_OPTI_CACHE_SIZE; i++)
		waypoint_task_optimise_store_cache_wpt(i);

	//replace the original task
	const char * task_root = PSTR("/TASKS");
	sprintf_P(pfrom, PSTR("%S/_TMP"), task_root);
	sprintf_P(pto, PSTR("%S/%s"), task_root, fc.task.name);

	f_unlink(pto);
	uint8_t res = f_rename(pfrom, pto);
	DEBUG("res = %u\n", res);

	//get optimal distance
	task_waypoint_t twpt;

	waypoint_task_get_wpt(0, &twpt);
	fc.task.head.opti_dist_m = -(int32_t)twpt.radius_m;
	int32_t lon1 = twpt.opti_longtitude;
	int32_t lat1 = twpt.opti_latitude;

	DEBUG("fc.task.head.opti_dist_m %ld\n", fc.task.head.opti_dist_m);

	for (uint8_t i = 1; i < fc.task.waypoint_count; i++)
	{
		waypoint_task_get_wpt(i, &twpt);

		int32_t lon2 = twpt.opti_longtitude;
		int32_t lat2 = twpt.opti_latitude;

		bool use_fai = fc.task.head.flags & CFG_TASK_FLAGS_FAI_SPHERE;
		twpt.opti_dist_m =  gps_distance(lat1, lon1, lat2, lon2, use_fai, NULL);

		lon1 = lon2;
		lat1 = lat2;

//		DEBUG("Loading from SD %u\n", i);
//		DEBUG("opti_angle %0.2f\n", twpt.opti_angle);
//		DEBUG("opti_latitude %ld\n", twpt.opti_latitude);
//		DEBUG("opti_longtitude %ld\n", twpt.opti_longtitude);
		DEBUG("dist_m %lu\n", twpt.dist_m);
		DEBUG("opti_dist_m %lu\n", twpt.opti_dist_m);
//		DEBUG("\n");

		fc.task.head.opti_dist_m += twpt.opti_dist_m;

		DEBUG("fc.task.head.opti_dist_m %ld\n", fc.task.head.opti_dist_m);

		waypoint_task_modify_wpt(i, &twpt);
	}

	//remove radius if goal is line
	if ((fc.task.head.flags & CFG_TASK_FLAGS_GOAL_LINE) == 0)
		fc.task.head.opti_dist_m -= twpt.radius_m;

	DEBUG("fc.task.head.opti_dist_m %ld\n", fc.task.head.opti_dist_m);

	fc.task.head.optimised = true;
	waypoint_task_modify_head((task_header_t *)&fc.task.head);

	//reload actual wpt
	waypoint_task_get_wpt(fc.task.waypoint_index, (task_waypoint_t *)&fc.task.next_waypoint.twpt);

	//go to wait mode
	fc.task.opti_step = 6;
}

void waypoint_task_optimise_now()
{
	DEBUG("--waypoint_task_optimise_now()\n");
	//sanity check
	if (fc.task.waypoint_count < 3)
		return;

	uint32_t start_time = task_get_ms_tick_once();

	fc.task.opti_step = 0;
	while (fc.task.opti_step < 6)
	{
		waypoint_task_optimise_step();
		ewdt_reset();
	}

	DEBUG("--total duration %lu\n\n", task_get_ms_tick_once() - start_time);

	for (uint8_t i = 0; i < fc.task.waypoint_count; i++)
	{
		task_waypoint_t actual_wpt;
		waypoint_task_get_wpt(i, &actual_wpt);

		DEBUG("w %u %ld %ld %0.2f\n", i, actual_wpt.opti_latitude, actual_wpt.opti_longtitude, actual_wpt.opti_angle);
	}
}

void waypoint_task_optimise_step()
{
	switch(fc.task.opti_step)
	{
		case(0):
			waypoint_task_optimise_init();
		break;
		case(1):
		case(2):
		case(3):
		case(4):
			waypoint_task_optimise_itter();
		break;
		case(5):
			waypoint_task_optimise_finalise();
		break;
		case(6):
			//wait
		break;

	}
}

bool waypoint_task_active()
{
	return fc.task.waypoint_index < fc.task.waypoint_count && fc.task.active;
}

uint32_t waypoint_task_time_to_start()
{
	return (fc.task.head.start_hour * 3600 + fc.task.head.start_min * 60) - (time_get_local() % 86400ul);
}

uint32_t waypoint_task_time_to_deadline()
{
	return (fc.task.head.deadline_hour * 3600 + fc.task.head.deadline_hour * 60) - (time_get_local() % 86400ul);
}

uint8_t waypoint_task_mode()
{
	if (waypoint_task_active())
	{
		uint32_t actual_epoch = time_get_local() % 86400ul;
		uint32_t start_epoch = (fc.task.head.start_hour != CFG_TASK_HOUR_DISABLED) ? (fc.task.head.start_hour * 3600 + fc.task.head.start_min * 60) : 0;
		uint32_t deadline_epoch = (fc.task.head.deadline_hour != CFG_TASK_HOUR_DISABLED) ? (fc.task.head.deadline_hour * 3600 + fc.task.head.deadline_min * 60) : 86400ul;

		if (start_epoch < actual_epoch)
			return TASK_MODE_PREPARE;
		if (actual_epoch <= deadline_epoch)
			return TASK_MODE_ACTIVE;

		return TASK_MODE_ENDED;
	}

	return TASK_MODE_NOT_ACTIVE;
}
