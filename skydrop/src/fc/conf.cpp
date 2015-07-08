#include "conf.h"
#include "../gui/widgets/layouts/layouts.h"

cfg_ro_t config_ro __attribute__ ((section(".cfg_ro")));

EEMEM cfg_t config = {
	//gui
	{
		//contrast
		70,
		//brightness
		100,
		//brightness_timeout
		3,
		//display_flags
		CFG_DISP_ANIM,
		//last_page
		2,
		//pages
		{
			//0
			{
				//type
				LAYOUT_12,
				{WIDGET_VARIO_BAR, WIDGET_VARIO, WIDGET_DEBUG, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY}
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
				{WIDGET_VARIO_BAR, WIDGET_VARIO, WIDGET_ALT2, WIDGET_GHEADING, WIDGET_GROUND_SPD, WIDGET_TIME, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY}
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
		0,
		//avg_vario_dampening
		10, // << last 10 sec
		//flags
		VARIO_UNITS_M,
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
	//calibration
	{
		//mag bias
		{476, 1179, 1141},
		//mag sensitivity
		{-2486, -2494, -2442},
		//acc bias
		{6, -46, -59},
		//acc sensitivity
		{1376, 1369, 1325},
	},
	//audio_profile
	{
		//freq
		{127, 131, 136, 141, 146, 152, 159, 167, 175, 186, 198, 216, 234, 258, 283, 313, 344, 379, 415, 489, 564, 632, 701, 744, 788, 817, 846, 870, 894, 910, 927, 941, 955, 970, 985, 996, 1008, 1022, 1037, 1053, 1070},
		//length
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 540, 489, 438, 403, 368, 340, 312, 285, 259, 239, 219, 197, 176, 157, 138, 124, 110, 95, 81, 70, 60},
		//pause
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 320, 281, 242, 215, 189, 172, 155, 144, 134, 124, 115, 105, 95, 85, 75, 65, 55, 46, 37, 33, 30},
		//lift
		10,
		//sink
		-70,
		//fluid
		1,
		//volume
		80,
	},
	//System
	{
		//usb_mode
		USB_MODE_MASSSTORAGE,
		//use_gps
		true,
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

	fc.QNH1 = eeprom_read_float(&config.altitude.QNH1);
	DEBUG("QNH1 %0.1f\n", fc.QNH1);

	fc.QNH2 = eeprom_read_float(&config.altitude.QNH2);
	DEBUG("QNH2 %0.1f\n", fc.QNH2);

	fc.digital_vario_dampening = eeprom_read_float(&config.vario.digital_vario_dampening);
	if (fc.digital_vario_dampening == 0)
		fc.digital_vario_dampening = 1;
	else
		fc.digital_vario_dampening = 1.0 / 100.0 / fc.digital_vario_dampening;

	DEBUG("digital_vario_dampening %0.2f\n", fc.digital_vario_dampening);

	fc.avg_vario_dampening = eeprom_read_float(&config.vario.avg_vario_dampening);
	if (fc.avg_vario_dampening == 0)
		fc.avg_vario_dampening = 1;
	else
		fc.avg_vario_dampening = 1.0 / 100.0 / fc.avg_vario_dampening;

	DEBUG("avg_vario_dampening %0.2f\n", fc.avg_vario_dampening);

	fc.vario_flags = eeprom_read_byte(&config.vario.flags);
	DEBUG("vario_flags %d\n", fc.vario_flags);

	fc.alt1_flags = eeprom_read_byte(&config.altitude.alt1_flags);
	DEBUG(" alt1_flags %02X\n", fc.alt1_flags);

	for (uint8_t i=0; i<NUMBER_OF_ALTIMETERS; i++)
	{
		DEBUG("altimeter[%d]\n", i);
		fc.altimeter[i].altitude = 0;

		fc.altimeter[i].flags = eeprom_read_byte(&config.altitude.altimeter[i].flags);
		DEBUG(" flags %02X\n", fc.altimeter[i].flags);

		eeprom_read_block((int16_t *) &fc.altimeter[i].delta, &config.altitude.altimeter[i].delta, sizeof(int16_t));
		DEBUG(" delta %d\n", fc.altimeter[i].delta);
	}

	eeprom_read_block((void *)&fc.mag_bias, &config.calibration.mag_bias, sizeof(vector_i16_t));
	DEBUG("mag_bias\n");
	DEBUG(" x %d\n", fc.mag_bias.x);
	DEBUG(" y %d\n", fc.mag_bias.y);
	DEBUG(" z %d\n", fc.mag_bias.z);

	eeprom_read_block((void *)&fc.mag_sensitivity, &config.calibration.mag_sensitivity, sizeof(vector_i16_t));
	DEBUG("mag_sensitivity\n");
	DEBUG(" x %d\n", fc.mag_sensitivity.x);
	DEBUG(" y %d\n", fc.mag_sensitivity.y);
	DEBUG(" z %d\n", fc.mag_sensitivity.z);

	eeprom_read_block((void *)&fc.acc_bias, &config.calibration.acc_bias, sizeof(vector_i16_t));
	DEBUG("acc_bias\n");
	DEBUG(" x %d\n", fc.acc_bias.x);
	DEBUG(" y %d\n", fc.acc_bias.y);
	DEBUG(" z %d\n", fc.acc_bias.z);

	eeprom_read_block((void *)&fc.acc_sensitivity, &config.calibration.acc_sensitivity, sizeof(vector_i16_t));
	DEBUG("acc_sensitivity\n");
	DEBUG(" x %d\n", fc.acc_sensitivity.x);
	DEBUG(" y %d\n", fc.acc_sensitivity.y);
	DEBUG(" z %d\n", fc.acc_sensitivity.z);

	eeprom_read_block((void *)&fc.buzzer_freq, &config.audio_profile.freq, sizeof(int16_t) * AUDIO_PROFILE_SIZE);
	DEBUG("buzzer_freq\n");
	for (uint8_t i = 0; i < AUDIO_PROFILE_SIZE; i++)
		DEBUG(" %d", fc.buzzer_freq[i]);
	DEBUG("\n");

	eeprom_read_block((void *)&fc.buzzer_length, &config.audio_profile.length, sizeof(int16_t) * AUDIO_PROFILE_SIZE);
	DEBUG("buzzer_length\n");
	for (uint8_t i = 0; i < AUDIO_PROFILE_SIZE; i++)
		DEBUG(" %d", fc.buzzer_length[i]);
	DEBUG("\n");

	eeprom_read_block((void *)&fc.buzzer_pause, &config.audio_profile.pause, sizeof(int16_t) * AUDIO_PROFILE_SIZE);
	DEBUG("buzzer_pause\n");
	for (uint8_t i = 0; i < AUDIO_PROFILE_SIZE; i++)
		DEBUG(" %d", fc.buzzer_pause[i]);
	DEBUG("\n");

	fc.audio_lift = eeprom_read_word((uint16_t *)&config.audio_profile.lift);
	DEBUG("audio_lift %d\n", fc.audio_lift);

	fc.audio_sink = eeprom_read_word((uint16_t *)&config.audio_profile.sink);
	DEBUG("audio_sink %d\n", fc.audio_sink);

	fc.audio_fluid = eeprom_read_byte(&config.audio_profile.fluid);
	DEBUG("audio_fluid %d\n", fc.audio_fluid);

	fc.audio_volume = eeprom_read_byte(&config.audio_profile.volume);
	DEBUG("audio_volume %d\n", fc.audio_volume);

	fc.usb_mode = eeprom_read_byte(&config.system.usb_mode);
	DEBUG("audio_volume %d\n", fc.usb_mode);

	fc.autostart_sensitivity = eeprom_read_byte(&config.autostart.sensititvity);
	DEBUG("autostart_sensitivity %d\n", fc.autostart_sensitivity);

	fc.audio_supress = eeprom_read_byte(&config.autostart.supress_audio);
	DEBUG("audio_supress %d\n", fc.audio_supress);

	fc.use_gps = eeprom_read_byte(&config.system.use_gps);
	DEBUG("use_gps %d\n", fc.use_gps);

	fc.time_flags = eeprom_read_byte(&config.system.time_flags);
	DEBUG("time_flags %d\n", fc.time_flags);

	eeprom_read_block((void *)&fc.time_zone, &config.system.time_zone, sizeof(int8_t));
	DEBUG("time_zone %d\n", fc.time_zone);

	fc.gps_data.format_flags = eeprom_read_byte(&config.system.gps_format_flags);
	DEBUG("gps_data.format_flags %d\n", fc.gps_data.format_flags);

	DEBUG("\n");
}
