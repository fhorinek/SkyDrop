/*
 * agl.h
 *
 *  Created on: 19. 1. 2017
 *      Author: horinek
 */

#ifndef FC_AGL_H_
#define FC_AGL_H_

#include "../common.h"

void agl_init();
int16_t agl_get_alt(int32_t latitude, int32_t longtitude);
void agl_step();

#define AGL_INVALID -32767

#endif /* FC_AGL_H_ */
