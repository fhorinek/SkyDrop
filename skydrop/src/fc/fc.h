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
#include "gyro.h"
#include "imu.h"

//metric to imperial
#define FC_METER_TO_FEET		(3.2808399)
#define FC_MPS_TO_100FPM		(1.96850394)  	//100 feet per min (WTF?)

#define FC_KNOTS_TO_KPH		(1.852)				//Kilometers per hour
#define FC_KNOTS_TO_MPH		(1.15077945)		//Miles per hour
#define FC_KNOTS_TO_MPS		(0.51444444444)		//Meters per seconds
#define FC_MPS_TO_KPH		(3.6)				//Kilometers per hour
#define FC_MPS_TO_MPH		(2.23693629)		//Miles per hour
#define FC_MPS_TO_KNOTS		(1.94384449)		//Knots
#define FC_KM_TO_MILE		(0.621371)


#define ALT_MODE_MASK	0b11000000

#define ALT_ABS_QNH1	0b00000000
#define ALT_ABS_QNH2	0b01000000
#define ALT_ABS_GPS		0b10000000
#define ALT_DIFF		0b11000000

#define ALT_REL_MASK	0b00000111

//single bit flags
#define ALT_UNIT_M		0b00000000
#define ALT_UNIT_I		0b00100000

#define ALT_AUTO_ZERO	0b00010000
#define ALT_AUTO_GPS	0b00001000

#define GPS_SAT_CNT	12
#define GPS_FIX_CNT_MAX		200
#define GPS_FIX_TIME_SYNC	10
#define GPS_FIX_ALT_SYNC	50

// The different logger states
#define LOGGER_IDLE			0
#define LOGGER_WAIT_FOR_GPS	1
#define LOGGER_ACTIVE		2
#define LOGGER_ERROR		3

/**
 * Bit masks for gps.new_sample.
 */
#define FC_GPS_NEW_SAMPLE_LOGGER		0b00000001
#define FC_GPS_NEW_SAMPLE_WIND			0b00000010
#define FC_GPS_NEW_SAMPLE_AGL			0b00000100
#define FC_GPS_NEW_SAMPLE_ODO			0b00001000
#define FC_GPS_NEW_SAMPLE_ALT           0b00010000
#define FC_GPS_NEW_SAMPLE_CIRCLE        0b00100000
#define FC_GPS_NEW_SAMPLE_AIRSPACE      0b01000000

// All lat/lon values are multiplied by GPS_COORD_MUL, so that we can use
// fixed point integer arithmetic instead of floating points:
#define GPS_COORD_MUL	10000000l

struct gps_data_t
{
	/**
	 * Set to "true" if we received a valid GPS position.
	 */
	bool valid;

	/**
	 * Whenever a new GPS position arrives, then
	 * new_sample will be set to 0xFF and every subroutine can check with its own bit (FC_GPS_NEW_SAMPLE_XXX)
	 * if there is a new GPS for them to be handled.
	 */
	uint8_t new_sample;

	char cache_igc_latitude[9];
	char cache_igc_longtitude[10];

	char cache_gui_latitude[16];
	char cache_gui_longtitude[16];

	int32_t latitude;	//* 10^7
	int32_t longtitude;	//* 10^7
	float groud_speed; 	//in knots

	uint16_t heading;
	uint32_t utc_time;

	uint8_t fix;        // GPGSA.fix: 1=No Fix, 2=2D fix, 3=3D fix
	float altitude;
	float geoid;
	float hdop;
	float vdop;
	float pdop;

	uint8_t sat_used;
	uint8_t sat_total;

	uint8_t sat_id[GPS_SAT_CNT];
	uint8_t sat_snr[GPS_SAT_CNT];

	uint8_t fix_cnt;     // number of fixes received since the first fix (kind of quality of fix).
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
	float zGCA;						//Z direction (external frame), gravity compensated acceleration in m/s
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
	vector_float_t vector;		//gyro data +calibration
	vector_float_t bias;		//
};

struct compass_data_t
{
	float azimuth;				//in degrees < 0 ; 360 )
	float azimuth_filtered;
	int16_t declination;		//magnetic declination offset in degrees +-180
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
	float fake;

	float pressure;
	float vario;

	uint16_t history_delay;
	int8_t history[VARIO_HISTORY_SIZE];
	uint8_t history_pointer;
	uint16_t history_step;

	float digital;
	float avg;

	uint32_t time_of_last_error_update;        // time in ms since the last error_over_time update
	float error_over_time;                     // This stores the error asscoiated with altitude1 in m.
};

struct waypoint_t
{
	uint8_t flags;
	int32_t lat;
	int32_t lon;
	char name[20];
	uint16_t radius_m;
	int16_t bearing;
	float distance;          // in km.
};


