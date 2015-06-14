/*
 * gui.h
 *
 *  Created on: 22.1.2015
 *      Author: horinek
 */

#ifndef GUI_H_
#define GUI_H_

#include "../common.h"
#include "../drivers/n5110_disp.h"
#include "../drivers/led.h"

extern n5110display disp;
extern FILE * lcd_out;

#define GUI_DISP_WIDTH	n5110_width
#define GUI_DISP_HEIGHT	n5110_height

void gui_switch_task(uint8_t new_task);

void gui_init();
void gui_load_eeprom();

void gui_stop();
void gui_loop();
void gui_irqh(uint8_t type, uint8_t * buff);

void gui_force_loop();

void gui_trigger_backlight();
void gui_set_contrast(uint8_t contrast);
void gui_showmessage_P(const char * msg);

void gui_dialog(char * title);
#define GUI_DIALOG_TOP 	13
#define GUI_DIALOG_LEFT 2
#define GUI_DIALOG_RIGHT (GUI_DISP_WIDTH - 3)
#define GUI_DIALOG_BOTTOM (GUI_DISP_HEIGHT - 3)


void gui_raligh_text(char * text, uint8_t x, uint8_t y);
void gui_caligh_text(char * text, uint8_t x, uint8_t y);

void gui_statusbar();

#define GUI_NONE			0xFF
#define GUI_PAGES			0
#define GUI_SETTINGS		1
#define GUI_SPLASH			2
#define GUI_SET_VARIO		3
#define GUI_SET_VAL			4
#define GUI_SET_AUDIO		5
#define GUI_SET_WIDGETS		6
#define GUI_LAYOUTS			7
#define GUI_SET_LAYOUT		8
#define GUI_SET_DISPLAY		9
#define GUI_USB				10
#define GUI_FTEST			11
#define GUI_SET_SYSTEM		12
#define GUI_SET_AUTOSTART	13
#define GUI_SET_GPS			14
#define GUI_SET_GPS_DETAIL	15

extern uint8_t lcd_brightness;
extern uint8_t lcd_brightness_timeout;
extern uint8_t lcd_contrast;

#endif /* GUI_H_ */
