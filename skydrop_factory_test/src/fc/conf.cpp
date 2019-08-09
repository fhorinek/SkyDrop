#include "conf.h"
#include "../gui/gui.h"

cfg_ro_t config_ro __attribute__ ((section(".cfg_ro")));

#define log_default_text	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

#define bt_link_key_blank	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
#define mac_invalid			{0, 0, 0, 0, 0, 0}

#define empty10				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
#define empty20				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
#define empty80				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

volatile cfg_t config = {
	//build_number
	BUILD_NUMBER,
	//gui
	{
		//contrast
		GUI_CONTRAST_STEPS / 2 + 1,
		//brightness
		20,
		//brightness_timeout
		10,
		//display_flags
		CFG_DISP_ANIM,
		//menu_audio_flags
		CFG_AUDIO_MENU_SPLASH | CFG_AUDIO_MENU_PAGES | CFG_AUDIO_MENU_BUTTONS | CFG_AUDIO_MENU_GPS,
		//menu_volume
		40,
		//vario_volume
		100,
		//vario_mute
		false,
		//alert_volume
		100,
	},
	//System
	{
		//debug_log
		false,//DEBUG_MAGIC_ON,
		//debug_gps
		false,
		//record_screen
		false,
	},
	//Connectivity
	{
		//usb_mode
		USB_MODE_NONE,
		//use_gps
		true,
		//gps_format_flags
		GPS_DDdddddd | GPS_SPD_KPH,
		//use_bt
		false,
		//bt_link_partner
		mac_invalid,
		//bt_link_key
		bt_link_key_blank,
		//btle_mac
		mac_invalid,
		//forward_gps
		false,
		//protocol
		PROTOCOL_LK8EX1,
		//uart_function
		UART_FORWARD_OFF,
	}
};

bool cfg_factory_passed()
{
	eeprom_busy_wait();
	return eeprom_read_byte(&config_ro.factory_passed) == CFG_FACTORY_PASSED_hex;
}

void cfg_reset_factory_test()
{
	uint8_t ff_buffer[sizeof(cfg_ro_t)];
	for (uint16_t i = 0; i < sizeof(cfg_ro_t); i++)
		ff_buffer[i] = 0xFF;

	eeprom_busy_wait();
	eeprom_update_block(ff_buffer, &config_ro, sizeof(cfg_ro_t));
	eeprom_busy_wait();
	task_set(TASK_POWERDOWN);
}

void cfg_acc_write_defaults()
{
	vector_i16_t tmp;
	//bias
	tmp.x = 45;
	tmp.y = 234;
	tmp.z = 66;

	eeprom_busy_wait();
	eeprom_write_block((void *)&tmp, (void *)&config_ro.calibration.acc_bias, sizeof(vector_i16_t));

	//sensitivity
	tmp.x = 8165;
	tmp.y = 8448;
	tmp.z = 8036;

	eeprom_busy_wait();
	eeprom_write_block((void *)&tmp, (void *)&config_ro.calibration.acc_sensitivity, sizeof(vector_i16_t));
}

void cfg_mag_write_defaults()
{
	vector_i16_t tmp;
	//bias
	tmp.x = 1916;
	tmp.y = -1542;
	tmp.z = 2252;

	eeprom_busy_wait();
	eeprom_write_block((void *)&tmp, (void *)&config_ro.calibration.mag_bias, sizeof(vector_i16_t));

	//sensitivity
	tmp.x = 5048;
	tmp.y = 5096;
	tmp.z = 4712;

	eeprom_busy_wait();
	eeprom_write_block((void *)&tmp, (void *)&config_ro.calibration.mag_sensitivity, sizeof(vector_i16_t));
}

void cfg_gyro_write_defaults()
{
	vector_i16_t tmp;
	//bias
	tmp.x = 1.1 * 10;
	tmp.y = 24.6 * 10;
	tmp.z = -44.8 * 10;

	eeprom_busy_wait();
	eeprom_write_block((void *)&tmp, (void *)&config_ro.gyro_bias, sizeof(vector_i16_t));
}

void cfg_compass_write_defaults()
{
	int16_t tmp = 0;
	eeprom_busy_wait();
	eeprom_update_block(&tmp, &config_ro.magnetic_declination, sizeof(config_ro.magnetic_declination));
}

void cfg_baro_write_defaults()
{
	int16_t tmp = 0;
	eeprom_busy_wait();
	eeprom_update_block(&tmp, &config_ro.baro_offset, sizeof(config_ro.baro_offset));
}


void cfg_load()
{
	//check and reload default calibration data (if needed)
	eeprom_busy_wait();
	uint8_t calib_flags = eeprom_read_byte(&config_ro.calibration_flags);

	if (calib_flags != CALIB_DEFAULT_LOADED)
	{
		cfg_acc_write_defaults();
		cfg_mag_write_defaults();
		cfg_gyro_write_defaults();
		cfg_compass_write_defaults();
		cfg_baro_write_defaults();

		eeprom_busy_wait();
		eeprom_update_byte(&config_ro.calibration_flags, CALIB_DEFAULT_LOADED);
	}
}
