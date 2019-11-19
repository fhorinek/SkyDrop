/*
 * wpt.c
 *
 *  Created on: 22.06.2018
 *      Author: tilmann@bubecks.de
 */

#include "waypoint.h"

//#include "../debug_on.h"

void show_waypoint()
{
	char message[50];

	sprintf_P(message, PSTR("Next WPT %d/%d\n\"%s\""),
			fc.task.waypoint_index + 1, fc.task.waypoint_index,
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
	if (fc.task.waypoint_index < fc.task.waypoint_count)
	{
		fc.task.waypoint_index++;
		waypoint_task_get_wpt(fc.task.waypoint_index, (task_waypoint_t *)&fc.task.next_waypoint.twpt);
		show_waypoint();
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
	if (fc.task.waypoint_index > 0)
	{
		fc.task.waypoint_index--;
		waypoint_task_get_wpt(fc.task.waypoint_index, (task_waypoint_t *)&fc.task.next_waypoint.twpt);
		show_waypoint();
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
	strcpy_P(line, PSTR("/WPT/CACHE"));
	f_mkdir(line);

	//get info about source file
	sprintf_P(line, PSTR("/WPT/%s"), name);
	FILINFO fno;
	f_stat(line, &fno);

	//open cache file first and check if it is up-to-date
	sprintf_P(line, PSTR("/WPT/CACHE/%s"), name);
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

	const char * wpt_root = PSTR("/WPT/CACHE");
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

	const char * wpt_root = PSTR("/WPT/CACHE");
	sprintf_P(line, PSTR("%S/%s"), wpt_root, waypoint_list_name);
	f_open(&handle, line, FA_READ);

	uint32_t index = windex * sizeof(waypoint_cache_t);
	f_lseek(&handle, index);
	f_read(&handle, wpt, sizeof(waypoint_cache_t), &br);
	f_close(&handle);
}

void waypoint_task_open_handle(FIL * handle, uint8_t mode)
{
	char line[32];
	const char * task_root = PSTR("/TASKS");
	sprintf_P(line, PSTR("%S/%s"), task_root, fc.task.name);

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

void waypoint_task_open(char * name)
{
	char line[32];
	FIL handle;

	const char * task_root = PSTR("/TASKS");

	//create cache directory
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

		f_write(&handle, (void *)&fc.task.head, sizeof(task_header_t), &bw_br);
	}
	else
	{
		f_read(&handle, (void *)&fc.task.head, sizeof(task_header_t), &bw_br);
	}

	fc.task.waypoint_index = 0;
	fc.task.waypoint_count = (f_size(&handle) - sizeof(task_header_t)) / sizeof(task_waypoint_t);
	f_close(&handle);

	fc.task.active = true;
}

void waypoint_task_modify_head(task_header_t * head)
{
	FIL handle;
	uint16_t bw;

	waypoint_task_open_handle(&handle, FA_WRITE);
	f_lseek(&handle, 0);
	f_write(&handle, head, sizeof(task_header_t), &bw);
	f_close(&handle);
}

void waypoint_task_get_wpt(uint8_t windex, task_waypoint_t * twpt)
{
	FIL handle;
	uint16_t br;

	waypoint_task_open_handle(&handle, FA_READ);

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
	twpt.dist_cm = 0;
	twpt.radius_m = 500;

	waypoint_task_open_handle(&handle, FA_WRITE);
	f_lseek(&handle, f_size(&handle));
	f_write(&handle, &twpt, sizeof(twpt), &bw);
	f_close(&handle);

	fc.task.waypoint_count += 1;
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

void waypoint_task_modify_wpt(uint8_t windex, task_waypoint_t * twpt)
{
	FIL handle;
	uint16_t bw;

	waypoint_task_open_handle(&handle, FA_WRITE);
	uint32_t index = sizeof(task_header_t) + windex * sizeof(task_waypoint_t);
	f_lseek(&handle, index);
	f_write(&handle, twpt, sizeof(task_waypoint_t), &bw);
	f_close(&handle);
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

void waypoint_task_calc()
{
	//sanity check
	if (fc.task.waypoint_count == 0)
		return;

	task_waypoint_t twpt;

	waypoint_task_get_wpt(0, &twpt);

	int32_t lat, lon;
	lat = twpt.wpt.latitude;
	lon = twpt.wpt.longtitude;

	for (uint8_t i = 1; i < fc.task.waypoint_count; i++)
	{
		waypoint_task_get_wpt(i, &twpt);
		twpt.dist_cm = gps_distance_2d(lat, lon, twpt.wpt.latitude, twpt.wpt.longtitude);

		lat = twpt.wpt.latitude;
		lon = twpt.wpt.longtitude;

		waypoint_task_modify_wpt(i, &twpt);
	}
}

void waypoint_task_optimise()
{
	//sanity check
	if (fc.task.waypoint_count == 0)
		return;
}

bool waypoint_task_active()
{
	return fc.task.waypoint_index < fc.task.waypoint_count - 1 && fc.task.active;
}
