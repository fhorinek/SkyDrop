#include "factory_test.h"

#include "../drivers/battery.h"
#include "../fc/fc.h"

#include "gui.h"
#include "splash.h"

uint8_t f_test_button_test;
uint8_t f_test_lcd;
uint8_t f_test_lcd_cont = 0;
uint8_t f_test_lcd_cont_min = 0;
uint8_t f_test_lcd_cont_max = 0;

void gui_factory_test_init()
{
	DEBUG(" *** Factory test ***\n");
//	fc_pause();
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


	disp.SetFlip(false);
	disp.SetInvert(false);
}

void gui_factory_test_stop()
{
	DEBUG("FACTORY TEST OK\n");
//	fc_continue();

	led_set(0x00, 0x00, 0x00);
	buzzer_set_vol(0);
}

void gui_factory_test_loop()
{
	gui_dialog_P(PSTR("Factory test"));

	bool blik = GUI_BLINK_TGL(1000);
	bool res;
	bool err = false;

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
		static bool self_test_done = false;
		static bool ms5611_self_test = false;
		static bool imu_self_test = false;
		static bool sht_self_test = false;

		if (!self_test_done)
		{
			fc_pause();
			mems_i2c.Wait();
			ms5611_self_test = ms5611.SelfTest();
			imu_self_test = lsm303d.SelfTest() && l3gd20.SelfTest();
			sht_self_test = sht21.SelfTest();
			fc_continue();
		}



		res = ms5611_self_test;
		if (!res) err = true;
		if (res || blik)
		{
			disp.GotoXY(4, f_h * 2 + 3);
			fprintf_P(lcd_out, PSTR("MS5611:%s"), (res) ? "OK" : "ERR");
//			assert(res);
		}

		res = imu_self_test;
		if (!res) err = true;
		if (res || blik)
		{
			disp.GotoXY(4, f_h * 3 + 3);
			fprintf_P(lcd_out, PSTR("IMU:%s"), (res) ? "OK" : "ERR");
//			assert(res);
		}


		if (imu_self_test)
		{
			disp.GotoXY(4, f_h * 4 + 3);
			fprintf_P(lcd_out, PSTR("ACC:%0.2f"), fc.acc.total_filtered);
//			assert(res);
		}

		res = sht_self_test;
//		if (!res) err = true;
		if (res || blik)
		{
			disp.GotoXY(4, f_h * 5 + 3);
			fprintf_P(lcd_out, PSTR("SHT21:%s"), (res) ? "OK" : "ERR");
//			assert(res);
		}
	}

	res = bt_ready();
	if (!res) err = true;
	if (res || blik)
	{
//		assert(res);
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
//		assert(res);
	}

	res = gps_selftest();
	if (!res) err = true;
	if (res || blik)
	{
		disp.GotoXY(GUI_DISP_WIDTH / 2, f_h * 3 + 3);
		fprintf_P(lcd_out, PSTR("GPS:%s"), (res) ? "OK" : "ERR");
//		assert(res);
	}

	res = f_test_button_test == 0b00000111;
	if (!res) err = true;
	if (res || blik)
	{
		disp.GotoXY(GUI_DISP_WIDTH / 2, f_h * 4 + 3);
		fprintf_P(lcd_out, PSTR("BUT:%d %d %d"), f_test_button_test & (1 << 0), (f_test_button_test & (1 << 1)) >> 1, (f_test_button_test & (1 << 2)) >> 2);
//		assert(res);
	}


	res = battery_adc_raw != 0;
	if (!res) err = true;
	if (res || blik)
	{
		disp.GotoXY(GUI_DISP_WIDTH / 2, f_h * 5 + 3);
		fprintf_P(lcd_out, PSTR("ADC:%d"), battery_adc_raw);
//		assert(res);
	}

	res = battery_per > 0;

	if (res || blik)
	{
		disp.GotoXY(GUI_DISP_WIDTH / 2, f_h * 6 + 3);
		fprintf_P(lcd_out, PSTR("BAT:%d%%"), battery_per);
//		assert(res);
	}


	if (!err)
	{
		eeprom_busy_wait();
		eeprom_update_byte(&config_ro.factory_passed, CFG_FACTORY_PASSED_hex);
		eeprom_update_byte(&config_ro.bt_module_type, bt_get_module_type());
		eeprom_busy_wait();

		gui_showmessage_P(PSTR("Factory test\nPASSED!"));
		gui_switch_task(GUI_SETTINGS);
	}

}

void gui_factory_test_irqh(uint8_t type, uint8_t * buff)
{
	if (type == TASK_IRQ_USB && *buff)
		task_set(TASK_USB);

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

			if (*buff == BE_LONG)
				gui_switch_task(GUI_SETTINGS);
		break;

		case (TASK_IRQ_BUTTON_R):
			f_test_button_test |= (1 << 2);
			led_set(0x00, 0x00, 0xFF);
			buzzer_set_vol(0);

		break;
	}

}

