/*
 * wind2.h
 *
 *  Created on: Jan 23, 2017
 *      Author: fiala
 */

#ifndef WIND_H_
#define WIND_H_

#include "../common.h"
#include "../debug.h"

#define number_of_sectors 8

struct vector_2d_t
{
	float X;
	float Y;
};

struct wind_data_t
{
	float 		dir[number_of_sectors];
	float 		spd[number_of_sectors];
//	vector_2d_t old_gps;	//GPS position input mode only
	uint8_t 	old_sector;
	int8_t		sectors_cnt;

	//calculated wind values
	bool 		valid;		// was wind calculated?
	float 		speed;		// m/s
	float 		direction;	// degrees
};


void wind_init(void);
void wind_step(void);


#endif /* WIND_H_ */
