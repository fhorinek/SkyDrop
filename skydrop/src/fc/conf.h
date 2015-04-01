/*
 * conf.h
 *
 *  Created on: 2.3.2015
 *      Author: horinek
 */

#ifndef CONF_H_
#define CONF_H_

#include "../gui/widgets/widgets.h"

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

struct cfg_altitude
{
	float QNH1;
	float QNH2;
};

struct cfg_t
{
	cfg_gui gui;

	cfg_vario vario;
	cfg_altitude altitude;
};


extern cfg_t config;

#define CheckRange(MIN, MAX, DEFAULT, VAL) \
	if (VAL < MIN || VAL > MAX) VAL = DEFUALT;


#endif /* CONF_H_ */
