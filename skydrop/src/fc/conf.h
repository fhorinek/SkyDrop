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
	uint8_t alert_volume;

	uint8_t number_of_pages;
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

struct cfg_calibration
{
	vector_i16_t mag_bias;
	vector_i16_t mag_sensitivity;

	vector_i16_t acc_bias;
	vector_i16_t acc_sensitivity;
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

#define NUMBER_OF_PROTOCOLS	3

//#define PROTOCOL_FLYNET		3

struct cfg_system
{
	uint8_t usb_mode;

	uint8_t use_gps;
	uint8_t use_bt;
	uint8_t forward_gps;
	uint8_t protocol;

	uint8_t time_flags;
	int8_t time_zone;

	uint8_t gps_format_flags;
};

struct cfg_autostart
{
	uint8_t sensititvity;
	uint8_t supress_audio;
};

struct cfg_t
{
	uint32_t build_number;

	cfg_gui gui;

	cfg_vario vario;
	cfg_altitude altitude;
	cfg_audio_profile audio_profile;
	cfg_system system;
	cfg_autostart autostart;
};

//DO NOT CHANGE THE ORDER

#define CFG_FACTORY_PASSED_hex	0xAA

struct cfg_ro_t //128
{
	uint8_t factory_passed;			//1
	uint8_t lcd_contrast_min;		//1
	uint8_t lcd_contrast_max;		//1

	uint8_t bt_module_type; 		//1

	cfg_calibration calibration;	//24

	uint8_t hw_revision;			//1

	uint8_t reserved[99];
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
