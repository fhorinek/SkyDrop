#include "gui.h"

#include "../drivers/audio/sequencer.h"
#include "../drivers/bluetooth/bt.h"
#include "gui_storage.h"

#include "widgets/widgets.h"

#include "pages.h"
#include "splash.h"
#include "settings/settings.h"
#include "settings/set_vario.h"
#include "settings/set_audio.h"
#include "gui_value.h"
#include "settings/set_widgets.h"
#include "settings/layouts.h"
#include "settings/set_layout.h"
#include "settings/set_display.h"
#include "usb.h"
#include "factory_test.h"
#include "settings/set_system.h"
#include "settings/set_autostart.h"
#include "settings/set_gps.h"
#include "settings/set_gps_detail.h"
#include "settings/set_debug.h"
#include "settings/set_altimeters.h"
#include "settings/set_altimeter.h"
#include "settings/set_time.h"
#include "settings/set_logger.h"
#include "gui_dialog.h"
#include "settings/set_bluetooth.h"
#include "update.h"
#include "settings/set_weaklift.h"
#include "settings/set_menu_audio.h"
#include "gui_text.h"
#include "settings/set_advanced.h"
#include "settings/set_calib.h"
#include "settings/gui_accel_calib.h"
#include "settings/gui_mag_calib.h"


lcd_display disp;
CreateStdOut(lcd_out, disp.Write);

volatile uint8_t gui_task = GUI_NONE;
volatile uint8_t gui_new_task = GUI_SPLASH;

Spi gui_disp_spi;

void (* gui_init_array[])() =
	{gui_pages_init, gui_settings_init, gui_splash_init, gui_set_vario_init, gui_value_init,
	gui_set_audio_init, gui_set_widgets_init, gui_layouts_init, gui_set_layout_init,
	gui_set_display_init, gui_usb_init, gui_factory_test_init, gui_set_system_init,
	gui_set_autostart_init, gui_set_gps_init, gui_set_gps_detail_init, gui_set_debug_init,
	gui_set_altimeters_init, gui_set_altimeter_init, gui_set_time_init, gui_set_logger_init,
	gui_dialog_init, gui_set_bluetooth_init, gui_update_init, gui_set_weaklift_init,
	gui_set_menu_audio_init, gui_text_init, gui_set_advanced_init, gui_set_calib_init,
	gui_accelerometer_calib_init, gui_mag_calib_init };

void (* gui_stop_array[])() =
	{gui_pages_stop, gui_settings_stop, gui_splash_stop, gui_set_vario_stop, gui_value_stop,
	gui_set_audio_stop, gui_set_widgets_stop, gui_layouts_stop, gui_set_layout_stop,
	gui_set_display_stop, gui_usb_stop, gui_factory_test_stop, gui_set_system_stop,
	gui_set_autostart_stop, gui_set_gps_stop, gui_set_gps_detail_stop, gui_set_debug_stop,
	gui_set_altimeters_stop, gui_set_altimeter_stop, gui_set_time_stop, gui_set_logger_stop,
	gui_dialog_stop, gui_set_bluetooth_stop, gui_update_stop, gui_set_weaklift_stop,
	gui_set_menu_audio_stop, gui_text_stop, gui_set_advanced_stop, gui_set_calib_stop,
	gui_accelerometer_calib_stop, gui_mag_calib_stop };

void (* gui_loop_array[])() =
	{gui_pages_loop, gui_settings_loop, gui_splash_loop, gui_set_vario_loop, gui_value_loop,
	gui_set_audio_loop, gui_set_widgets_loop, gui_layouts_loop, gui_set_layout_loop,
	gui_set_display_loop, gui_usb_loop, gui_factory_test_loop, gui_set_system_loop,
	gui_set_autostart_loop, gui_set_gps_loop, gui_set_gps_detail_loop, gui_set_debug_loop,
	gui_set_altimeters_loop, gui_set_altimeter_loop, gui_set_time_loop, gui_set_logger_loop,
	gui_dialog_loop, gui_set_bluetooth_loop, gui_update_loop, gui_set_weaklift_loop,
	gui_set_menu_audio_loop, gui_text_loop, gui_set_advanced_loop, gui_set_calib_loop,
	gui_accelerometer_calib_loop, gui_mag_calib_loop };

