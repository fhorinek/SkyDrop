/*
 * fc.h
 *
 *  Created on: 23.1.2015
 *      Author: horinek
 */

#ifndef FC_H_
#define FC_H_

#include "../common.h"
#include "conf.h"

typedef struct
{

} gps_data_t;

extern gps_data_t gps_data;


typedef struct
{
	// --- RAW from sensors ---
	bool mag_data_new;
	vector_i16_t mag_data;

	bool acc_data_new;
	vector_i16_t acc_data;

	bool gps_data_new;
	gps_data_t gps_data;

	// --- CONST from cfg ---
	float QNH1;
	float QNH2;

	float digital_vario_dampening;
	float avg_vario_dampening;

	// --- CALC ---
	bool baro_valid;
	float vario;
	float digital_vario;
	float avg_vario;
	float altitude1;
	float altitude2;

} flight_data_t;

void fc_init();
void fc_step();

extern volatile flight_data_t flight_data;

#endif /* FC_H_ */
