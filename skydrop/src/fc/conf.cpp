#include "conf.h"
#include "../gui/widgets/layouts/layouts.h"

cfg_ro_t config_ro __attribute__ ((section(".cfg_ro")));

#define log_default_text	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

#define bt_link_key_blank	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
#define mac_invalid			{0, 0, 0, 0, 0, 0}

volatile cfg_t config;

EEMEM cfg_t config_ee = {
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
		//last_page
		2,
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
		//number_of_pages
		MAX_NUMBER_OF_PAGES,
		//silent
		0b00000000,
		//hide_label
		0b00000000,
		//pages
		{
			//0
			{
				//type
				LAYOUT_222,
				{WIDGET_TIME, WIDGET_FTIME, WIDGET_CTRL_AUDIO, WIDGET_CTRL_WLIFT, WIDGET_TEMPERATURE, WIDGET_BATTERY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY}
			},
			//1
			{
				//type
				LAYOUT_123,
				{WIDGET_VARIO_BAR, WIDGET_VARIO, WIDGET_GHEADING_ARROW, WIDGET_ALT1, WIDGET_GLIDE_RATIO, WIDGET_GROUND_SPD, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY}
			},
			//2
			{
				//type
				LAYOUT_123,
				{WIDGET_VARIO_BAR, WIDGET_VARIO, WIDGET_AVG_VARIO, WIDGET_ALT1, WIDGET_FTIME, WIDGET_GROUND_SPD, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY}
			},
			//3
			{
				//type
				LAYOUT_121,
				{WIDGET_VARIO_BAR, WIDGET_VARIO, WIDGET_VARIO_HISTORY, WIDGET_ALT2, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY}
			},
			//4
			{
				//type
				LAYOUT_21,
				{WIDGET_TIME, WIDGET_ALT3, WIDGET_POSITION, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY}
			},
		},

	},
	//vario
	{
		//digital_vario_dampening
		1.0 / 100.0 / 0.3, // << last 0.3 sec
		//avg_vario_dampening
		1.0 / 100.0 / 10.0, // << last 10 sec
		//flags
		VARIO_UNITS_M,
		//weak_lift_enabled
		false,
		//weak_lift
		30, // << -0.3 m/s (buzzer start at lift - week_lift)
	},
	//altitude
	{
		//QNH1
		103000,
		//QNH2
		101325,
		//atl1_flags
		ALT_UNIT_M,
		//altimeter
		{
			//altimeter2
			{
				//flags
				ALT_AUTO_ZERO | ALT_DIFF | 0,
				//diff
				0,
			},
			//altimeter3
			{
				//flags
				ALT_ABS_GPS,
				//diff
				0,
			},
			//altimeter4
			{
				//flags
				ALT_DIFF | 1,
				//diff
				200,
			},
			//altimeter5
			{
				//flags
				ALT_ABS_QNH2,
				//diff
				0,
			},
		},
	},
	//audio_profile
	{
		//freq
		{127, 131, 136, 141, 146, 152, 159, 167, 175, 186, 198, 216, 234, 258, 283, 313, 344, 379, 415, 489, 564, 632, 701, 744, 788, 817, 846, 870, 894, 910, 927, 941, 955, 970, 985, 996, 1008, 1022, 1037, 1053, 1070},
		//length
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 320, 281, 242, 215, 189, 172, 155, 144, 134, 124, 115, 105, 95, 85, 75, 65, 55, 46, 37, 33, 30},
		//pause
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 540, 489, 438, 403, 368, 340, 312, 285, 259, 239, 219, 197, 176, 157, 138, 124, 110, 95, 81, 70, 60},
		//weak_lift_freq
		60,
		//lift
		10,
		//sink
		-150,
		//fluid
		1,
	},
	//System
	{
		//time flags
		TIME_SYNC,
		//timezone
		+1 * 2,
		//debug_log
		false,//DEBUG_MAGIC_ON,
		//debug_gps
		false,
		//record_screen
		false,
		//auto_power_off
		25,
	},
	//Autostart
	{
		//start_sensititvity
		6,
		//land_sensitivity
		1,
		//timeout
		60,
		//flags
		AUTOSTART_SUPRESS_AUDIO,
	},
	//Logger
	{
		//enabled
		true,
		//format
		LOGGER_IGC,
		//pilot
		log_default_text,
		//glider_type
		log_default_text,
		//glider_id
		log_default_text
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
		//password
		{'1', '2', '3', '4', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
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
	tmp.x = 89;
	tmp.y = 107;
	tmp.z = -240;
	eeprom_busy_wait();
	eeprom_write_block((void *)&tmp, (void *)&config_ro.calibration.acc_bias, sizeof(vector_i16_t));

	//sensitivity
	tmp.x = 2759;
	tmp.y = 2729;
	tmp.z = 2686;
	eeprom_busy_wait();
	eeprom_write_block((void *)&tmp, (void *)&config_ro.calibration.acc_sensitivity, sizeof(vector_i16_t));
}

void cfg_mag_write_defaults()
{
	vector_i16_t tmp;
	//bias
	tmp.x = 1364;
	tmp.y = 1752;
	tmp.z = 2136;
	eeprom_busy_wait();
	eeprom_write_block((void *)&tmp, (void *)&config_ro.calibration.mag_bias, sizeof(vector_i16_t));

	//sensitivity
	tmp.x = 5392;
	tmp.y = 4780;
	tmp.z = 5154;
	eeprom_busy_wait();
	eeprom_write_block((void *)&tmp, (void *)&config_ro.calibration.mag_sensitivity, sizeof(vector_i16_t));
}

void cfg_load()
{

	eeprom_busy_wait();
	uint8_t calib_flags = eeprom_read_byte(&config_ro.calibration_flags);
	if (calib_flags & CALIB_ACC_NOT_DONE)
	{
		cfg_acc_write_defaults();
		calib_flags &= ~CALIB_ACC_NOT_DONE;
	}

	if (calib_flags & CALIB_MAG_NOT_DONE)
	{
		cfg_mag_write_defaults();
		calib_flags &= ~CALIB_MAG_NOT_DONE;
	}

	eeprom_busy_wait();
	eeprom_update_byte(&config_ro.calibration_flags, calib_flags);

	eeprom_busy_wait();
	eeprom_read_block((void *)&config, &config_ee, sizeof(cfg_t));
}
