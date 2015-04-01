#include "conf.h"
#include "../gui/widgets/layouts/layouts.h"


EEMEM cfg_t config = {
	//gui
	{
		//contrast
		72,
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
				{WIDGET_VARIO_BAR, WIDGET_DUMMY, WIDGET_DUMMY, WIDGET_DUMMY, WIDGET_DUMMY, WIDGET_DUMMY, WIDGET_DUMMY, WIDGET_DUMMY, WIDGET_DUMMY}
			},
			//1
			{
				//type
				LAYOUT_22,
				{WIDGET_VARIO, WIDGET_VARIO_BAR, WIDGET_ALT1, WIDGET_DUMMY, WIDGET_DUMMY, WIDGET_DUMMY, WIDGET_DUMMY, WIDGET_DUMMY, WIDGET_DUMMY}
			},
			//2
			{
				//type
				LAYOUT_122,
				{WIDGET_VARIO_BAR, WIDGET_VARIO, WIDGET_AVG_VARIO, WIDGET_ALT1, WIDGET_ALT2, WIDGET_DUMMY, WIDGET_DUMMY, WIDGET_DUMMY, WIDGET_DUMMY}
			},
			//3
			{
				//type
				LAYOUT_222,
				{WIDGET_VARIO_BAR, WIDGET_VARIO, WIDGET_ALT1, WIDGET_ALT2, WIDGET_ACCX, WIDGET_DUMMY, WIDGET_DUMMY, WIDGET_DUMMY, WIDGET_DUMMY}
			},
			//4
			{
				//type
				LAYOUT_12,
				{WIDGET_VARIO_BAR, WIDGET_ALT1, WIDGET_ALT2, WIDGET_DUMMY, WIDGET_DUMMY, WIDGET_DUMMY, WIDGET_DUMMY, WIDGET_DUMMY, WIDGET_DUMMY}
			},
		},

	},
	//vario
	{
		//digital_vario_dampening
		1/1,
		//avg_vario_dampening
		(1.0/100.0) / 1.0, // << last 10 sec
	},
	//altitude
	{
		//QNH1
		103000,
		//QNH2
		101325,
	}

};
