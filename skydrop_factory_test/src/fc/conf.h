/*
 * conf.h
 *
 *  Created on: 2.3.2015
 *      Author: horinek
 */

#ifndef CONF_H_
#define CONF_H_

#include "fc.h"

typedef uint8_t		uint8_t_arr;
typedef uint16_t	uint16_t_arr;
typedef uint32_t	uint32_t_arr;

typedef int8_t		int8_t_arr;
typedef int16_t		int16_t_arr;
typedef int32_t		int32_t_arr;


#define CFG_DISP_INVERT			0b00000001
#define CFG_DISP_FLIP			0b00000010
#define CFG_DISP_ANIM			0b00000100
#define CFG_DISP_CYCLE			0b00001000               // Automatically cycle screens

#define CFG_AUDIO_MENU_SPLASH	0b10000000
#define CFG_AUDIO_MENU_PAGES	0b01000000
#define CFG_AUDIO_MENU_BUTTONS	0b00100000
#define CFG_AUDIO_MENU_GPS		0b00010000

#define PAGE_AUTOSET_CIRCLING_THOLD	6
#define PAGE_AUTOSET_CIRCLING_AVG	10

struct cfg_gui
{
	uint8_t contrast;
	uint8_t brightness;
	uint8_t brightness_timeout;
	uint8_t disp_flags;

	uint8_t menu_audio_flags;

	uint8_t menu_volume;
	uint8_t vario_volume;
	uint8_t vario_mute;
	uint8_t alert_volume;
};

#define TIME_DST	0b00000001
#define TIME_SYNC	0b00000010

//prevent fake true (0xFF) on not loaded eeprom
#define DEBUG_MAGIC_ON		0b10100101

struct cfg_system
{
	uint8_t debug_log;
	uint8_t debug_gps;
	uint8_t record_screen;
};


#define GPS_FORMAT_MASK	0b00001100

#define GPS_DDdddddd	0b00000000
#define GPS_DDMMmmm		0b00000100
#define GPS_DDMMSS		0b00001000

#define GPS_SPD_MASK	0b00000011

#define GPS_SPD_MS		0b00000000
#define GPS_SPD_KPH		0b00000001
#define GPS_SPD_MPH		0b00000010
#define GPS_SPD_KNOT	0b00000011

#define GPS_DIST_UNIT_MASK 0b00010000
#define GPS_DIST_UNIT_M	   0b00000000
#define GPS_DIST_UNIT_I	   0b00010000


#define PROTOCOL_DIGIFLY	0
#define PROTOCOL_LK8EX1		1
#define PROTOCOL_BLUEFLY	2

#define NUMBER_OF_PROTOCOLS	3

#define UART_FORWARD_DEBUG	0
#define UART_FORWARD_OFF	1
#define UART_FORWARD_9600	2
#define UART_FORWARD_19200	3
#define UART_FORWARD_38400	4
#define UART_FORWARD_57600	5
#define UART_FORWARD_115200	6

#define NUMBER_OF_UART_FORWARD	7

struct cfg_connectivity
{
	uint8_t usb_mode;

	uint8_t use_gps;
	uint8_t gps_format_flags;

	uint8_t use_bt;
	uint8_t_arr bt_link_partner[6];
	uint8_t_arr bt_link_key[16];
	uint8_t_arr btle_mac[6];

	uint8_t forward_gps;
	uint8_t protocol;

	uint8_t uart_function;
};


//Main user configurations
//start address				32	0x20
struct cfg_t
{
	uint32_t build_number;

	cfg_gui gui;

	cfg_system system;
	cfg_connectivity connectivity;
};


#define CFG_FACTORY_PASSED_hex	0xAA

//DO NOT CHANGE THE ORDER
struct cfg_calibration
{
	vector_i16_t mag_bias;			//6
	vector_i16_t mag_sensitivity;	//6

	vector_i16_t acc_bias;			//6
	vector_i16_t acc_sensitivity;	//6
};

//DO NOT CHANGE THE ORDER
struct debug_info
{
	uint32_t time;				//4
	uint16_t build_number;		//2
	uint32_t program_counter;	//4
	uint16_t min_stack;			//2
	uint16_t max_heap;			//2
};

//0xFF or 0b11111111 is default value
#define CALIB_DEFAULT_LOADED	0x12	//if you need to replace calibration defaults, increase this number

//DO NOT CHANGE THE ORDER, add new value at the end
//Device config not related to user settings
//						dec		hex
//size 					128		0x80
//start address			1920	0x780
struct cfg_ro_t
{									//offset	size
	uint8_t factory_passed;			//+0		1
	uint8_t lcd_contrast_min;		//+1		1
	uint8_t lcd_contrast_max;		//+2		1

	uint8_t bt_module_type; 		//+3		1

	cfg_calibration calibration;	//+4		24

	uint8_t hw_revision;			//+28		1

	uint8_t flight_number;			//+29		1
	uint32_t flight_date;			//+30		4

	debug_info debug;				//+34		14

	uint16_t bat_adc_max;			//+48		2

	uint8_t calibration_flags;		//+50		1

	vector_i16_t gyro_bias;			//+51		6

	int16_t	magnetic_declination;	//+57		2

	uint32_t total_flight_time;		//+59		4

	int16_t baro_offset;			//+61		2

	uint8_t reserved[70];			//+63		62
};

//configuration in RAM
extern volatile cfg_t config;


extern cfg_ro_t config_ro __attribute__ ((section(".cfg_ro")));

#define CheckRange(MIN, MAX, DEFAULT, VAL) \
	if (VAL < MIN || VAL > MAX) VAL = DEFUALT;


#define USB_MODE_NONE			0
#define USB_MODE_MASSSTORAGE	1

bool cfg_factory_passed();
void cfg_reset_factory_test();
void cfg_load();

#endif /* CONF_H_ */
