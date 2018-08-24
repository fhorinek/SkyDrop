/*
 * igc.h
 *
 *  Created on: 5.10.2015
 *      Author: horinek
 */

#ifndef IGC_H_
#define IGC_H_

#include "../../common.h"

void igc_comment(char * text);
uint8_t igc_start(char * path);
void igc_step();
void igc_stop();
void igc_pre_step();


#endif /* IGC_H_ */
