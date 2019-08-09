/*
 * task_update.cpp
 *
 *  Created on: 26.8.2015
 *      Author: horinek
 */

#include "task_update.h"

#include "../gui/update.h"
#include "../gui/gui_dialog.h"

uint8_t update_state;
SleepLock update_lock;

void task_update_init()
{
	DEBUG(" *** THIS IS TASK UPDATE ***\n");

	storage_init();

	//init gui
	gui_init();
	gui_dialog_set_P(PSTR("Update"), PSTR("Update file found\nApply?"), GUI_STYLE_YESNO, gui_update_cb);
	gui_switch_task(GUI_DIALOG);
	gui_trigger_backlight();

	update_state = UPDATE_IDLE;
	update_lock.Lock();
}


void task_update_stop()
{
	gui_stop();
	storage_deinit();
	update_lock.Unlock();
}

void task_update_loop()
{
	gui_loop();
}


void task_update_irqh(uint8_t type, uint8_t * buff)
{
	gui_irqh(type, buff);
}
