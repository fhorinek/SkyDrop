#ifndef SET_ADVANCED_H_
#define SET_ADVANCED_H_

#include "../gui.h"

#define gui_set_advanced_stop gui_dummy
#define gui_set_advanced_loop gui_list_draw
#define gui_set_advanced_irqh gui_list_irqh

void gui_set_advanced_init();
void gui_set_advanced_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_set_advanced_action(uint8_t index);

#endif /* SET_ADVANCED_H_ */
