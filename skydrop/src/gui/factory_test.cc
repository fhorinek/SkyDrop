#include "factory_test.h"

#include "../drivers/battery.h"
#include "../fc/fc.h"

#include "splash.h"

uint8_t f_test_button_test;


void gui_factory_test_init()
{
	fc_pause();

	buzzer_set_freq(0);
	buzzer_set_vol(0);
	f_test_button_test = 0;


	mems_i2c.StartTransmittion(0, 0);
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
	gui_dialog("Factory test");

	disp.LoadFont(F_TEXT_S);
	uint8_t f_h = disp.GetTextHeight();

	bool blik = (task_get_ms_tick() % 1000 < 500) ? true : false;
	bool res;
	bool err = false;

	if (!mems_i2c_selftest())
	{
		if (blik)
		{
			disp.GotoXY(4, f_h * 3.5 + 3);
			fprintf_P(lcd_out, PSTR("I2C ERROR"));
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
		}

		res = lsm303d.SelfTest();
		if (!res) err = true;
		if (res || blik)
		{
			disp.GotoXY(4, f_h * 3 + 3);
			fprintf_P(lcd_out, PSTR("LSM303:%s"), (res) ? "OK" : "ERR");
		}

		res = l3gd20.SelfTest();
		if (!res) err = true;
		if (res || blik)
		{
			disp.GotoXY(4, f_h * 4 + 3);
			fprintf_P(lcd_out, PSTR("L3GD20:%s"), (res) ? "OK" : "ERR");
		}

		res = sht21.SelfTest();
		if (!res) err = true;
		if (res || blik)
		{
			disp.GotoXY(4, f_h * 5 + 3);
			fprintf_P(lcd_out, PSTR("SHT21:%s"), (res) ? "OK" : "ERR");
		}
	}

	res = bt_selftest();
	if (!res) err = true;
	if (res || blik)
	{
		disp.GotoXY(4, f_h * 6 + 3);
		fprintf_P(lcd_out, PSTR("BT:%s"), (res) ? "OK" : "ERR");
	}

	res = storage_selftest();
	if (!res) err = true;
	if (res || blik)
	{
		disp.GotoXY(GUI_DISP_WIDTH / 2, f_h * 2 + 3);
		fprintf_P(lcd_out, PSTR("SD:%s"), (res) ? "OK" : "ERR");
	}

	res = gps_selftest();
	if (!res) err = true;
	if (res || blik)
	{
		disp.GotoXY(GUI_DISP_WIDTH / 2, f_h * 3 + 3);
		fprintf_P(lcd_out, PSTR("GPS:%s"), (res) ? "OK" : "ERR");
	}

	res = f_test_button_test == 0b00000111;
	if (!res) err = true;
	if (res || blik)
	{
		disp.GotoXY(GUI_DISP_WIDTH / 2, f_h * 4 + 3);
		fprintf_P(lcd_out, PSTR("BUT:%d %d %d"), f_test_button_test & (1 << 0), (f_test_button_test & (1 << 1)) >> 1, (f_test_button_test & (1 << 2)) >> 2);
	}


	res = battery_adc_raw != 0;
	if (!res) err = true;
	if (res || blik)
	{
		disp.GotoXY(GUI_DISP_WIDTH / 2, f_h * 5 + 3);
		fprintf_P(lcd_out, PSTR("ADC:%d"), battery_adc_raw);
	}

	res = battery_per > 0;
//	if (!res) err = true;
	if (res || blik)
	{
		disp.GotoXY(GUI_DISP_WIDTH / 2, f_h * 6 + 3);
		fprintf_P(lcd_out, PSTR("BAT:%d%%"), battery_per);
	}


	if (!err)
	{
		gui_splash_set_mode(SPLASH_ON);
		gui_switch_task(GUI_SPLASH);

		eeprom_busy_wait();
		eeprom_update_byte(&ee_fw_info.test_pass, APP_INFO_TEST_hex);
	}

}

void gui_factory_test_irqh(uint8_t type, uint8_t * buff)
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