void (* gui_irqh_array[])(uint8_t type, uint8_t * buff) =
	{gui_pages_irqh, gui_settings_irqh, gui_splash_irqh, gui_set_vario_irqh, gui_value_irqh,
	gui_set_audio_irqh, gui_set_widgets_irqh, gui_layouts_irqh, gui_set_layout_irqh,
	gui_set_display_irqh, gui_usb_irqh, gui_factory_test_irqh, gui_set_system_irqh,
	gui_set_autostart_irqh, gui_set_gps_irqh, gui_set_gps_detail_irqh, gui_set_debug_irqh,
	gui_set_altimeters_irqh, gui_set_altimeter_irqh, gui_set_time_irqh, gui_set_logger_irqh,
	gui_dialog_irqh, gui_set_bluetooth_irqh, gui_update_irqh, gui_set_weaklift_irqh,
	gui_set_menu_audio_irqh, gui_text_irqh, gui_set_advanced_irqh, gui_set_calib_irqh,
	gui_accelerometer_calib_irqh, gui_mag_calib_irqh };

#define GUI_ANIM_STEPS	20

/**
 * This are 14 6x8 images giving a GPS animation. There is no width/height at the beginning, only bit map.
 */
#define IMG_GPS_WIDTH 6
#define IMG_GPS_HEIGHT 8
#define IMG_GPS_ON_NO 14

const uint8_t PROGMEM img_gps_on[] = {
		0x38, 0x00, 0x18, 0x18, 0x00, 0x00,
		0x18, 0x04, 0x18, 0x18, 0x00, 0x00,
		0x08, 0x04, 0x1A, 0x18, 0x00, 0x00,
		0x00, 0x04, 0x1A, 0x1A, 0x00, 0x00,
		0x00, 0x00, 0x1A, 0x1A, 0x02, 0x00,
		0x00, 0x00, 0x18, 0x1A, 0x02, 0x04,
		0x00, 0x00, 0x18, 0x18, 0x02, 0x0C,
		0x00, 0x00, 0x18, 0x18, 0x00, 0x1C,
		0x00, 0x00, 0x18, 0x18, 0x20, 0x18,
		0x00, 0x00, 0x18, 0x58, 0x20, 0x10,
		0x00, 0x00, 0x58, 0x58, 0x20, 0x00,
		0x00, 0x40, 0x58, 0x58, 0x00, 0x00,
		0x20, 0x40, 0x58, 0x18, 0x00, 0x00,
		0x30, 0x40, 0x18, 0x18, 0x00, 0x00
};

#define IMG_GPS_OFF_NO 6

const uint8_t PROGMEM img_gps_off[] = {
		0x00, 0x00, 0x10, 0x08, 0x00, 0x00,
		0x00, 0x00, 0x18, 0x18, 0x00, 0x00,
		0x00, 0x18, 0x3C, 0x3C, 0x18, 0x00,
		0x18, 0x3C, 0x7E, 0x7E, 0x3C, 0x18,
		0x00, 0x18, 0x3C, 0x3C, 0x18, 0x00,
		0x00, 0x00, 0x10, 0x08, 0x00, 0x00
};

const uint8_t PROGMEM img_bluetooth[] = {
		6, 8, // width, heigth
		0x22, 0x14, 0x7F, 0x2A, 0x14, 0x00
};

const uint8_t PROGMEM img_logger[] = {
		6, 8, // width, heigth
		0xFF, 0x81, 0xBD, 0xA1, 0x82, 0xFC
};

const uint8_t PROGMEM img_logger_error[] = {
		6, 8, // width, heigth
		0xFF, 0xC5, 0xA9, 0x91, 0xAA, 0xFC
};

const uint8_t PROGMEM img_debug[] = {
		6, 8, // width, heigth
		0x93, 0x54, 0x3D, 0x3D, 0x54, 0x93
};

MK_SEQ(snd_but_short, ARR({1000}), ARR({50}));
MK_SEQ(snd_but_long, ARR({800}), ARR({200}));

uint32_t gui_idle_timer;
#define GUI_IDLE_RETURN			30000

void gui_switch_task(uint8_t new_task)
{
	gui_new_task = new_task;
}

void gui_reset_timeout()
{
	gui_idle_timer = task_get_ms_tick();
}

uint32_t lcd_brightness_end = 0;

uint8_t lcd_contrast_min;
uint8_t lcd_contrast_max;

volatile bool gui_buttons_override = false;

