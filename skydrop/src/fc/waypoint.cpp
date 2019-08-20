/*
 * wpt.c
 *
 *  Created on: 22.06.2018
 *      Author: tilmann@bubecks.de
 */

#include "waypoint.h"

//#include "../debug_on.h"

char waypoint_filename[24];        // The filename of the current waypoint file

void show_waypoint()
{
	char message[50];

	sprintf_P(message, PSTR("Next WPT %d/%d\n\"%s\""),
			fc.flight.waypoint_no, fc.flight.waypoints_count,
			fc.flight.next_waypoint.name);
	gui_showmessage(message);
}

void set_waypoint_file(const char *filename)
{
	if (strlen(filename) < sizeof(waypoint_filename) - 1)
	{
		strcpy(waypoint_filename, filename);

		fc.flight.waypoints_count = count_waypoints();
		if (fc.flight.waypoints_count > 0)
		{
			fc.flight.waypoint_no = 1;
			read_waypoint(fc.flight.waypoint_no, &(fc.flight.next_waypoint));
			show_waypoint();
		}
		else
		{
			fc.flight.waypoint_no = 0;
		}
	}
}

int read_waypoint(int num, volatile waypoint_t *wpt)
{
	FIL fp;
	FRESULT f_r;
	char line[80];
	char *start, *end;

	// Set defaults, if nothing could be found in the file:
	wpt->lat = 0;
	wpt->lon = 0;
	wpt->name[0] = 0;
	wpt->radius_m = 0;

	DEBUG("parse_waypoint(%s, %d)\n", waypoint_filename, num);

	f_r = f_open(&fp, waypoint_filename, FA_READ);
	if (f_r != FR_OK)
		return num;

	while (1)
	{
		if (f_gets(line, sizeof(line), &fp) == NULL)
			break;

		DEBUG_1("Read line: %s", line);
		if (line[0] == '#')
			continue;

		if (startsWith_P(PSTR("WP "), line))
		{
			start = index(line, '"');
			if (start != NULL)
			{
				start++;                           // skip "
				end = index(start, '"');
				if (end != NULL)
				{
					int len = min(sizeof(wpt->name) - 1, (size_t )(end - start)); // Ensure no buffer overflow
					*(start + len) = 0;
					strcpy((char *) wpt->name, start);

					end = skip_ws(end + 1);
					wpt->lat = atof(end) * 10000000.0;
					end = index(end, ' ');
					if (end != NULL)
					{
						end = skip_ws(end + 1);
						wpt->lon = atof(end) * 10000000.0;
						end = index(end, ' ');
						if (end != NULL)
						{
							wpt->radius_m = atoi(end);
							num--;
							if (num == 0)
								break; // "num" allows to select a specific waypoint in the file.
						}
					}
				}
			}
		}
	}
	DEBUG("read waypoint: name=%s lat=%ld lon=%ld radius_m=%d\n", wpt->name, wpt->lat, wpt->lon, wpt->radius_m);
	f_close(&fp);

	return num;
}

int count_waypoints()
{
	waypoint_t wpt;
	#define WAYPOINT_MAX 999

	return WAYPOINT_MAX - read_waypoint(WAYPOINT_MAX, &wpt);
}

/**
 * Go to the next waypoint in the list of waypoints.
 *
 * @return true if next waypoint is available, false otherwise
 */
bool waypoint_goto_next()
{
	if (fc.flight.waypoint_no < fc.flight.waypoints_count)
	{
		fc.flight.waypoint_no++;
		read_waypoint(fc.flight.waypoint_no, &(fc.flight.next_waypoint));
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
	if (fc.flight.waypoint_no > 1)
	{
		fc.flight.waypoint_no--;
		read_waypoint(fc.flight.waypoint_no, &(fc.flight.next_waypoint));
		show_waypoint();
		return true;
	}
	else
	{
		return false;
	}
}

