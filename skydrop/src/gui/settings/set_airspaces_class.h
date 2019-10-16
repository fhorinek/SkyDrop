#ifndef SET_airspaces_class_H_
#define SET_airspaces_class_H_

#include "../gui.h"

#define gui_set_airspaces_class_stop gui_dummy
#define gui_set_airspaces_class_loop gui_list_draw
#define gui_set_airspaces_class_irqh gui_list_irqh

void gui_set_airspaces_class_init();
void gui_set_airspaces_class_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_set_airspaces_class_action(uint8_t index);

#endif /* SET_airspaces_class_H_ */
