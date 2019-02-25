/*
 * gui_homedetail.cpp
 *
 *  Created on: 11.05.2017
 *      Author: tilmann@bubecks.de
 */

#include <gui/settings/gui_filemanager.h>
#include "gui_homedetail.h"

#include "common.h"
#include "../gui_list.h"
#include "../gui_dialog.h"
#include "../../fc/fc.h"
#include "../../fc/conf.h"
#include "../../fc/logger/logger.h"

#include "../../debug_on.h"

cfg_home home_new;

void read_homefile(const char *filename, cfg_home *home)
{
	FIL fp;
	FRESULT f_r;
	char line[80];
	char *p;
	double d;

	// Set defaults, if nothing could be found in the file:
	home->lat = 0;
	home->lon = 0;
	home->name[0] = 0;
	home->freq[0] = 0;
	home->rwy[0] = 0;
	home->traffic_pattern[0] = 0;
	home->info[0] = 0;

	DEBUG("parse_homefile(%s)\n", filename);

	f_r = f_open(&fp, filename, FA_READ);
	if (f_r != FR_OK)
		return;

	while (1)
	{
		if (f_gets(line, sizeof(line), &fp) == NULL)
			break;

		DEBUG("line '%s'\n", line);

		if (startsWith_P(PSTR("NAME: "), line))
		{
			strncpy(home->name, line + 6, sizeof(home->name));
			DEBUG("home->name %s\n", home->name);
			continue;
		}

		if (startsWith_P(PSTR("LOC: "), line))
		{
			d = atof(line + 5) * 10000000.0;
			home->lat = d;
			p = rindex(line, ' ');
			if (p != NULL)
			{
				d = atof(p + 1) * 10000000.0;
				home->lon = d;
			}
			if (strchr(line, 'S'))
			{
				home->lat = -home->lat;
			}
			if (strchr(line, 'W'))
			{
				home->lon = -home->lon;
			}

			DEBUG("home->lat %ld\n", home->lat);
			DEBUG("home->lon %ld\n", home->lon);

			continue;
		}

		if (startsWith_P(PSTR("FREQ: "), line))
		{
			strncpy(home->freq, line + 6, sizeof(home->freq));
			continue;
		}

		if (startsWith_P(PSTR("RWY: "), line))
		{
			strncpy(home->rwy, line + 5, sizeof(home->rwy));
			continue;
		}

		if (startsWith_P(PSTR("TRAFFIC PATTERN: "), line))
		{
			strncpy(home->traffic_pattern, line + 17,
					sizeof(home->traffic_pattern));
			continue;
		}

		if (startsWith_P(PSTR("INFO: "), line))
		{
			strncpy(home->info, line + 6, sizeof(home->info));
			continue;
		}
	}

	DEBUG("closing home file\n");
	f_close(&fp);
}

void gui_homedetail_init()
{
	char tmp[44];

	sprintf_P(tmp, PSTR("%s/%s"), gui_filemanager_path, gui_filemanager_name);
	read_homefile(tmp, &home_new);

	gui_list_set(gui_homedetail_item, gui_homedetail_action, 8,
	GUI_FILEMANAGER);
}

void gui_homedetail_stop()
{
}

void gui_homedetail_loop()
{
	gui_list_draw();
}

void gui_homedetail_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_homedetail_action(uint8_t index)
{
	if (index == 0)
	{
		gui_showmessage_P(PSTR("Home selected"));
		gui_switch_task(GUI_PAGES);

		memcpy((void *) &config.home, (void *) &home_new, sizeof(cfg_home));

		config.home.flags = HOME_LOADED_FROM_SD;
		eeprom_busy_wait();
		eeprom_update_block((void *) &config.home, &config_ee.home,
				sizeof(config.home));

		fc.flight.home_valid = true;
	}
}

void gui_homedetail_item(uint8_t idx, char * text, uint8_t * flags,
		char * sub_text)
{
	switch (idx)
	{
	case 0:
		strcpy_P(text, PSTR("Use this home"));
		break;

	case 1:
		strcpy_P(text, PSTR("Name:"));
		strcpy(sub_text, home_new.name);
		*flags |= GUI_LIST_SUB_TEXT;
		break;

	case 2:
		strcpy_P(text, PSTR("Lat:"));
		sprintf_P(sub_text, PSTR("%2.6f"), home_new.lat / 10000000.0);
		*flags |= GUI_LIST_SUB_TEXT;
		break;

	case 3:
		strcpy_P(text, PSTR("Lon:"));
		sprintf_P(sub_text, PSTR("%2.6f"), home_new.lon / 10000000.0);
		*flags |= GUI_LIST_SUB_TEXT;
		break;

	case 4:
		strcpy_P(text, PSTR("Frequency:"));
		strcpy(sub_text, home_new.freq);
		*flags |= GUI_LIST_SUB_TEXT;
		break;

	case 5:
		strcpy_P(text, PSTR("Runway:"));
		strcpy(sub_text, home_new.rwy);
		*flags |= GUI_LIST_SUB_TEXT;
		break;

	case 6:
		strcpy_P(text, PSTR("Traffic Pattern:"));
		strcpy(sub_text, home_new.traffic_pattern);
		*flags |= GUI_LIST_SUB_TEXT;
		break;

	case 7:
		strcpy_P(text, PSTR("Info:"));
		strcpy(sub_text, home_new.info);
		*flags |= GUI_LIST_SUB_TEXT;
		break;

	}

}
