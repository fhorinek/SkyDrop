#ifndef SET_AIRSPACES_H_
#define SET_AIRSPACES_H_

#include "../gui.h"

#define gui_set_airspaces_stop gui_dummy
#define gui_set_airspaces_loop gui_list_draw
#define gui_set_airspaces_irqh gui_list_irqh

void gui_set_airspaces_init();
void gui_set_airspaces_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void gui_set_airspaces_action(uint8_t index);

#endif /* SET_AIRSPACES_H_ */
