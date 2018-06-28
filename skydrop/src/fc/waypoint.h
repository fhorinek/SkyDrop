/*
 * wpt.h
 *
 *  Created on: 22.06.2018
 *      Author: bubeck
 */

#ifndef FC_WAYPOINT_H_
#define FC_WAYPOINT_H_

#include "fc.h"

extern char waypoint_filename[];

void set_waypoint_file(const char *filename);
int read_waypoint(int num, volatile waypoint_t *wpt);
int count_waypoints();
bool waypoint_goto_next();
bool waypoint_goto_prev();

#endif /* FC_WAYPOINT_H_ */