volatile bool lcd_new_cfg = false;

void gui_trigger_backlight()
{
	if (config.gui.brightness == 0 || config.gui.brightness_timeout == 0)
		lcd_bckl(0);
	else
	{
		lcd_bckl(config.gui.brightness);
		lcd_brightness_end = task_get_ms_tick() + config.gui.brightness_timeout * 1000;
	}
}

void gui_change_disp_cfg()
{
	lcd_new_cfg = true;
}

char gui_message_line1[20];
char gui_message_line2[20];
char gui_message_line3[20];
uint32_t gui_message_end = 0;
#define MESSAGE_DURATION	5
#define MESSAGE_FORCE_ON		0xFFFFFFFF

void gui_showmessage_P(const char * msg)
{
	char tmp[63];
	strcpy_P(tmp, msg);
	gui_showmessage(tmp);
}

void gui_showmessage(char * msg)
{
	char * ptr;
	if ((ptr = strchr(msg, '\n')) != NULL)
	{
		memcpy(gui_message_line1, msg, ptr - msg + 1);
		gui_message_line1[ptr - msg] = 0;
		msg = ptr + 1;
		if ((ptr = strchr(msg, '\n')) != NULL)
		{
			memcpy(gui_message_line2, msg, ptr - msg + 1);
			gui_message_line2[ptr - msg] = 0;
			strcpy(gui_message_line3, ptr + 1);
		}
		else
		{
			strcpy(gui_message_line2, msg);
			gui_message_line3[0] = 0;
		}
	}
	else
	{
		strcpy(gui_message_line1, msg);
		gui_message_line2[0] = 0;
		gui_message_line3[0] = 0;
	}
	gui_message_end = task_get_ms_tick() + MESSAGE_DURATION * 1000ul;
}

void gui_forcemessage()
{
	gui_message_end = MESSAGE_FORCE_ON;
}

void gui_hidemessage()
{
	gui_message_end = 0;
}

void gui_raligh_text_P(const char * text, uint8_t x, uint8_t y)
{
	char tmp[16];
	strcpy_P(tmp, text);
	gui_raligh_text(tmp, x, y);
}

void gui_raligh_text(char * text, uint8_t x, uint8_t y)
{
	if (x >= disp.GetTextWidth(text))
		disp.GotoXY(x - disp.GetTextWidth(text), y);
	else
		disp.GotoXY(0, y);
	fprintf_P(lcd_out, PSTR("%s"), text);
}

void gui_caligh_text_P(const char * text, uint8_t x, uint8_t y)
{
	char tmp[16];
	strcpy_P(tmp, text);
	gui_caligh_text(tmp, x, y);
}

void gui_caligh_text(char * text, uint8_t x, uint8_t y)
{
	disp.GotoXY(x - disp.GetTextWidth(text) / 2, y);
	fprintf_P(lcd_out, PSTR("%s"), text);
}

void gui_fit_text(char * in, char * out, uint8_t size)
{
	uint8_t width = disp.GetTextWidth(in);

	if (width <= size)
	{
		strcpy(out, in);
		return;
	}

	char tmp[4];

	tmp[0] = '.';
	tmp[1] = '.';
	tmp[2] = '.';
	tmp[3] = 0;

	uint8_t tmp_len = disp.GetTextWidth(tmp);
	uint8_t len = strlen(in);
	uint8_t i = len - 1;

	while ((width + tmp_len) - disp.GetTextWidth(in + i) > size)
	{
		i--;
	}

	memcpy(out, in, i);
	strcpy(out + i, tmp);
}

void gui_init()
{
	LCD_SPI_PWR_ON;

	gui_disp_spi.InitMaster(LCD_SPI);
	gui_disp_spi.SetDivider(spi_div_64);
	gui_disp_spi.SetDataOrder(MSB);

	disp.Init(&gui_disp_spi);
	gui_load_eeprom();
	gui_idle_timer = task_get_ms_tick();
}

