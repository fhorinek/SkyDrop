/*
 * gui_compass_calib.cpp
 *
 *  Created on: Mar 30, 2017
 *      Author: fiala
 */

#include "gui_compass_calib.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"

void gui_compass_calib_init()
{
	gui_list_set(gui_compass_calib_item, gui_compass_calib_action, 2, GUI_SET_CALIB);
}

void gui_compass_calib_declination(float val)
{
	gui_switch_task(GUI_SET_COMPASS);


	fc.compass.declination = int(val);
	int16_t value = fc.compass.declination;
	eeprom_busy_wait();
	eeprom_update_block(&value, &config_ro.magnetic_declination, sizeof(config_ro.magnetic_declination));

}

void gui_compass_calib_action(uint8_t index)
{
	switch(index)
	{
	case(0):
		if(fc.compass.declination == 0)
		{
			gui_value_conf_P(PSTR("Set declination"), GUI_VAL_NUMBER, PSTR("%0.0f deg"), fc.compass.declination, -180.0, 180.0, 1.0, gui_compass_calib_declination);
			gui_switch_task(GUI_SET_VAL);
		}
		else
		{
			fc.compass.declination = 0;
			eeprom_busy_wait();
			int16_t value = 0;
			eeprom_update_block(&value, &config_ro.magnetic_declination, sizeof(config_ro.magnetic_declination));
		}
	break;

	case(1):
		gui_value_conf_P(PSTR("Set declination"), GUI_VAL_NUMBER, PSTR("%0.0f deg"), fc.compass.declination, -180.0, 180.0, 1.0, gui_compass_calib_declination);
		gui_switch_task(GUI_SET_VAL);
	break;


	}
}

void gui_compass_calib_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
	switch (index)
	{

	case (0):
		strcpy_P(text, PSTR("Compass mode"));
		*flags = GUI_LIST_SUB_TEXT;

		if(fc.compass.declination == 0.0)
			strcpy_P(sub_text, PSTR("magnetic north"));
		else
			strcpy_P(sub_text, PSTR("geographic north"));

	break;

	case(1):
		strcpy_P(text, PSTR("Declination"));
		sprintf_P(sub_text, PSTR("%d deg"), fc.compass.declination);
		*flags = GUI_LIST_SUB_TEXT;
	break;
	}
}
