#ifndef SET_ADVANCED_H_
#define SET_ADVANCED_H_

#include "../gui.h"

void gui_set_advanced_init();
void gui_set_advanced_stop();
void gui_set_advanced_loop();
void gui_set_advanced_irqh(uint8_t type, uint8_t * buff);
void gui_set_advanced_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_set_advanced_action(uint8_t index);

#endif /* SET_ADVANCED_H_ */