void gui_load_eeprom()
{
	eeprom_busy_wait();

	if (config.gui.brightness == 0xFF)
		config.gui.brightness = 100;

	if (config.gui.brightness_timeout == 0xFF)
		config.gui.brightness_timeout = 3;

	if (config.gui.contrast > GUI_CONTRAST_STEPS)
		config.gui.contrast = GUI_CONTRAST_STEPS / 2;

	lcd_contrast_min = eeprom_read_byte(&config_ro.lcd_contrast_min);
	if (lcd_contrast_min == 0xFF)
		lcd_contrast_min = 16;

	lcd_contrast_max = eeprom_read_byte(&config_ro.lcd_contrast_max);
	if (lcd_contrast_max == 0xFF)
		lcd_contrast_max = 127;

//	DEBUG("lcd_contrast_min %d\n", lcd_contrast_min);
//	DEBUG("lcd_contrast_max %d\n", lcd_contrast_max);
//	DEBUG("lcd_contrast %d\n", config.gui.contrast);

	if (config.gui.disp_flags == 0xFF)
		config.gui.disp_flags = CFG_DISP_ANIM;

	disp.SetFlip(config.gui.disp_flags & CFG_DISP_FLIP);
	gui_change_disp_cfg();
	gui_update_disp_cfg();
}

void gui_stop()
{
	disp.Stop();

	gui_disp_spi.Stop();

	LCD_SPI_PWR_OFF;
}

uint8_t fps_counter = 0;
uint8_t fps_val = 0;
uint32_t fps_timer = 0;

uint16_t gui_record_cnt;

uint32_t gui_loop_timer = 0;

void gui_dialog_P(const char * title)
{
	char tmp[16];
	strcpy_P(tmp, title);
	gui_dialog(tmp);
}

void gui_dialog(char * title)
{
	disp.LoadFont(F_TEXT_M);
	gui_caligh_text(title, GUI_DISP_WIDTH / 2, 2);

	disp.InvertPart(0, 0, GUI_DISP_WIDTH, 0);
	disp.PutPixel(0, 0, 0);
	disp.PutPixel(GUI_DISP_WIDTH - 1, 0 ,0);
	disp.Invert(0, 8, GUI_DISP_WIDTH, 11);

	disp.DrawLine(0, 12, 0, GUI_DISP_HEIGHT - 2, 1);
	disp.DrawLine(GUI_DISP_WIDTH - 1, 12, GUI_DISP_WIDTH - 1, GUI_DISP_HEIGHT - 2, 1);
	disp.DrawLine(1, GUI_DISP_HEIGHT - 1, GUI_DISP_WIDTH - 2, GUI_DISP_HEIGHT - 1, 1);
}

void gui_update_disp_cfg()
{
	if (lcd_new_cfg)
	{
		//need to be outside of IRQ
		lcd_new_cfg = false;

//		DEBUG(" ** gui_update_disp_cfg **\n");
//		DEBUG("lcd_contrast %d\n", config.gui.contrast);


		uint8_t new_contrast = lcd_contrast_min + ((lcd_contrast_max - lcd_contrast_min) * config.gui.contrast) / GUI_CONTRAST_STEPS;

//		DEBUG("new_contrast %d\n", new_contrast);

		disp.SetContrast(new_contrast);
		disp.SetInvert(config.gui.disp_flags & CFG_DISP_INVERT);
	}
}

