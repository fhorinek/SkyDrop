#include "set_gps.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"
#include "../../drivers/sensors/gps_l80.h"

void gui_set_gps_init()
{
	gui_list_set(gui_set_gps_item, gui_set_gps_action, 5);

	if (fc.use_gps)
		gps_detail();
}

void gui_set_gps_stop()
{

}

void gui_set_gps_loop()
{
	gui_list_draw();
}

void gui_set_gps_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_set_gps_action(uint8_t index)
{
	switch(index)
	{
	case(0):
		fc.use_gps = !fc.use_gps;
		eeprom_busy_wait();
		eeprom_update_byte(&config.system.use_gps, fc.use_gps);
	break;

	case(1):
		gui_switch_task(GUI_SET_GPS_DETAIL);
	break;

	case(2):
	break;

	case(3):

	break;

	case(4):
		gui_switch_task(GUI_SETTINGS);
		if (fc.use_gps)
			gps_normal();
	break;
	}
}

void gui_set_gps_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			sprintf_P(text, PSTR("Enable GPS"));
			if (fc.use_gps)
				*flags |= GUI_LIST_CHECK_ON;
			else
				*flags |= GUI_LIST_CHECK_OFF;

		break;
		case (1):
			sprintf_P(text, PSTR("Satellites"));
			sprintf_P(sub_text, PSTR("%d/%d"), fc.gps_data.sat_used, fc.gps_data.sat_total);
			*flags |= GUI_LIST_SUB_TEXT;
		break;
		case (2):
			sprintf_P(text, PSTR("GPS fix"));
			switch (fc.gps_data.fix)
			{
				case(2):
					sprintf_P(sub_text, PSTR("2D Fix"));
				break;
				case(3):
					sprintf_P(sub_text, PSTR("3D Fix"));
				break;
				default:
					sprintf_P(sub_text, PSTR("No Fix"));
				break;
			}
			*flags |= GUI_LIST_SUB_TEXT;
		break;
		case (3):
			sprintf_P(text, PSTR("Details"));
		break;
		case (4):
			sprintf_P(text, PSTR("back"));
			*flags |= GUI_LIST_BACK;
		break;

	}
}

