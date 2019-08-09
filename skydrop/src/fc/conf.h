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
#define CFG_DISP_CYCLE			0b00001000               // Automatically cycle screens

#define CFG_AUDIO_MENU_SPLASH	0b10000000
#define CFG_AUDIO_MENU_PAGES	0b01000000
#define CFG_AUDIO_MENU_BUTTONS	0b00100000
#define CFG_AUDIO_MENU_GPS		0b00010000

#define PAGE_NONE               0xFF
#define PAGE_MODE_PREPARE       0
#define PAGE_MODE_CIRCLING      1
#define PAGE_MODE_NORMAL        2
#define PAGE_MODE_ACRO          3
#define PAGE_MODE_LANDED        4
#define NUMBER_OF_PAGE_MODES    5

#define PAGE_AUTOSET_CIRCLING_THOLD	6
#define PAGE_AUTOSET_CIRCLING_AVG	10

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

	int8_t page_acro_thold; //*10
	uint8_t page_circling_timeout; //in s
    uint8_t page_mode[NUMBER_OF_PAGE_MODES];
    cfg_gui_layout pages[MAX_NUMBER_OF_PAGES];
};

#define VARIO_UNITS_M		0b00000000
#define VARIO_UNITS_I		0b10000000
#define VARIO_USE_ACC		0b01000000

struct cfg_vario
{
	float digital_vario_dampening;
	float avg_vario_dampening;

	uint8_t flags;
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

	uint8_t alarm_enabled;
	uint8_t alarm_confirm_secs; // Number of secs to confirm an alarm. "0" means forever.
	uint16_t alarm_1;          // Lower alarm 1. If lower than this: alarm
	uint16_t alarm_2;          // Lower alarm 2. If lower than this: alarm
	uint16_t alarm_h1;         // Height alarm 1. If higher than this: alarm
	uint16_t alarm_reset;
};

#define AUDIO_FLUID		0b00000001
#define AUDIO_WEAK		0b00000010
#define AUDIO_BEEP_SINK	0b00000100

struct cfg_audio_profile
{
	uint16_t freq[41];	//in Hz
	uint16_t pause[41];	//in ms
	uint16_t length[41];//in ms

	int16_t lift;		//in cm
	int16_t sink;		//in cm
	int16_t weak;		//in cm

	uint8_t	flags;

	uint8_t prebeep_offset; //in Hz
	uint8_t prebeep_length; //in ms
};

#define TIME_DST	0b00000001
#define TIME_SYNC	0b00000010

//prevent fake true (0xFF) on not loaded eeprom
#define DEBUG_MAGIC_ON		0b10100101

struct cfg_system
{
	uint8_t time_flags;
	int8_t time_zone;      // number of hours +/- UTC multiplied by 2

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
//#define LOGGER_AERO	3

#define NUMBER_OF_FORMATS	3
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

#define GPS_DIST_UNIT_MASK 0b00010000
#define GPS_DIST_UNIT_M	   0b00000000
#define GPS_DIST_UNIT_I	   0b00010000

#define PROTOCOL_DIGIFLY	0
#define PROTOCOL_LK8EX1		1
#define PROTOCOL_BLUEFLY	2
//#define PROTOCOL_SKYBEAN	3

#define NUMBER_OF_PROTOCOLS	3

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

#define HOME_SET_AS_TAKEOFF		0b00000001
#define HOME_LOADED_FROM_SD		0b00000010

struct cfg_home
{
	uint8_t flags;
	int32_t lat;
	int32_t lon;
	char name[20];
    char freq[10];
    char rwy[20];
    char traffic_pattern[20];
    char info[80];
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
	cfg_home home;
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
//
//This data is accessible by using eeprom_read_byte/eeprom_update_byte...
//
//						dec		hex
//size 					384		0x180
//start address			1920	0x680
struct cfg_ro_t
{									//offset	size
	uint8_t reserved0[54];          //+0        54

	uint16_t bat_runtime_minutes;   //+54       2    How many minutes does the battery run if full? 0xffff=no calib done
	uint16_t bat_calibration[100];  //+56       200

	uint8_t factory_passed;			//+256		1
	uint8_t lcd_contrast_min;		//+257		1
	uint8_t lcd_contrast_max;		//+258		1

	uint8_t bt_module_type; 		//+259		1

	cfg_calibration calibration;	//+260		24

	uint8_t hw_revision;			//+284		1

	uint8_t flight_number;			//+285		1
	uint32_t flight_date;			//+286		4

	debug_info debug;				//+290		14

	uint16_t bat_adc_max;			//+304		2

	uint8_t calibration_flags;		//+306		1

	vector_i16_t gyro_bias;			//+307		6

	int16_t	magnetic_declination;	//+313		2

	uint32_t total_flight_time;		//+315		4

	int16_t baro_offset;			//+319		2

	uint8_t reserved[70];			//+321		62
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

void cfg_load();

#endif /* CONF_H_ */
