/*
 * gui_flightlog3.cpp
 *
 *  Created on: 22.02.2017
 *      Author: tilmann@bubecks.de
 */

#include "gui_flightlog3.h"
#include "gui_flightlog2.h"
#include "../gui_list.h"
#include "../gui_dialog.h"
#include "../../fc/conf.h"
#include "../../fc/logger/logger.h"

uint32_t log_duration;
uint32_t log_start;
struct flight_stats_t log_stat;

/*-----------------------------------------------------------------------*/
/* Get a string from the file                                            */
/*-----------------------------------------------------------------------*/

TCHAR* f_gets (
	TCHAR* buff,	/* Pointer to the string buffer to read */
	int len,		/* Size of string buffer (characters) */
	FIL* fp			/* Pointer to the file object */
)
{
	int n = 0;
	TCHAR c, *p = buff;
	BYTE s[2];
	UINT rc;

	while (n < len - 1) {	/* Read characters until buffer gets filled */
		f_read(fp, s, 1, &rc);
		if (rc != 1) break;
		c = s[0];
		*p++ = c;
		n++;
		if (c == '\n') break;		/* Break on EOL */
	}
	*p = 0;
	return n ? buff : 0;			/* When no data read (eof or error), return with error. */
}

void parse_logfile(const char *filename)
{
	FIL fp;
	FRESULT f_r;
	char line[80];
	char *p;

	// Set defaults, if nothing could be found in the file:
	log_start = time_get_local();
	log_duration = 0;
	log_stat.max_alt = 0;
	log_stat.min_alt = 0;
	log_stat.max_climb = 0;
	log_stat.max_sink = 0;

	DEBUG("parse_logfile(%s)\n", filename);

	f_r = f_open(&fp, filename, FA_READ);
	if ( f_r != FR_OK ) return;

	// Read from the end of the file
	f_lseek(&fp, f_size(&fp) - 512);

	while(1) {
		if ( f_gets(line, sizeof(line), &fp) == NULL ) break;

		p = strstr_P(line, PSTR("SKYDROP-START: "));
		if ( p != NULL ) {
			log_start = atol(p + 15);
			continue;
		}

		p = strstr_P(line, PSTR("SKYDROP-DURATION-ms: "));
		if ( p != NULL ) {
			log_duration = atol(p + 21);
			continue;
		}

		p = strstr_P(line, PSTR("SKYDROP-ALT-MAX-m: "));
		if ( p != NULL ) {
			log_stat.max_alt = atoi(p + 19);
			continue;
		}

		p = strstr_P(line, PSTR("SKYDROP-ALT-MIN-m: "));
		if ( p != NULL ) {
			log_stat.min_alt = atoi(p + 19);
			continue;
		}

		p = strstr_P(line, PSTR("SKYDROP-CLIMB-MAX-cm: "));
		if ( p != NULL ) {
			log_stat.max_climb = atoi(p + 22);
			continue;
		}

		p = strstr_P(line, PSTR("SKYDROP-SINK-MAX-cm: "));
		if ( p != NULL ) {
			log_stat.max_sink = atoi(p + 21);
			continue;
		}
	}
	f_close(&fp);
}

void gui_flightlog3_init()
{
	parse_logfile(flightlog_file);

	gui_list_set(gui_flightlog3_item, gui_flightlog3_action, 7, GUI_FLIGHTLOG2);
}

void gui_flightlog3_stop() {}

void gui_flightlog3_loop()
{
	gui_list_draw();
}

void gui_flightlog3_irqh(uint8_t type, uint8_t * buff)
{
	gui_list_irqh(type, buff);
}

void gui_flightlog3_action(uint8_t index)
{
}

void gui_flightlog3_item(uint8_t idx, char * text, uint8_t * flags, char * sub_text)
{
	uint32_t diff;
	uint8_t sec, min, hour, day, wday, month;
	uint16_t year;

	switch (idx) {
	case 0:
		datetime_from_epoch(log_start, &sec, &min, &hour, &day, &wday, &month, &year);

		strcpy_P(text, PSTR("Start (date):"));
		sprintf_P(sub_text, PSTR("%02d.%02d.%04d"), day, month, year);
		break;

	case 1:
		datetime_from_epoch(log_start, &sec, &min, &hour, &day, &wday, &month, &year);

		strcpy_P(text, PSTR("Start (time):"));
		sprintf_P(sub_text, PSTR("%02d:%02d.%02d"), hour, min, sec);
		break;

	case 2:
		strcpy_P(text, PSTR("Duration:"));

		diff = log_duration / 1000;
		hour = diff / 3600;
		diff %= 3600;
		min = diff / 60;
		diff %= 60;

		if (hour > 0)
			sprintf_P(sub_text, PSTR("%02d:%02d"), hour, min);
		else
			sprintf_P(sub_text, PSTR("%02d.%02d"), min, diff);

		break;
	case 3:
		strcpy_P(text, PSTR("Altitude(max):"));
		sprintf_P(sub_text, PSTR("%dm"), log_stat.max_alt);
		break;
	case 4:
		strcpy_P(text, PSTR("Altitude(min):"));
		sprintf_P(sub_text, PSTR("%dm"), log_stat.min_alt);
		break;
	case 5:
		strcpy_P(text, PSTR("Sink(max):"));
		sprintf_P(sub_text, PSTR("%0.1fm/s"), log_stat.max_sink / 100.0);
		break;
	case 6:
		strcpy_P(text, PSTR("Climb(max):"));
		sprintf_P(sub_text, PSTR("%0.1fm/s"), log_stat.max_climb / 100.0 );
		break;
	}

	*flags |= GUI_LIST_SUB_TEXT;
}

