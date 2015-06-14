#include "conf.h"
#include "../gui/widgets/layouts/layouts.h"


EEMEM cfg_t config = {
	//gui
	{
		//contrast
		70,
		//brightness
		100,
		//brightness_timeout
		3,
		//pages
		{
			//0
			{
				//type
				LAYOUT_1,
				{WIDGET_VARIO_BAR, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY}
			},
			//1
			{
				//type
				LAYOUT_1,
				{WIDGET_VARIO, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY}
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
				LAYOUT_222,
				{WIDGET_VARIO_BAR, WIDGET_VARIO, WIDGET_ALT1, WIDGET_ALT2, WIDGET_ALT3, WIDGET_ALT4, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY}
			},
			//4
			{
				//type
				LAYOUT_12,
				{WIDGET_VARIO_BAR, WIDGET_VARIO, WIDGET_ALT1, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY, WIDGET_EMPTY}
			},
		},

	},
	//vario
	{
		//digital_vario_dampening
		0,
		//avg_vario_dampening
		10, // << last 10 sec
	},
	//altitude
	{
		//QNH1
		103000,
		//QNH2
		101325,

		//altimeter
		{
			//altimeter2
			{
				//flags
				ALT_DIFF | 1,
				//diff
				+100,
			},
			//altimeter3
			{
				//flags
				ALT_DIFF | 1,
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

	},
	//Autostart
	{
		//sensitivity
		4,
		//supress_audio
		true,
	},

};
