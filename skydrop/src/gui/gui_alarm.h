/*
 * alarm.h
 *
 *  Created on: 8. 2. 2018
 *      Author: horinek
 */

#ifndef GUI_GUI_ALARM_H_
#define GUI_GUI_ALARM_H_

#include "../common.h"

void gui_set_alarm_init();
void gui_set_alarm_stop();
void gui_set_alarm_loop();
void gui_set_alarm_irqh(uint8_t type, uint8_t * buff);

#endif /* GUI_GUI_ALARM_H_ */
