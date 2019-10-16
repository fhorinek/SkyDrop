/*
 * settings.h
 *
 *  Created on: 5.5.2015
 *      Author: horinek
 */

#ifndef SET_AUDIO_H_
#define SET_AUDIO_H_

#include "../gui.h"

#define gui_set_audio_stop gui_dummy
#define gui_set_audio_loop gui_list_draw
#define gui_set_audio_irqh gui_list_irqh

void gui_set_audio_init();
void gui_set_audio_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_set_audio_action(uint8_t index);

#endif /* SETTINGS_H_ */
