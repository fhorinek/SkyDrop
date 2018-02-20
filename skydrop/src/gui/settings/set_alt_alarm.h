/*
 * settings.h
 *
 *  Created on: 5.5.2015
 *      Author: horinek
 */

#ifndef SET_ALT_ALARM_H_
#define SET_ALT_ALARM_H_

#include "../gui.h"

void gui_set_alt_alarm_init();
void gui_set_alt_alarm_stop();
void gui_set_alt_alarm_loop();
void gui_set_alt_alarm_irqh(uint8_t type, uint8_t * buff);
void gui_set_alt_alarm_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_set_alt_alarm_action(uint8_t index);


#endif /* SET_ALT_ALARM_H_ */
