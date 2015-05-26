/*
 * settings.h
 *
 *  Created on: 5.5.2015
 *      Author: horinek
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "../gui.h"

void gui_settings_init();
void gui_settings_stop();
void gui_settings_loop();
void gui_settings_irqh(uint8_t type, uint8_t * buff);
void gui_settings_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_settings_action(uint8_t index);

#endif /* SETTINGS_H_ */
