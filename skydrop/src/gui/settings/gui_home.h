/*
 * gui_home.h
 *
 *  Created on: 11.05.2017
 *      Author: tilmann@bubecks.de
 */


#ifndef GUI_HOME_H_
#define GUI_HOME_H_

#include "../gui.h"

void gui_home_init();
void gui_home_stop();
void gui_home_loop();
void gui_home_irqh(uint8_t type, uint8_t * buff);
void gui_home_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_home_action(uint8_t index);

#endif /* GUI_HOME_H_ */
