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
	uint8_t widgets[WIDGES_PER_PAGE];
};

struct cfg_gui
{
	uint8_t contrast;
	uint8_t brightness;
	uint8_t brightness_timeout;

	cfg_gui_layout pages[NUMBER_OF_PAGES];
};

struct cfg_vario
{
	float digital_vario_dampening;
	float avg_vario_dampening;
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

	int16_t lift;		//in cm
	int16_t sink;		//in cm

	uint8_t	fluid;		//true/false
	uint8_t volume;		//0-100
};

struct cfg_system
{
	uint8_t usb_mode;
};

struct cfg_autostart
{
	uint8_t sensititvity;
	uint8_t supress_audio;
};

struct cfg_t
{
	cfg_gui gui;

	cfg_vario vario;
	cfg_altitude altitude;
	cfg_calibration calibration;
	cfg_audio_profile audio_profile;
	cfg_system system;
	cfg_autostart autostart;
};

extern cfg_t config;

#define CheckRange(MIN, MAX, DEFAULT, VAL) \
	if (VAL < MIN || VAL > MAX) VAL = DEFUALT;


#define USB_MODE_NONE			0
#define USB_MODE_MASSSTORAGE	1

#endif /* CONF_H_ */