struct flight_stats_t
{
	int16_t max_alt;	//in m
	int16_t min_alt;	//in m

	int16_t max_climb; 	//in cm
	int16_t max_sink;	//in cm

};

#define FLIGHT_WAIT		0
#define FLIGHT_FLIGHT	1
#define FLIGHT_LAND		2

#define VARIO_CIRCLING_HISTORY_SCALE	12 // == 1m/s

struct flight_data_t
{
	flight_stats_t stats;

	//flight time and flight duration holder after land
	uint32_t timer; //in ms
	uint8_t state;

	//wait timer
	uint32_t autostart_timer;
	float autostart_altitude;

	//takeoff (home) position
	bool home_valid;

	//we will cache the bearing and distance to save cpu
	//computation @ widget_draw ~30Hz
	//computation @ gps new_sample ~1Hz
	int16_t home_bearing;
	float home_distance;    // in km

	uint32_t total_time; //in seconds

	uint32_t autostart_odo;   // odometer at start time.

    //waypoints
	uint8_t waypoint_no;                 // The number of the next waypoint where we need to fly. The first waypoint is "1".
	uint8_t waypoints_count;			 // The number of waypoints in the file.
	waypoint_t next_waypoint;        // The next waypoint where we need to fly

    //last gps heading
	int16_t last_heading;
	float avg_heading_change;
	int16_t total_heading_change;
	//uint32_t heading_change_timer;

	//circling
	bool circling;
	uint32_t circling_stop;
	uint32_t circling_start;
	uint16_t circling_time; //in sec
	float circling_start_altitude;
	float circling_gain; //in m

	int8_t circling_history[8];
};

struct agl_data_t
{
	bool valid;
	bool file_valid;

	char filename[10];        // The filename of the currently opened HAGL file
	int16_t ground_level;     // The ground level of the current GPS position or "AGL_INVALID".
	float ground_gradient;    // the gradient of the current GPS position
};


#define AIR_CACHE_VALID		0x01
#define AIR_CACHE_INSIDE	0x02
#define AIR_CACHE_NORMAL	0x04
#define AIR_CACHE_FAR		0x08

struct airspace_cache_t
{
	uint8_t flags;

	//target
	int32_t latitude;
	int32_t longtitude;

	//offset
	int8_t lat_offset;
    int8_t lon_offset;

	uint16_t floor;
	uint16_t ceil;

	uint8_t index;
	uint8_t airspace_class;
};

#define AIR_LEVELS					5
#define AIR_LEVEL_SIZE				3
#define AIR_INDEX_SIZE				64

struct airspace_data_t
{
	bool file_valid;

	char filename[10];        // The filename of the currently opened airspace file

	bool forbidden;           // Is the pilot currently inside the forbidden airspace?
	uint16_t angle;            // The angle out of the airspace or "AGL_INVALID".
	uint16_t distance_m;
	uint16_t ceiling;
	uint16_t floor;

	uint16_t min_alt;
	uint16_t max_alt;

	uint8_t airspace_name_index;
	char airspace_name[50];

	uint16_t cache_index;
	airspace_cache_t cache[AIR_LEVELS];
};

#define FC_GLIDE_MIN_KNOTS		(1.07) //2km/h
#define FC_GLIDE_MIN_SINK		(-0.01)

#define FC_ODO_MAX_SPEED_DIFF	(5.39957) 	//10km/h
#define FC_ODO_MIN_SPEED		(0.539957) //1km/h

#define FC_ALT_ALARM1_SUPPRESS		0b00000001
#define FC_ALT_ALARM2_SUPPRESS		0b00000010
#define FC_ALT_ALARM_H1_SUPPRESS	0b00000100

struct flight_computer_data_t
{
	accel_data_t acc;		//accelerometer data
	mag_data_t mag;			//magnetometer data
	gyro_data_t gyro;		//gyroscope data
	imu_data_t imu;			//imu data
	compass_data_t compass;	//compass data

	gps_data_t gps_data;

	wind_data_t wind; 		//wind calculation data

	temp_data_t temp;		//temperature and humidity data

	vario_data_t vario;		//vario, pressure, history data

	flight_data_t flight;	//flight related stats, measurements, data

	agl_data_t agl;

	airspace_data_t airspace;

	uint32_t odometer;              // in cm gives up to 42.000km

	uint8_t logger_state;           // One of the LOGGER_IDLE, LOGGER_WAIT_FOR_GPS, ...

	bool glide_ratio_valid;
	float glide_ratio;

	float altitude1;                // in meter
	int16_t altitudes[NUMBER_OF_ALTIMETERS];

	uint8_t altitude_alarm_status;
};

void fc_init();
void fc_step();
void fc_deinit();

void fc_pause();
void fc_continue();

uint8_t fc_active_alarm();

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
