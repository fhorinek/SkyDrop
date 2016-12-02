#include "factory_test.h"

#include "../drivers/battery.h"
#include "../fc/fc.h"

#include "splash.h"

#define FTEST_LCD_DONE		0
#define FTEST_LCD_MIN_AUTO	1
#define FTEST_LCD_MIN		2
#define FTEST_LCD_MAX_AUTO	3
#define FTEST_LCD_MAX		4
#define FTEST_LCD_MID		5

uint8_t f_test_button_test;
uint8_t f_test_lcd;
uint8_t f_test_lcd_cont = 0;
uint8_t f_test_lcd_cont_min = 0;
uint8_t f_test_lcd_cont_max = 0;

void gui_factory_test_init()
{
	DEBUG(" *** Factory test ***\n");
	fc_pause();
	led_notify_disable();

	//We need to test gps and bt module
	if (!bt_ready())
	{
		DEBUG("Force enable BT\n");
		bt_module_init();
	}

	if (!gps_selftest())
	{
		DEBUG("Force enable GPS\n");
		gps_start();
	}

	buzzer_set_vol(0);
	buzzer_set_freq(0);
	f_test_button_test = 0;

	f_test_lcd = FTEST_LCD_MIN_AUTO;

	disp.SetFlip(false);
	disp.SetInvert(false);
}

void gui_factory_test_stop()
{
	DEBUG("FACTORY TEST OK\n");
	fc_continue();

	led_set(0x00, 0x00, 0x00);
	buzzer_set_vol(0);
}

