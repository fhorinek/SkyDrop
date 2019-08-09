/*
 * set_baro_calib.h
 */

#ifndef GUI_SETTINGS_GUI_SET_BARO_H_
#define GUI_SETTINGS_GUI_SET_BARO_H_

#include "../gui.h"

void set_baro_calib_init();
void set_baro_calib_stop();
void set_baro_calib_loop();
void set_baro_calib_irqh(uint8_t type, uint8_t * buff);
void set_baro_calib_item(uint8_t index, char * text, uint8_t * flags, char * sub_text);
void set_baro_calib_action(uint8_t index);



#endif /* GUI_SETTINGS_GUI_SET_COMPASS_H_ */
