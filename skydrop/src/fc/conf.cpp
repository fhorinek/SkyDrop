#include "conf.h"
#include "../gui/widgets/layouts/layouts.h"

cfg_ro_t config_ro __attribute__ ((section(".cfg_ro")));

volatile cfg_t config;

EEMEM cfg_t config_ee = {
	//build_number
	BUILD_NUMBER,
	//gui
	{
		//contrast
		GUI_CONTRAST_STEPS / 2,
		//brightness
		100,
		//brightness_timeout
		3,
		//display_flags
		CFG_DISP_ANIM,
		//last_page
		2,
		//menu_audio_flags
		CFG_AUDIO_MENU_SPLASH | CFG_AUDIO_MENU_PAGES | CFG_AUDIO_MENU_BUTTONS,
		//menu_volume
		75,
		//vario_volume
		75,
		//alert_volume
		75,
		//number_of_pages
		MAX_NUMBER_OF_PAGES,
		//pages
		{
			//0
			{
				//type
				LAYOUT_12,
				{WIDGET_VARIO_BAR, WIDGET_VARIO, WIDGET_AVG_VARIO, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY}
			},
			//1
			{
				//type
				LAYOUT_12,
				{WIDGET_VARIO_BAR, WIDGET_TIME, WIDGET_FTIME, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY}
			},
			//2
			{
				//type
				LAYOUT_122,
				{WIDGET_VARIO_BAR, WIDGET_VARIO, WIDGET_AVG_VARIO, WIDGET_ALT1, WIDGET_ALT2, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY}
			},
			//3
			{
				//type
				LAYOUT_123,
				{WIDGET_VARIO_BAR, WIDGET_VARIO, WIDGET_ALT2, WIDGET_GHEADING, WIDGET_GROUND_SPD, WIDGET_GLIDE_RATIO, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY}
			},
			//4
			{
				//type
				LAYOUT_12,
				{WIDGET_VARIO_BAR, WIDGET_GHEADING, WIDGET_GROUND_SPD, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY}
			},
		},

	},
	//vario
	{
		//digital_vario_dampening
		1.0,
		//avg_vario_dampening
		1.0 / 100.0 / 10.0, // << last 10 sec
		//flags
		VARIO_UNITS_M,
		//weak_lift_enabled
		true,
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
				ALT_DIFF | 0,
				//diff
				+100,
			},
			//altimeter3
			{
				//flags
				ALT_DIFF | 0,
				//diff
				+200,
			},
			//altimeter4
			{
				//flags
				ALT_ABS_QNH2,
				//diff
				0,
			},
			//altimeter5
			{
				//flags
				ALT_ABS_GPS,
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
		-70,
		//fluid
		1,
	},
	//System
	{
		//usb_mode
		USB_MODE_MASSSTORAGE,
		//use_gps
		true,
		//use_bt
		false,
		//protocol
		PROTOCOL_DIGIFLY,
		//time flags
		TIME_DST | TIME_SYNC,
		//timezone
		+2 * 2,
		//gps_format_flags
		GPS_DDdddddd | GPS_SPD_KPH,
	},
	//Autostart
	{
		//sensitivity
		4,
		//supress_audio
		true,
	},

};

////calibration
//{
//	//mag bias
//	{476, 1179, 1141},
//	//mag sensitivity
//	{-2486, -2494, -2442},
//	//acc bias
//	{6, -46, -59},
//	//acc sensitivity
//	{1376, 1369, 1325},
//},

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
	SystemReset();
}

void cfg_load()
{
	DEBUG("Loading data form EEPROM\n");
	eeprom_busy_wait();

	eeprom_read_block((void *)&config, &config_ee, sizeof(cfg_t));
}
