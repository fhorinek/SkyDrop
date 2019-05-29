/*
 * airspace.h
 *
 *  Created on: 27.11.2018
 *      Author: tilmann@bubecks.de
 */

#ifndef FC_AIRSPACE_H
#define FC_AIRSPACE_H

#include "../common.h"

void airspace_init();
void airspace_read_pos(int32_t latitude, int32_t longtitude);
void airspace_is_forbidden();
void airspace_get_angle();
void airspace_get_distance();
void airspace_step();

#define AIRSPACE_INVALID 			0xFFFF
#define AIRSPACE_TOO_FAR 			0xFFFF
#define AIR_INDEX_INVALID			0x7F

#define AIR_RESOLUTION				200

#define MIN_RES						6
#define OFFSET_BASE					0.5
#define OFFSET_MUL_0				(OFFSET_BASE / 64)
#define OFFSET_MUL_1				((OFFSET_MUL_0 * MIN_RES) / 64)
#define OFFSET_MUL_2				((OFFSET_MUL_1 * MIN_RES) / 64)

#define AIR_AGL_FLAG 				0x8000

uint16_t airspace_convert_alt_ft(uint16_t raw_alt);
uint16_t airspace_convert_alt_m(uint16_t raw_alt);

#endif /* FC_AIRSPACE_H */
