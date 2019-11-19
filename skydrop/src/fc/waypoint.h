/*
 * wpt.h
 *
 *  Created on: 22.06.2018
 *      Author: bubeck
 */

#ifndef FC_WAYPOINT_H_
#define FC_WAYPOINT_H_

#include "fc.h"


void set_waypoint_file(const char *filename);
int read_waypoint(int num, volatile waypoint_t *wpt);
int count_waypoints();
bool waypoint_goto_next();
bool waypoint_goto_prev();

extern char waypoint_list_name[13];
extern uint16_t waypoint_list_count;

struct waypoint_cache_file_t
{
	uint32_t size;
	uint16_t date;
	uint16_t time;
	uint16_t point_count;
};


void waypoint_list_cache(char * name);
void waypoint_list_open(char * name);
void waypoint_list_get_wpt(uint8_t windex, waypoint_cache_t * wpt);
void waypoint_list_close();

bool waypoint_task_active();
void waypoint_task_open(char * name);
void waypoint_task_modify_head(task_header_t * head);
void waypoint_task_get_wpt(uint8_t windex, task_waypoint_t * twpt);
void waypoint_task_add_wpt(waypoint_cache_t * wpt);
void waypoint_task_switch_wpt(uint8_t windex_a, uint8_t windex_b);
void waypoint_task_modify_wpt(uint8_t windex, task_waypoint_t * twpt);
void waypoint_task_remove_wpt(uint8_t windex);
void waypoint_task_remove();
void waypoint_task_calc();

#endif /* FC_WAYPOINT_H_ */
