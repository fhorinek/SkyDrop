#include "igc.h"
#include "logger.h"
#include "../../drivers/storage/storage.h"
#include "sha256.h"

#include <private_key.h>

Sha256Class sha256;

char igc_i2c(uint8_t n)
{
	if (n < 10)
		return '0' + n;

	n -= 10;
	return 'A' + n;
}

void igc_writeline(char * line, bool sign = true)
{
	uint8_t l = strlen(line);
	uint16_t wl;

//	DEBUG("IGC:%s\n", line);

	strcpy_P(line + l, PSTR("\r\n"));
	l += 2;

	assert(f_write(&log_file, line, l, &wl) == FR_OK);
	assert(wl == l);
	assert(f_sync(&log_file) == FR_OK);

#ifndef IGC_NO_PRIVATE_KEY
	if (sign)
		for (uint8_t i = 0; i < l; i++)
			sha256.write(line[i]);
#endif
}

IGC_PRIVATE_KEY_BODY

bool igc_start(char * path)
{
	char filename[128];

	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t day;
	uint8_t wday;
	uint8_t month;
	uint16_t year;

	char line[79];
	char id[32];

	sha256.init();

	IGC_PRIVATE_KEY_ADD;

	datetime_from_epoch(time_get_utc(), &sec, &min, &hour, &day, &wday, &month, &year);

	//XXX
	#define device_uid "DRP"

	sprintf_P(filename, PSTR("/%s/%02d-%02d%02d.IGC"), path, logger_flight_number, hour, min);
	DEBUG("IGC filename %s\n", filename);

	uint8_t res = f_open(&log_file, filename, FA_WRITE | FA_CREATE_ALWAYS);
	assert(res == FR_OK);
	DEBUG("f_open res = %02X\n", res);

	//cannot create file
	if (res != FR_OK)
		return false;

	//A record
	GetID_str(id);
	sprintf_P(line, PSTR("A%S%s:%s"), LOG_MID_P, device_uid, id);
	igc_writeline(line);

	//H records
	//H F DTE
	sprintf_P(line, PSTR("HFDTE%02u%02u%02u"), day, month, year % 100);
	igc_writeline(line);
//	//H F DTE
//	sprintf_P(line, PSTR("HFDTEDATE:%02u%02u%02u,%02u"), day, month, year  % 100, logger_flight_number);
//	igc_writeline(line);
	//H F PLT PILOT IN CHARGE
	sprintf_P(line, PSTR("HFPLTPILOTINCHARGE: %s"), config.logger.pilot);
	igc_writeline(line);
	//H F CM2 CREW 2
	sprintf_P(line, PSTR("HFCM2CREW2:"));
	igc_writeline(line);
	//H F GTY GLIDER TYPE
	sprintf_P(line, PSTR("HFGTYGLIDERTYPE: %s"), config.logger.glider_type);
	igc_writeline(line);
	//H F GID GLIDER ID
	sprintf_P(line, PSTR("HFGIDGLIDERID: %s"), config.logger.glider_id);
	igc_writeline(line);
	//H F DTM GPS DATUM
	sprintf_P(line, PSTR("HFDTMGPSDATUM:WGS84"));
	igc_writeline(line);
	//H F RFW FIRMWARE VERSION
	sprintf_P(line, PSTR("HFRFWFIRMWAREVERSION:build %d"), BUILD_NUMBER);
	igc_writeline(line);
	//H F RHW HARDWARE VERSION
	sprintf_P(line, PSTR("HFRHWHARDWAREVERSION:drop_%d"), (hw_revision == HW_REW_1504) ? 1504 : 1506);
	igc_writeline(line);
	//H F FTY FR TYPE
	sprintf_P(line, PSTR("HFFTYFRTYPE:SkyBean,SkyDrop"));
	igc_writeline(line);
	//H F GPS RECEIVER
	sprintf_P(line, PSTR("HFGPSRECEIVER:Quectel,L80,22cm,18000m"));
	igc_writeline(line);
	//H F PRS PRESS ALT SENSOR
	sprintf_P(line, PSTR("HFPRSPRESSALTSENSOR:Measurement specialties,MS5611,25907m"));
	igc_writeline(line);
	//H F ALG ALT GPS
	sprintf_P(line, PSTR("HFALGALTGPS:GEO"));
	igc_writeline(line);
	//H F ALP
	sprintf_P(line, PSTR("HFALPALTPRESSURE:ISA"));
	igc_writeline(line);

#ifdef IGC_NO_PRIVATE_KEY
	//Developer note: we can't publish the private key for signing the IGC file

	//H F FRS
	sprintf_P(line, PSTR("HFFRSSECSUSPECTUSEVALIPROG:This file is not valid. Private key not available!"));
	igc_writeline(line);
#endif

	return (fc.gps_data.valid) ? LOGGER_ACTIVE : LOGGER_WAIT_FOR_GPS;
}

void igc_write_grecord()
{
#ifndef IGC_NO_PRIVATE_KEY
	char line[79];

	Sha256Class tmp_sha;
	memcpy(&tmp_sha, &sha256, sizeof(tmp_sha));

	//G record
	uint8_t * res = tmp_sha.result();
	strcpy(line, "G");
	for (uint8_t i = 0; i < 20; i++)
	{
		char tmp[3];

		sprintf_P(tmp, PSTR("%02X"), res[i]);
		strcat(line, tmp);
	}

	igc_writeline(line, false);

	//rewind pointer
	uint32_t pos = f_tell(&log_file);
	assert(f_lseek(&log_file, pos - 43) == FR_OK);
#endif
}

uint32_t igc_last_timestamp = 0;

void igc_step()
{
	char line[79];

	uint8_t sec;
	uint8_t min;
	uint8_t hour;

	char c;

	if (fc.gps_data.valid)
	{
		if (fc.logger_state == LOGGER_WAIT_FOR_GPS)
			fc.logger_state = LOGGER_ACTIVE;

		if (igc_last_timestamp == fc.gps_data.utc_time)
			return;

		igc_last_timestamp = fc.gps_data.utc_time;

		time_from_epoch(fc.gps_data.utc_time, &sec, &min, &hour);
		c = 'A';
	}
	else
	{
		if (fc.logger_state == LOGGER_WAIT_FOR_GPS)
			return;

		if (igc_last_timestamp == time_get_utc())
			return;

		igc_last_timestamp = time_get_utc();

		time_from_epoch(time_get_utc(), &sec, &min, &hour);
		c = 'V';
	}

	uint16_t alt = fc_press_to_alt(fc.vario.pressure, 101325);

	//B record
	sprintf_P(line, PSTR("B%02d%02d%02d%s%s%c%05d%05.0f"), hour, min, sec, fc.gps_data.cache_igc_latitude, fc.gps_data.cache_igc_longtitude, c, alt, fc.gps_data.altitude);
	igc_writeline(line);
	igc_write_grecord();
}

void igc_comment(char * text)
{
	char line[79];

	sprintf_P(line, PSTR("L%S %s"), LOG_MID_P, text);
	igc_writeline(line, false);
}

void igc_stop()
{
	assert(f_close(&log_file) == FR_OK);
}