void gui_factory_test_loop()
{
	gui_dialog_P(PSTR("Factory test"));

	bool blik = GUI_BLINK_TGL(1000);
	bool res;
	bool err = false;

	if (f_test_lcd)
	{
		char tmp[16];

		disp.LoadFont(F_TEXT_L);
		switch (f_test_lcd)
		{
			case(FTEST_LCD_MIN_AUTO):
				sprintf_P(tmp, PSTR("Min auto"));
			break;
			case(FTEST_LCD_MIN):
				sprintf_P(tmp, PSTR("Set minimum"));
			break;
			case(FTEST_LCD_MAX_AUTO):
				sprintf_P(tmp, PSTR("Max auto"));
			break;
			case(FTEST_LCD_MAX):
				sprintf_P(tmp, PSTR("Set maximum"));
			break;
			case(FTEST_LCD_MID):
				sprintf_P(tmp, PSTR("Set optimal"));
			break;
		}
		gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, GUI_DIALOG_TOP);

		disp.DrawLine(4, 24, 14, 24, 1);
		disp.DrawLine(4, 26, 14, 26, 1);
		disp.DrawLine(4, 28, 14, 28, 1);
		disp.DrawLine(4, 30, 14, 30, 1);
		disp.DrawLine(4, 32, 14, 32, 1);

		disp.DrawLine(16, 24, 16, 32, 1);
		disp.DrawLine(18, 24, 18, 32, 1);
		disp.DrawLine(20, 24, 20, 32, 1);
		disp.DrawLine(22, 24, 22, 32, 1);
		disp.DrawLine(24, 24, 24, 32, 1);

		disp.DrawRectangle(26, 24, 35, 33, 1, 1);

		disp.DrawRectangle(37, 24, 46, 33, 1, 0);

		disp.DrawRectangle(48, 24, 57, 33, 1, 0);
		disp.DrawRectangle(50, 26, 55, 31, 1, 0);
		disp.DrawRectangle(52, 28, 53, 29, 1, 0);

		disp.DrawCircle(71, 35, 1, 1);
		disp.DrawCircle(71, 35, 3, 1);
		disp.DrawCircle(71, 35, 5, 1);
		disp.DrawCircle(71, 35, 7, 1);
		disp.DrawCircle(71, 35, 9, 1);

		disp.LoadFont(F_TEXT_M);
		disp.GotoXY(4, 36);
		fprintf_P(lcd_out, PSTR("%03d"), f_test_lcd_cont_min);
		disp.GotoXY(24, 36);
		fprintf_P(lcd_out, PSTR("%03d"), f_test_lcd_cont);
		disp.GotoXY(44, 36);
		fprintf_P(lcd_out, PSTR("%03d"), f_test_lcd_cont_max);

		if (f_test_lcd == FTEST_LCD_MIN_AUTO || f_test_lcd == FTEST_LCD_MAX_AUTO)
		{
			f_test_lcd_cont = (f_test_lcd_cont + 1) % 128;
		}

		led_set(0, f_test_lcd_cont / 4, 0);

//		lcd_contrast = f_test_lcd_cont;
//		gui_change_disp_cfg();

		if (f_test_lcd == FTEST_LCD_MID)
			disp.SetContrast(lcd_contrast_min + ((lcd_contrast_max - lcd_contrast_min) * f_test_lcd_cont) / GUI_CONTRAST_STEPS);
		else
			disp.SetContrast(f_test_lcd_cont);

		return;
	}

	disp.LoadFont(F_TEXT_S);
	uint8_t f_h = disp.GetTextHeight();

	if (!mems_i2c_selftest())
	{
		if (blik)
		{
			disp.GotoXY(4, f_h * 3.5 + 3);
			fprintf_P(lcd_out, PSTR("I2C ERROR"));
			assert(0);
		}
		err = true;
	}
	else
	{
		res = ms5611.SelfTest();
		if (!res) err = true;
		if (res || blik)
		{
			disp.GotoXY(4, f_h * 2 + 3);
			fprintf_P(lcd_out, PSTR("MS5611:%s"), (res) ? "OK" : "ERR");
			assert(res);
		}

		res = lsm303d.SelfTest();
		if (!res) err = true;
		if (res || blik)
		{
			disp.GotoXY(4, f_h * 3 + 3);
			fprintf_P(lcd_out, PSTR("LSM303:%s"), (res) ? "OK" : "ERR");
			assert(res);
		}

		res = l3gd20.SelfTest();
		if (!res) err = true;
		if (res || blik)
		{
			disp.GotoXY(4, f_h * 4 + 3);
			fprintf_P(lcd_out, PSTR("L3GD20:%s"), (res) ? "OK" : "ERR");
			assert(res);
		}

		res = sht21.SelfTest();
		if (!res) err = true;
		if (res || blik)
		{
			disp.GotoXY(4, f_h * 5 + 3);
			fprintf_P(lcd_out, PSTR("SHT21:%s"), (res) ? "OK" : "ERR");
			assert(res);
		}
	}

	res = bt_ready();
	if (!res) err = true;
	if (res || blik)
	{
		assert(res);
		disp.GotoXY(4, f_h * 6 + 3);
		if (!res)
		{
			fprintf_P(lcd_out, PSTR("BT:ERR"));
		}
		else
		{
			if (bt_get_module_type() == BT_PAN1322)
				fprintf_P(lcd_out, PSTR("BT:1322"));
			else
				fprintf_P(lcd_out, PSTR("BT:1026"));
		}
	}

	res = storage_ready();
	if (!res) err = true;
	if (res || blik)
	{
		disp.GotoXY(GUI_DISP_WIDTH / 2, f_h * 2 + 3);
		fprintf_P(lcd_out, PSTR("SD:%s"), (res) ? "OK" : "ERR");
		assert(res);
	}

	res = gps_selftest();
	if (!res) err = true;
	if (res || blik)
	{
		disp.GotoXY(GUI_DISP_WIDTH / 2, f_h * 3 + 3);
		fprintf_P(lcd_out, PSTR("GPS:%s"), (res) ? "OK" : "ERR");
		assert(res);
	}

	res = f_test_button_test == 0b00000111;
	if (!res) err = true;
	if (res || blik)
	{
		disp.GotoXY(GUI_DISP_WIDTH / 2, f_h * 4 + 3);
		fprintf_P(lcd_out, PSTR("BUT:%d %d %d"), f_test_button_test & (1 << 0), (f_test_button_test & (1 << 1)) >> 1, (f_test_button_test & (1 << 2)) >> 2);
		assert(res);
	}


	res = battery_adc_raw != 0;
	if (!res) err = true;
	if (res || blik)
	{
		disp.GotoXY(GUI_DISP_WIDTH / 2, f_h * 5 + 3);
		fprintf_P(lcd_out, PSTR("ADC:%d"), battery_adc_raw);
		assert(res);
	}

	res = battery_per > 0;

	if (res || blik)
	{
		disp.GotoXY(GUI_DISP_WIDTH / 2, f_h * 6 + 3);
		fprintf_P(lcd_out, PSTR("BAT:%d%%"), battery_per);
		assert(res);
	}


	if (!err)
	{
		eeprom_busy_wait();
		eeprom_update_byte(&config_ro.factory_passed, CFG_FACTORY_PASSED_hex);

		eeprom_update_byte(&config_ro.lcd_contrast_max, f_test_lcd_cont_max);
		eeprom_update_byte(&config_ro.lcd_contrast_min, f_test_lcd_cont_min);
		eeprom_update_byte(&config_ro.bt_module_type, bt_get_module_type());

		eeprom_update_byte(&config_ee.gui.contrast, f_test_lcd_cont);
		eeprom_busy_wait();

		task_set(TASK_POWERDOWN);
	}

}

