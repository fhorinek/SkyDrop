/*
 * wpt.h
 *
 *  Created on: 22.06.2018
 *      Author: bubeck
 */

#ifndef FC_WAYPOINT_H_
#define FC_WAYPOINT_H_

#include "fc.h"

extern char waypoint_list_name[13];
extern uint16_t waypoint_list_count;

struct waypoint_cache_file_t
{
	uint32_t size;
	uint16_t date;
	uint16_t time;
	uint16_t point_count;
};

#define TASK_MODE_NOT_ACTIVE	0
#define TASK_MODE_PREPARE		1
#define TASK_MODE_ACTIVE		2
#define TASK_MODE_ENDED			3

//use waypoints from UI
void waypoint_show();
bool waypoint_goto_next();
bool waypoint_goto_prev();

uint32_t waypoint_task_time_to_start();
uint32_t waypoint_task_time_to_deadline();
uint8_t waypoint_task_mode();


//work with wpt list list
void waypoint_list_cache(char * name);
void waypoint_list_open(char * name);
void waypoint_list_get_wpt(uint8_t windex, waypoint_cache_t * wpt);
void waypoint_list_close();

//work with task
bool waypoint_task_active();
bool waypoint_task_optimal();

void waypoint_task_open(char * name);
void waypoint_task_close();

void waypoint_task_modify_head(task_header_t * head, bool use_temp = false);
void waypoint_task_get_wpt(uint8_t windex, task_waypoint_t * twpt, bool use_tmp = false);
void waypoint_task_modify_wpt(uint8_t windex, task_waypoint_t * twpt, bool use_tmp = false);

void waypoint_task_add_wpt(waypoint_cache_t * wpt);
void waypoint_task_switch_wpt(uint8_t windex_a, uint8_t windex_b);
void waypoint_task_remove_wpt(uint8_t windex);

void waypoint_task_remove();
void waypoint_task_calc_distance();

void waypoint_task_optimise_reset();
void waypoint_task_optimise_step();
#endif /* FC_WAYPOINT_H_ */
