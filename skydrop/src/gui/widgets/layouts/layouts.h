/*
 * layouts.h
 *
 *  Created on: 2.3.2015
 *      Author: horinek
 */

#ifndef LAYOUTS_H_
#define LAYOUTS_H_

struct layout_t;

#include "../widgets.h"


struct widget_pos {
	uint8_t x;
	uint8_t y;
	uint8_t w;
	uint8_t h;
};

struct layout_desc {
	uint8_t number_of_widgets;

	widget_pos widgets[WIDGES_PER_PAGE];
};


struct layout_t
{
	uint8_t type;

	uint8_t widgets[WIDGES_PER_PAGE];
};


#define LAYOUT_OFF		0xFF
#define LAYOUT_1		0
#define LAYOUT_12		1
#define LAYOUT_122		2
#define LAYOUT_123		3
#define LAYOUT_22		4
#define LAYOUT_222		5
#define LAYOUT_333		6

#define NUMBER_OF_LAYOUTS	7


extern const layout_desc * layout_list[NUMBER_OF_LAYOUTS];

#endif /* LAYOUTS_H_ */
