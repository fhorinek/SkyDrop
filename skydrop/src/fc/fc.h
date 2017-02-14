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
#include "fake_data.h"

#include "wind.h"

#include "acc.h"
#include "mag.h"

//metric to imperial
#define FC_METER_TO_FEET		(3.2808399)
#define FC_MPS_TO_100FPM		(1.96850394)  	//100 feet per min (WTF?)

#define FC_KNOTS_TO_KPH		(1.852)				//Kilometers per hour
#define FC_KNOTS_TO_MPH		(1.15077945)		//Miles per hour
#define FC_KNOTS_TO_MPS		(0.51444444444)		//Meters per seconds
#define FC_MPS_TO_KPH		(3.6)				//Kilometers per hour
#define FC_MPS_TO_MPH		(2.23693629)		//Miles per hour
#define FC_MPS_TO_KNOTS		(1.94384449)		//Knots


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

#define LOGGER_IDLE			0
#define LOGGER_WAIT_FOR_GPS	1
#define LOGGER_ACTIVE		2
#define LOGGER_ERROR		3

struct flight_stats_t
{
	int16_t max_alt;	//in m
	int16_t min_alt;	//in m

	int16_t max_climb; 	//in cm
	int16_t max_sink;	//in cm
};

#define FC_GPS_NEW_SAMPLE_LOGGER		0b00000001
#define FC_GPS_NEW_SAMPLE_WIND			0b00000010
#define FC_GPS_NEW_SAMPLE_AGL			0b00000100

struct gps_data_t
{
	bool valid;
	uint8_t new_sample;

	char cache_igc_latitude[9];
	char cache_igc_longtitude[10];

	char cache_gui_latitude[16];
	char cache_gui_longtitude[16];

	int32_t latitude;	//* 10^7
	int32_t longtitude;	//* 10^7
	float groud_speed; 	//in knots

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
};

struct accel_data_t
{
	vector_float_t sens;
	vector_float_t bias;

	vector_i16_t raw;				//raw data from sensor
	vector_float_t vector;			//acceleration vector in g, max +-16g for each axis
	float total;					//total acceleration, absolute value of vector

	float filter_old;
	uint8_t filter_hold_time;
	float total_filtered;			//total acceleration, + filtered, + peak detection, used by acceleration widget
};

struct mag_data_t
{
	vector_float_t bias;
	vector_float_t sens;

	vector_i16_t raw;			//raw data from sensor
	vector_float_t vector;		//magnetic vector, not in scale
};

struct gyro_data_t
{
	vector_i16_t raw;			//raw data from sensor
};

#define FC_TEMP_PERIOD	100
struct temp_data_t
{
	int16_t temp;				//temperature
	int16_t humid;				//humidity
	uint8_t step;
	uint8_t cnt;
};

#define VARIO_HISTORY_SIZE 32
#define VARIO_HISTORY_SCALE	24 // == 1m/s
struct vario_data_t
{
	bool valid;					//baro valid

	float pressure;
	float vario;

	uint16_t history_delay;
	int8_t history[VARIO_HISTORY_SIZE];
	uint8_t history_pointer;
	uint16_t history_step;

	float digital;
	float avg;
};

#define FLIGHT_WAIT		0
#define FLIGHT_FLIGHT	1
#define FLIGHT_LAND		2
struct flight_data_t
{
	flight_stats_t stats;

	//flight time and flight duration holder after land
	uint32_t timer; //in ms
	uint8_t state;

	//wait timer
	uint32_t autostart_timer;
	float autostart_altitude;
};

struct agl_data_t
{
	bool valid;
	bool file_valid;

	char filename[10];
	int16_t ground_level;
};

#define FC_GLIDE_MIN_KNOTS	(1.07) //2km/h
#define FC_GLIDE_MIN_SINK	(-0.01)

struct flight_computer_data_t
{
	accel_data_t acc;		//accelerometer data
	mag_data_t mag;			//magnetometer data
	gyro_data_t gyro;		//gyroscope data

	gps_data_t gps_data;

	wind_data_t wind; 		//wind calculation data

	temp_data_t temp;		//temperature and humidity data

	vario_data_t vario;		//vario, pressure, history data

	flight_data_t flight;	//flight related stats, measurements, data

	agl_data_t agl;

	uint8_t logger_state;

	bool glide_ratio_valid;
	float glide_ratio;

	float altitude1;
	int16_t altitudes[NUMBER_OF_ALTIMETERS];
};

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
void fc_reset();

void fc_log_battery();

extern volatile flight_computer_data_t fc;

#endif /* FC_H_ */
