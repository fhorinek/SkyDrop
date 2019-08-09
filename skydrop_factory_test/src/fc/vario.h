/*
 * vario.h
 *
 *  Created on: 4.3.2015
 *      Author: horinek
 */

#ifndef FC_VARIO_H_
#define FC_VARIO_H_

#include "../common.h"

void vario_init();
void vario_calc(float pressure);
void vario_update_history_delay();

#endif /* FC_VARIO_H_ */
