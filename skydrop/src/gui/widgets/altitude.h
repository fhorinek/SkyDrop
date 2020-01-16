/*
 * altitude.h
 *
 *  Created on: 5.3.2015
 *      Author: horinek
 */

#ifndef ALTITUDE_H_
#define ALTITUDE_H_

#include "widgets.h"

extern widget w_alt1;
extern widget w_alt2;
extern widget w_alt3;
extern widget w_alt4;
extern widget w_alt5;

void widget_alt_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t index);

#endif /* ALTITUDE_H_ */
