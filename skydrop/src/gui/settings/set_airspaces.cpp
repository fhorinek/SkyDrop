#include "set_airspaces.h"

#include "../gui_list.h"
#include "../gui_value.h"
#include "../gui_dialog.h"
#include "gui_accel_calib.h"

#include "../../fc/conf.h"

void gui_set_airspaces_init()
{
	gui_list_set(gui_set_airspaces_item, gui_set_airspaces_action, 5, GUI_SETTINGS);
}


void gui_set_airspaces_warning_cb(float val)
{
    gui_switch_task(GUI_SET_AIRSPACE);

    config.airspaces.warning_m = val;
    
    ee_update_word(&config_ee.airspaces.warning_m, config.airspaces.warning_m);
}

void gui_set_airspaces_reset_cb(uint8_t ret)
{
	if (ret == GUI_DIALOG_YES)
	{
		char path[32];

		strcpy_P(path, PSTR("/AIR/IGN_SOFT"));
		f_delete_node(path, sizeof(path));
		strcpy_P(path, PSTR("/AIR/IGN_HARD"));
		f_delete_node(path, sizeof(path));

	    memset((void *)&fc.airspace.ignore, 0, sizeof(fc.airspace.ignore));
	    gui_showmessage_P(PSTR("Done"));
	}

	gui_switch_task(GUI_SET_AIRSPACE);
}

void gui_set_airspace_confirm_secs_cb(float val)
{
	gui_switch_task(GUI_SET_AIRSPACE);

	config.airspaces.alarm_confirm_secs = val;
	ee_update_byte(&config_ee.airspaces.alarm_confirm_secs, config.airspaces.alarm_confirm_secs);
}


void gui_set_airspaces_action(uint8_t index)
{
	switch(index)
	{
		case(0):
				gui_switch_task(GUI_SET_AIRSPACE_CLASS);
		break;

		case(1):
				gui_value_conf_P(PSTR("Alert near"), GUI_VAL_NUMBER_DISABLE, PSTR("%0.0f m"), config.airspaces.warning_m, 0, 3000, 100, gui_set_airspaces_warning_cb);
				gui_switch_task(GUI_SET_VAL);
		break;

		case(2):
				config.airspaces.alert_on = !config.airspaces.alert_on;
				
				ee_update_byte((uint8_t *)&config_ee.airspaces.alert_on, config.airspaces.alert_on);
		break;

		case(3):
			gui_value_conf_P(PSTR("Confirm time"), GUI_VAL_NUMBER_DISABLE, PSTR("%0.0f secs"), config.airspaces.alarm_confirm_secs, 0, 255, 1, gui_set_airspace_confirm_secs_cb);

			gui_switch_task(GUI_SET_VAL);
		break;

		case(4):
				gui_dialog_set_P(PSTR("Confirm"), PSTR("Reset airspace\nignore list?"), GUI_STYLE_YESNO, gui_set_airspaces_reset_cb);
				gui_switch_task(GUI_DIALOG);
		break;
	}
}

void gui_set_airspaces_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{
		case (0):
			strcpy_P(text, PSTR("Enabled classes"));
		break;

		case (1):
			strcpy_P(text, PSTR("Alert near"));
			*flags =  GUI_LIST_SUB_TEXT;
			if (config.airspaces.warning_m == 0)
				strcpy_P(sub_text, PSTR("disabled"));
			else
				sprintf_P(sub_text, PSTR("%um"), config.airspaces.warning_m);
		break;

		case (2):
			strcpy_P(text, PSTR("Alert inside"));

			if (config.airspaces.alert_on)
				*flags =  GUI_LIST_CHECK_ON;
			else
				*flags =  GUI_LIST_CHECK_OFF;

		break;

		case (3):
			strcpy_P(text, PSTR("Auto confirm"));
			if (config.airspaces.alarm_confirm_secs == 0)
				sprintf_P(sub_text, PSTR("Disabled"));
			else
				sprintf_P(sub_text, PSTR("%u secs"), config.airspaces.alarm_confirm_secs);
			*flags =  GUI_LIST_SUB_TEXT;
		break;

		case (4):
			strcpy_P(text, PSTR("Reset ignore list"));
		break;
	}
}

