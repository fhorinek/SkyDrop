#include "task_active.h"
#include "../gui/gui.h"
#include "../fc/fc.h"
#include "../gui/splash.h"

#define CONTRAST_MIN	70
#define CONTRAST_MAX	110

void task_active_init()
{
	DEBUG(" *** THIS IS TASK ACTIVE ***\n");

	//init gui
	gui_init();
	gui_trigger_backlight();
	ewdt_reset();

	gui_splash_set_mode(SPLASH_ON);
	gui_switch_task(GUI_SPLASH);

	ewdt_reset();
	if (storage_init())
	{
		task_special_files_handle();
	}

	ewdt_reset();

	if (!cfg_factory_passed())
	{
		gui_switch_task(GUI_FTEST);

		//store contrast values
		eeprom_busy_wait();
		eeprom_update_byte(&config_ro.lcd_contrast_min, CONTRAST_MIN);
		eeprom_update_byte(&config_ro.lcd_contrast_max, CONTRAST_MAX);
		eeprom_busy_wait();

		gui_load_eeprom();
	}


	//init flight computer
	fc_init();

	led_notify_enable();
}

void task_special_files_handle()
{
	//Handle update files
	FILINFO fno;

	//new way to update FW if SKYDROP.FW file found
	if (f_stat("SKYDROP.FW", &fno) == FR_OK)
	{
		task_set(TASK_UPDATE);
		return;
	}

	if (f_stat("SET_CONT", &fno) == FR_OK)
	{
		//store contrast values
		eeprom_busy_wait();
		eeprom_update_byte(&config_ro.lcd_contrast_min, CONTRAST_MIN);
		eeprom_update_byte(&config_ro.lcd_contrast_max, CONTRAST_MAX);
		eeprom_busy_wait();

		gui_load_eeprom();
	}

	//preserve EE and FW file if NO_WIPE file found (factory programming)
	if (f_stat("NO_WIPE", &fno) != FR_OK)
	{
		//remove applied update files
		f_unlink("UPDATE.EE");
		f_unlink("UPDATE.FW");
	}
}

void task_active_stop()
{
	led_notify_disable();

	fc_deinit();

	audio_off();

	gui_stop();

	debug_end();

	storage_deinit();
}

void task_active_loop()
{
	fc_step();

	gui_loop();

	storage_step();

	debug_step();
}

void task_active_irqh(uint8_t type, uint8_t * buff)
{
	switch (type)
	{
	case(TASK_IRQ_USB):
		if (*buff && config.connectivity.usb_mode == USB_MODE_MASSSTORAGE)
			task_set(TASK_USB);
		break;

	default:
		gui_irqh(type, buff);
	}
}
