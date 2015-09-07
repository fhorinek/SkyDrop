/*
 * fc.h
 *
 *  Created on: 23.1.2015
 *      Author: horinek
 */

#ifndef FC_H_
#define FC_H_

#include "../common.h"

#define NUMBER_OF_ALTIMETERS	4
#define AUDIO_PROFILE_SIZE		41

#include "conf.h"

//metric to imperial
#define FC_METER_TO_FEET		(3.2808399)
#define FC_MPS_TO_100FPM		(1.96850394)  	//100 feat per min (WTF?)

#define FC_KNOTS_TO_KPH		(1.852)				//Kilometers per hour
#define FC_KNOTS_TO_MPH		(1.15077945)		//Miles per hour
#define FC_KNOTS_TO_MPS		(0.51444444444)		//Meters peR seconds

#define ALT_MODE_MASK	0b11000000

#define ALT_ABS_QNH1	0b00000000
#define ALT_ABS_QNH2	0b01000000
#define ALT_ABS_GPS		0b10000000
#define ALT_DIFF		0b11000000

#define ALT_REL_MASK	0b00001111

//single bit flags
#define ALT_UNIT_M		0b00000000
#define ALT_UNIT_I		0b00100000

#define ALT_AUTO_ZERO	0b00010000

#define GPS_SAT_CNT	12
#define GPS_FIX_CNT_MAX		200
#define GPS_FIX_TIME_SYNC	10
#define GPS_FIX_ALT_SYNC	50

typedef struct
{
	bool valid;

	float latitude;
	float longtitude;
	float groud_speed; //in knots

	float heading;
	uint32_t utc_time;

	uint8_t fix;
	float altitude;
	float hdop;

	uint8_t sat_used;
	uint8_t sat_total;

	uint8_t sat_id[GPS_SAT_CNT];
	uint8_t sat_snr[GPS_SAT_CNT];

	uint8_t fix_cnt;
} gps_data_t;

#define FC_TEMP_PERIOD	1000

#define AUTOSTART_WAIT		0
#define AUTOSTART_FLIGHT	1
#define AUTOSTART_LAND		2

#define FC_GLIDE_MIN_KNOTS	(1.07) //2km/h
#define FC_AUTOSTART_RESET	(30)

typedef struct
{
	// --- RAW from sensors ---
	vector_i16_t mag_data;
	vector_i16_t acc_data;
	vector_i16_t gyro_data;

	gps_data_t gps_data;

	int16_t temperature;
	int16_t humidity;

	uint8_t temp_step;
	uint32_t temp_next;

	// --- CALC ---
	bool baro_valid;
	float pressure;

	float vario;

	float digital_vario;
	float avg_vario;

	bool glide_ratio_valid;
	float glide_ratio;

	float altitude1;

	int16_t altitudes[NUMBER_OF_ALTIMETERS];

	vector_float_t mag_f;
	vector_float_t acc_f;
	vector_float_t gyro_f;

	//serve as wait timer, flight time and flight duration holder after land
	uint32_t epoch_flight_timer;
	float start_altitude;
	uint8_t autostart_state;
} flight_data_t;

void fc_init();
void fc_step();
void fc_deinit();

void fc_pause();
void fc_continue();

float fc_alt_to_qnh(float alt, float pressure);
float fc_press_to_alt(float pressure, float qnh);
float fc_alt_to_press(float alt, float qnh);

void fc_zero_alt(uint8_t index);
void fc_manual_alt0_change(float val);

void fc_sync_gps_time();

void fc_takeoff();
void fc_landing();

extern volatile flight_data_t fc;

#endif /* FC_H_ */
