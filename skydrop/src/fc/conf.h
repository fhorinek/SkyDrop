/*
 * conf.h
 *
 *  Created on: 2.3.2015
 *      Author: horinek
 */

#ifndef CONF_H_
#define CONF_H_

#include "../gui/widgets/widgets.h"
#include "fc.h"

typedef uint8_t		uint8_t_arr;
typedef uint16_t	uint16_t_arr;
typedef uint32_t	uint32_t_arr;

typedef int8_t		int8_t_arr;
typedef int16_t		int16_t_arr;
typedef int32_t		int32_t_arr;

struct cfg_gui_layout
{
	uint8_t type;
	uint8_t widgets[MAX_WIDGES_PER_PAGE];
};

#define CFG_DISP_INVERT			0b00000001
#define CFG_DISP_FLIP			0b00000010
#define CFG_DISP_ANIM			0b00000100

#define CFG_AUDIO_MENU_SPLASH	0b10000000
#define CFG_AUDIO_MENU_PAGES	0b01000000
#define CFG_AUDIO_MENU_BUTTONS	0b00100000
#define CFG_AUDIO_MENU_GPS		0b00010000

struct cfg_gui
{
	uint8_t contrast;
	uint8_t brightness;
	uint8_t brightness_timeout;
	uint8_t disp_flags;

	uint8_t last_page;
	uint8_t menu_audio_flags;

	uint8_t menu_volume;
	uint8_t vario_volume;
	uint8_t vario_mute;
	uint8_t alert_volume;

	uint8_t number_of_pages;
	uint8_t silent;
	uint8_t hide_label;
	cfg_gui_layout pages[MAX_NUMBER_OF_PAGES];
};

#define VARIO_UNITS_M		0b00000000
#define VARIO_UNITS_I		0b10000000

struct cfg_vario
{
	float digital_vario_dampening;
	float avg_vario_dampening;

	uint8_t flags;
	uint8_t weak_lift_enabled;
	uint8_t weak_lift;
};

struct cfg_altimeter
{
	uint8_t flags;
	int16_t delta;
};

struct cfg_altitude
{
	float QNH1;
	float QNH2;

	uint8_t alt1_flags;
	cfg_altimeter altimeter[NUMBER_OF_ALTIMETERS];
};

struct cfg_audio_profile
{
	uint16_t freq[41];	//in Hz
	uint16_t pause[41];	//in ms
	uint16_t length[41];//in ms

	uint16_t weak_lift_freq; //in Hz

	int16_t lift;		//in cm
	int16_t sink;		//in cm

	uint8_t	fluid;		//true/false
};

#define TIME_DST	0b00000001
#define TIME_SYNC	0b00000010

//prevent fake true (0xFF) on not loaded eeprom
#define DEBUG_MAGIC_ON		0b10100101

struct cfg_system
{
	uint8_t time_flags;
	int8_t time_zone;

	uint8_t debug_log;
	uint8_t debug_gps;
	uint8_t record_screen;

	uint8_t auto_power_off; //in minutes
};

#define AUTOSTART_SUPRESS_AUDIO		0b00000001
#define AUTOSTART_ALWAYS_ENABLED	0b00000010

struct cfg_autostart
{
	uint8_t start_sensititvity;
	uint8_t land_sensititvity;
	uint8_t timeout;
	uint8_t flags;
};

#define LOGGER_IGC	0
#define LOGGER_KML	1
#define LOGGER_RAW	2
#define LOGGER_AERO	3

#define NUMBER_OF_FORMATS	4
#define LOG_TEXT_LEN		50

struct cfg_logger
{
	uint8_t enabled;
	uint8_t format;

	char pilot[LOG_TEXT_LEN];
	char glider_type[LOG_TEXT_LEN];
	char glider_id[LOG_TEXT_LEN];
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

#define PROTOCOL_DIGIFLY	0
#define PROTOCOL_LK8EX1		1
#define PROTOCOL_BLUEFLY	2
#define PROTOCOL_SKYBEAN	3

#define NUMBER_OF_PROTOCOLS	4

#define UART_FORWARD_DEBUG	0
#define UART_FORWARD_OFF	1
#define UART_FORWARD_9600	2
#define UART_FORWARD_19200	3
#define UART_FORWARD_38400	4
#define UART_FORWARD_57600	5
#define UART_FORWARD_115200	6

#define NUMBER_OF_UART_FORWARD	7

#define LOGIN_PASSWORD_LEN 16

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

	char password[LOGIN_PASSWORD_LEN];
};

//Main user configurations
//start address				32	0x20
struct cfg_t
{
	uint32_t build_number;

	cfg_gui gui;

	cfg_vario vario;
	cfg_altitude altitude;
	cfg_audio_profile audio_profile;
	cfg_system system;
	cfg_autostart autostart;
	cfg_logger logger;
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
#define CALIB_ACC_NOT_DONE	0b00000001
#define CALIB_MAG_NOT_DONE	0b00000010

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

	uint8_t reserved[78];			//+52		77
};

//configuration in RAM
extern volatile cfg_t config;
//configuration in EE
extern cfg_t config_ee;

extern cfg_ro_t config_ro __attribute__ ((section(".cfg_ro")));

#define CheckRange(MIN, MAX, DEFAULT, VAL) \
	if (VAL < MIN || VAL > MAX) VAL = DEFUALT;


#define USB_MODE_NONE			0
#define USB_MODE_MASSSTORAGE	1

bool cfg_factory_passed();
void cfg_reset_factory_test();
void cfg_load();

#endif /* CONF_H_ */
