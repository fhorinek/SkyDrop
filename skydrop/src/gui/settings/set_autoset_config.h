/*
 * set_autoset_config.h
 *
 *  Created on: 25.5.2015
 *      Author: horinek
 */

#ifndef SET_AUTOSET_CONFIG_H_
#define SET_AUTOSET_CONFIG_H_

#include "../gui.h"

#define gui_set_autoset_config_stop gui_dummy
#define gui_set_autoset_config_loop gui_list_draw
#define gui_set_autoset_config_irqh gui_list_irqh

void gui_set_autoset_config_init();
void gui_set_autoset_config_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_set_autoset_config_action(uint8_t index);

#endif /* SET_autoset_config_H_ */