//disp layers
// 0 - layer to draw
// 1 - help layer
void gui_loop()
{
	if (gui_loop_timer > task_get_ms_tick())
		return;

//	gui_loop_timer = (uint32_t)task_get_ms_tick() + (uint32_t)50; //20 fps
	gui_loop_timer = (uint32_t)task_get_ms_tick() + (uint32_t)33; //30 fps

	gui_update_disp_cfg();

	//timeout tasks
	if (actual_task == TASK_ACTIVE)
	{
		//return to main gui task, except for GPS detail page, dialog or splash
		if (gui_task != GUI_PAGES
				&& gui_task != GUI_SET_GPS_DETAIL
				&& gui_task != GUI_DIALOG
				&& gui_task != GUI_SPLASH
				&& gui_task != GUI_FTEST
				&& gui_task != GUI_SET_VAL
				&& gui_task != GUI_UPDATE
				&& gui_task != GUI_TEXT
				&& gui_task != GUI_SET_CALIB_ACC
				&& gui_task != GUI_SET_CALIB_MAG
				)
		{
			if (task_get_ms_tick() - gui_idle_timer > GUI_IDLE_RETURN)
				gui_switch_task(GUI_PAGES);
		}

		//no activity for a long time -> power off
		if (gui_task == GUI_PAGES)
		{
			//Power off if not in flight, auto power off is enabled and bluetooth device is not connected
			if (fc.flight.state != FLIGHT_FLIGHT && config.system.auto_power_off > 0 && !bt_device_active())
			{
				if (task_get_ms_tick() - gui_idle_timer > (uint32_t)config.system.auto_power_off * 60ul * 1000ul)
				{
					gui_page_power_off();
				}
			}
		}
	}

	if (gui_new_task != gui_task)
	{
		//DEBUG("switching tasks %d %d\n", gui_new_task, gui_task);

		if (gui_task != GUI_NONE)
			gui_stop_array[gui_task]();

		//DEBUG("1 %6X\n", (uint32_t)gui_stop_array[gui_task]);

		gui_task = gui_new_task;

		buttons_reset();
		gui_init_array[gui_task]();
		//DEBUG("2 %6X\n", (uint32_t)gui_init_array[gui_task]);
	}

	disp.ClearBuffer();

	gui_loop_array[gui_task]();

	//display message pop-up
	if (gui_message_end > task_get_ms_tick())
	{
		disp.LoadFont(F_TEXT_M);
		uint8_t w = disp.GetTextWidth(gui_message_line1);
		uint8_t h = disp.GetAHeight();

		if (gui_message_line2[0] != 0)
		{
			uint8_t tmp = disp.GetTextWidth(gui_message_line2);
			if (tmp > w)
				w = tmp;
			h += disp.GetTextHeight();
		}

		if (gui_message_line3[0] != 0)
		{
			uint8_t tmp = disp.GetTextWidth(gui_message_line3);
			if (tmp > w)
				w = tmp;
			h += disp.GetTextHeight();
		}

		uint8_t x = GUI_DISP_WIDTH / 2 - w / 2;
		uint8_t y = GUI_DISP_HEIGHT / 2 - h / 2;

		uint8_t pad = 3;
		disp.DrawRectangle(x - 1 - pad, y - 1 - pad, x + w + pad, y + h + pad, 0, 1);

		disp.DrawLine(x - pad,			y - 2 - pad, 		x + w + pad, 		y - 2 - pad, 		1);
		disp.DrawLine(x - pad, 			y + h + 1 + pad, 	x + w + pad, 		y + h + 1 + pad, 	1);
		disp.DrawLine(x - 1 - pad, 		y - 1 - pad, 		x - 1 - pad, 		y + h + pad, 	1);
		disp.DrawLine(x + w + 1 + pad, 	y - 1 - pad, 		x + w + 1 + pad, 	y + h + pad, 	1);

		disp.GotoXY(x, y);
		fprintf_P(lcd_out, PSTR("%s"), gui_message_line1);

		if (gui_message_line2[0] != 0)
		{
			disp.GotoXY(x, y + disp.GetTextHeight());
			fprintf_P(lcd_out, PSTR("%s"), gui_message_line2);
		}

		if (gui_message_line3[0] != 0)
		{
			disp.GotoXY(x, y + disp.GetTextHeight() * 2);
			fprintf_P(lcd_out, PSTR("%s"), gui_message_line3);
		}
	}

	// FPS counter

//	fps_counter++;
//
//	if (fps_timer < task_get_ms_tick())
//	{
//		fps_val = fps_counter;
//		fps_counter = 0;
//		fps_timer = task_get_ms_tick() + 1000;
//	}
//
//	disp.LoadFont(font_6px_normal_ttf_8);
//	disp.GotoXY(1, 1);
//	disp.ClearPart(0, 0, 1, 10);
//	fprintf_P(lcd_out, PSTR("%d"), fps_val);

	// FPS end
	disp.Draw();

	if (config.system.record_screen && storage_ready())
	{
		FIL fimg;
		UINT wb;
		char fname[32];

		sprintf_P(fname, PSTR("/REC/%08d"), gui_record_cnt);

		uint8_t res = f_open(&fimg, fname, FA_WRITE | FA_CREATE_ALWAYS);
		DEBUG("rec = %02X\n", res);

		if (res == FR_OK)
		{
			f_write(&fimg, disp.GetActiveLayerPtr(), lcb_layer_size, &wb);
			f_close(&fimg);

			gui_record_cnt++;
		}
	}


	if (gui_task != GUI_SPLASH)
	{
		if (buttons_read(B_LEFT) || buttons_read(B_RIGHT) || buttons_read(B_MIDDLE))
		{
			gui_trigger_backlight();
			gui_reset_timeout();
		}
	}

	if (lcd_brightness_end < task_get_ms_tick())
		lcd_bckl(0);
}