void gui_factory_test_irqh(uint8_t type, uint8_t * buff)
{
	if (f_test_lcd)
	{
		if (*buff == BE_CLICK || *buff == BE_DBL_CLICK)
		switch (type)
		{
			case (TASK_IRQ_BUTTON_L):
				if (f_test_lcd == FTEST_LCD_MIN_AUTO)
					f_test_lcd = FTEST_LCD_MIN;
				if (f_test_lcd == FTEST_LCD_MAX_AUTO)
					f_test_lcd = FTEST_LCD_MAX;

				if (f_test_lcd == FTEST_LCD_MID)
					f_test_lcd_cont = (f_test_lcd_cont - 1) % GUI_CONTRAST_STEPS;
				else
					f_test_lcd_cont = (f_test_lcd_cont - 1) % 128;
			break;
			case (TASK_IRQ_BUTTON_M):
				switch (f_test_lcd)
				{
					case(FTEST_LCD_MIN_AUTO):
						f_test_lcd = FTEST_LCD_MIN;
					break;
					case(FTEST_LCD_MIN):
						f_test_lcd = FTEST_LCD_MAX_AUTO;
						f_test_lcd_cont_min = f_test_lcd_cont;
					break;
					case(FTEST_LCD_MAX_AUTO):
						f_test_lcd = FTEST_LCD_MAX;
					break;
					case(FTEST_LCD_MAX):
						f_test_lcd = FTEST_LCD_MID;
						f_test_lcd_cont_max = f_test_lcd_cont;
						f_test_lcd_cont = GUI_CONTRAST_STEPS / 2;
					break;
					case(FTEST_LCD_MID):
						f_test_lcd = FTEST_LCD_DONE;
						led_set(0,0,0);
					break;
				}
			break;
			case (TASK_IRQ_BUTTON_R):
				if (f_test_lcd == FTEST_LCD_MIN_AUTO)
					f_test_lcd = FTEST_LCD_MIN;
				if (f_test_lcd == FTEST_LCD_MAX_AUTO)
					f_test_lcd = FTEST_LCD_MAX;

				if (f_test_lcd == FTEST_LCD_MID)
					f_test_lcd_cont = (f_test_lcd_cont + 1) % GUI_CONTRAST_STEPS;
				else
					f_test_lcd_cont = (f_test_lcd_cont + 1) % 128;
			break;
		}
	}
	else
	{
		switch (type)
		{
			case (TASK_IRQ_BUTTON_L):
				f_test_button_test |= (1 << 0);
				led_set(0xFF, 0x00, 0x00);
				buzzer_set_vol(100);
				buzzer_set_freq(200);
			break;
			case (TASK_IRQ_BUTTON_M):
				f_test_button_test |= (1 << 1);
				led_set(0x00, 0xFF, 0x00);
				buzzer_set_vol(100);
				buzzer_set_freq(300);
			break;
			case (TASK_IRQ_BUTTON_R):
				f_test_button_test |= (1 << 2);
				led_set(0x00, 0x00, 0xFF);
				buzzer_set_vol(0);
			break;
		}
	}
}
