#include "set_airspaces_class.h"

#include "../gui_list.h"
#include "../gui_value.h"
#include "../gui_dialog.h"
#include "gui_accel_calib.h"

#include "../../fc/conf.h"
#include "../../fc/airspace.h"

void gui_set_airspaces_class_init()
{
	gui_list_set(gui_set_airspaces_class_item, gui_set_airspaces_class_action, 16, GUI_SET_AIRSPACE);
}

void gui_set_airspaces_class_action(uint8_t index)
{
	if (config.airspaces.class_enabled & (1 << index))
		config.airspaces.class_enabled &= (~(1 << index));
	else
		config.airspaces.class_enabled |= (1 << index);

	
	ee_update_word(&config_ee.airspaces.class_enabled, config.airspaces.class_enabled);
	

	//Force to reload airspace cache
	fc.airspace.cache_index = 0xFFFF;
}


void gui_set_airspaces_class_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	airspace_class_to_text(text, index);

	if (config.airspaces.class_enabled & (1 << index))
		*flags =  GUI_LIST_CHECK_ON;
	else
		*flags =  GUI_LIST_CHECK_OFF;

}

