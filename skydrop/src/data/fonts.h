/*
 * fonts.h
 *
 *  Created on: 27.2.2015
 *      Author: horinek
 */

#ifndef FONTS_H_
#define FONTS_H_

#include "../common.h"

extern const uint8_t font_6px_normal_ttf_8[] PROGMEM;
extern const uint8_t font_arial_bold_ttf_16[] PROGMEM;
extern const uint8_t font_arial_bold_ttf_36[] PROGMEM;
extern const uint8_t font_arial_ttf_12[] PROGMEM;
extern const uint8_t font_arial_ttf_10[] PROGMEM;

#define F_VALUES_S	font_6px_normal_ttf_8
#define F_VALUES_M	font_arial_ttf_10
#define F_VALUES_L	font_arial_bold_ttf_16
#define F_VALUES_XL	font_arial_bold_ttf_36

#define F_TEXT_S	font_6px_normal_ttf_8
#define F_TEXT_M	font_arial_ttf_10
#define F_TEXT_L	font_arial_ttf_12

#define F_LABEL		F_TEXT_S



#endif /* FONTS_H_ */
