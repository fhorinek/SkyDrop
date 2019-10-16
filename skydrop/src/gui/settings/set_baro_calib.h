/*
 * set_baro_calib.h
 */

#ifndef GUI_SETTINGS_GUI_SET_BARO_H_
#define GUI_SETTINGS_GUI_SET_BARO_H_

#include "../gui.h"

#define set_baro_calib_stop gui_dummy
#define set_baro_calib_loop gui_list_draw
#define set_baro_calib_irqh gui_list_irqh

void set_baro_calib_init();
void set_baro_calib_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void set_baro_calib_action(uint8_t index);

#endif /* GUI_SETTINGS_GUI_SET_COMPASS_H_ */
