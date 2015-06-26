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
#define ALT_M_TO_F		(3.2808399)
#define VARIO_M_TO_F	(1.96850394)  //100 feat per min (WTF?)

#define ALT_ABS_QNH1	0b00000000
#define ALT_ABS_QNH2	0b01000000
#define ALT_ABS_GPS		0b10000000
#define ALT_DIFF		0b11000000

#define ALT_UNIT_M		0b00000000
#define ALT_UNIT_I		0b00100000

#define ALT_AUTO_ZERO	0b00010000

typedef struct
{
	uint8_t flags;
	int16_t delta;
	int16_t altitude;
} alt_data_t;

#define GPS_SAT_CNT	12
#define GPS_FIX_CNT_MAX		200
#define GPS_FIX_TIME_SYNC	10
#define GPS_FIX_ALT_SYNC	50

typedef struct
{
	bool valid;

	float latitude;
	float longtitude;
	float groud_speed;

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

	// --- CONST from cfg ---
	float QNH1;
	float QNH2;

	float digital_vario_dampening;
	float avg_vario_dampening;

	uint8_t vario_flags;

	vector_i16_t mag_bias;
	vector_i16_t mag_sensitivity;

	vector_i16_t acc_bias;
	vector_i16_t acc_sensitivity;

	uint16_t buzzer_freq[AUDIO_PROFILE_SIZE];
	uint16_t buzzer_pause[AUDIO_PROFILE_SIZE];
	uint16_t buzzer_length[AUDIO_PROFILE_SIZE];

	int16_t audio_lift;
	int16_t audio_sink;
	uint8_t audio_fluid;
	uint8_t audio_volume;
	uint8_t audio_supress;

	uint8_t usb_mode;

	uint8_t autostart_sensitivity;

	uint8_t use_gps;
	uint8_t time_flags;
	int8_t time_zone;

	// --- CALC ---
	bool baro_valid;
	float pressure;
	float vario;
	float digital_vario;
	float avg_vario;

	float altitude1;
	uint8_t alt1_flags;
	alt_data_t altimeter[NUMBER_OF_ALTIMETERS];

	vector_float_t mag_f;
	vector_float_t acc_f;
	vector_float_t gyro_f;

	uint32_t epoch_flight_start;
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

void fc_sync_gps_time();

extern volatile flight_data_t fc;

#endif /* FC_H_ */
