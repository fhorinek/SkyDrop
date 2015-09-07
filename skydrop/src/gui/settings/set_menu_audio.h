/*
 * settings.h
 *
 *  Created on: 5.5.2015
 *      Author: horinek
 */

#ifndef set_menu_audio_H_
#define set_menu_audio_H_

#include "../gui.h"

void gui_set_menu_audio_init();
void gui_set_menu_audio_stop();
void gui_set_menu_audio_loop();
void gui_set_menu_audio_irqh(uint8_t type, uint8_t * buff);
void gui_set_menu_audio_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_set_menu_audio_action(uint8_t index);

#endif /* set_menu_audio_H_ */
