/*
 * set_vario_advanced.h
 *
 *  Created on: 4.9.2015
 *      Author: horinek
 */

#ifndef SET_VARIO_ADVANCED_H_
#define SET_VARIO_ADVANCED_H_

#include "../gui.h"

void gui_set_vario_advanced_init();
void gui_set_vario_advanced_stop();
void gui_set_vario_advanced_loop();
void gui_set_vario_advanced_irqh(uint8_t type, uint8_t * buff);
void gui_set_vario_advanced_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_set_vario_advanced_action(uint8_t index);

#endif /* SET_VARIO_ADVANCED_H_ */
