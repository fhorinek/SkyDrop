/*
 * gui_homedetail.h
 *
 *  Created on: 11.05.2017
 *      Author: tilmann@bubecks.de
 */


#ifndef GUI_HOMEDETAIL_H_
#define GUI_HOMEDETAIL_H_

#include "../gui.h"
#include "../../fc/fc.h"

void gui_homedetail_init();
void gui_homedetail_stop();
void gui_homedetail_loop();
void gui_homedetail_irqh(uint8_t type, uint8_t * buff);
void gui_homedetail_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_homedetail_action(uint8_t index);

void read_homefile(const char *filename, home_t *home);

#endif /* GUI_HOMEDETAIL_H_ */
