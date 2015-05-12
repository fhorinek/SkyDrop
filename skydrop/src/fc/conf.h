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

struct cfg_t
{
	cfg_gui gui;

	cfg_vario vario;
	cfg_altitude altitude;
	cfg_calibration calibration;
};


extern cfg_t config;

#define CheckRange(MIN, MAX, DEFAULT, VAL) \
	if (VAL < MIN || VAL > MAX) VAL = DEFUALT;


#endif /* CONF_H_ */
