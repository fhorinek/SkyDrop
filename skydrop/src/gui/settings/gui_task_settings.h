
#ifndef GUI_TASK_SETTINGS_H_
#define GUI_TASK_SETTINGS_H_

#include "../gui.h"

#define gui_task_settings_stop gui_dummy
#define gui_task_settings_loop gui_list_draw
#define gui_task_settings_irqh gui_list_irqh

void gui_task_settings_init();

void gui_task_settings_action(uint8_t index);
void gui_task_settings_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);


extern uint8_t gui_task_settings_mode;

#endif /* GUI_TASK_SETTINGS_H_ */