void gui_force_loop()
{
	gui_loop_timer = 0;
	gui_loop();
}

void gui_irqh(uint8_t type, uint8_t * buff)
{
	if (type == B_LEFT || type == B_MIDDLE || type == B_RIGHT)
	{
		if ( (config.gui.menu_audio_flags & CFG_AUDIO_MENU_BUTTONS) && gui_buttons_override == false)
		{
			if (*buff == BE_CLICK)
				seq_start(&snd_but_short, config.gui.menu_volume);
			if (*buff == BE_LONG)
				seq_start(&snd_but_long, config.gui.menu_volume);
		}

		if (gui_message_end > task_get_ms_tick())
		{
			if (type == B_MIDDLE && (*buff == BE_CLICK || *buff == BE_LONG))
				if (gui_message_end != MESSAGE_FORCE_ON)
					gui_hidemessage();

			return;
		}
	}

	switch(type)
	{
		case(TASK_IRQ_MOUNT_ERROR):
			gui_storage_mount_error();
		break;

		default:
			if (gui_task != GUI_NONE)
				gui_irqh_array[gui_task](type, buff);
	}
}

void gui_statusbar()
{
	//GPS indicator
	if (config.connectivity.use_gps)
	{
		int index;

		if(fc.gps_data.valid)
		{
			index = (task_get_ms_tick() % 1000) / (1000 / (IMG_GPS_ON_NO - 1));
		    disp.DrawImage(img_gps_on + index * 6, GUI_DISP_WIDTH - IMG_GPS_WIDTH, 1, IMG_GPS_WIDTH, IMG_GPS_HEIGHT);
		}
		else
		{
			if (GUI_BLINK_TGL(2000)) {
				index = (task_get_ms_tick() % 1000) / (1000 / (IMG_GPS_OFF_NO - 1));
				disp.DrawImage(img_gps_off + index * 6, GUI_DISP_WIDTH - IMG_GPS_WIDTH, 1, IMG_GPS_WIDTH, IMG_GPS_HEIGHT);
			}
		}
	}

	//BT indicator
	if (bt_ready())
	{
		if (bt_device_active())
		{
			disp.DrawImage(img_bluetooth, GUI_DISP_WIDTH - 6, 9);
		}
		else
		{
			if (GUI_BLINK_TGL(1000))
				disp.DrawImage(img_bluetooth, GUI_DISP_WIDTH - 6, 9);
		}
	}

	//LOG indicator
	if (fc.logger_state != LOGGER_IDLE)
	{

		if (fc.logger_state == LOGGER_ACTIVE)
		{
			disp.DrawImage(img_logger, GUI_DISP_WIDTH - 6, 17);
		}
		else if (fc.logger_state == LOGGER_WAIT_FOR_GPS)
		{
			if (GUI_BLINK_TGL(1000))
				disp.DrawImage(img_logger, GUI_DISP_WIDTH - 6, 17);
		}
		else if (fc.logger_state == LOGGER_ERROR)
		{
			if (GUI_BLINK_TGL(500))
				disp.DrawImage(img_logger_error, GUI_DISP_WIDTH - 6, 17);
		}
	}

	//Debug.log indicator
	if (config.system.debug_log == DEBUG_MAGIC_ON)
	{
		disp.DrawImage(img_debug, GUI_DISP_WIDTH - 6, 26);
	}

	//battery indicator
	uint8_t a = battery_per / 10;

	if (battery_per == BATTERY_CHARGING)
	{
		a = 1 + (task_get_ms_tick() % 2000) / 200;
	}

	if (battery_per == BATTERY_FULL)
		a = 10;

	disp.DrawLine(GUI_DISP_WIDTH - 5, GUI_DISP_HEIGHT - 13, GUI_DISP_WIDTH - 2, GUI_DISP_HEIGHT - 13, 1);
	disp.DrawRectangle(GUI_DISP_WIDTH - 6, GUI_DISP_HEIGHT - 12, GUI_DISP_WIDTH - 1, GUI_DISP_HEIGHT - 1, 1, 0);
	disp.DrawRectangle(GUI_DISP_WIDTH - 5, GUI_DISP_HEIGHT - 1 - a, GUI_DISP_WIDTH - 2, GUI_DISP_HEIGHT - 1, 1, 1);
}
