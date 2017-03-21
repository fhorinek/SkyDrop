/*
 * settings.h
 *
 *  Created on: 5.5.2015
 *      Author: horinek
 */

#ifndef SET_autostart_H_
#define SET_autostart_H_

#include "../gui.h"

void gui_set_autostart_init();
void gui_set_autostart_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_set_autostart_action(uint8_t index);

#endif /* SETTINGS_H_ */
