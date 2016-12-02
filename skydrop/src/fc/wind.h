/*
 * wind.h
 *
 *  Created on: 16.9.2016
 *      Author: fiala
 */

#ifndef WIND_H_
#define WIND_H_

#include "../common.h"
#include "../debug.h"


#define	wind_kBuff_size	64

struct vector_2d_t
{
	float X;
	float Y;
};

struct wind_calc_t
{
	bool fromEnd;
	vector_2d_t kBuff[wind_kBuff_size];
	uint8_t kBuff_start;
	uint8_t kBuff_end;
	bool kBuff_full;
	vector_2d_t old_gps;
	uint32_t old_time;
	//
	uint8_t circle_end;
	float oldAngle;
	float sumAngle;
	vector_2d_t wind_coords;
	bool first_wind_calc_data;
	bool first_wind_calc_avg;
	uint8_t avg_weight;
	uint8_t nOfTurns_old;

	//calculated wind values
	bool valid;			// was wind calculated?
	float speed;		// m/s
	float direction;	// degrees
};


void wind_init(void);
void wind_step(void);


#endif /* WIND_H_ */
