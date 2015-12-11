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

void igc_writeline(char * line)
{
	uint8_t l = strlen(line);
	uint16_t wl;

//	DEBUG("IGC:%s\n", line);

	strcpy_P(line + l, PSTR("\r\n"));
	l += 2;

	assert(f_write(log_fil, line, l, &wl) == FR_OK);
	assert(wl == l);
	assert(f_sync(log_fil) == FR_OK);

	for (uint8_t i = 0; i < l; i++)
		sha256.write(line[i]);
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

	datetime_from_epoch(fc.gps_data.utc_time, &sec, &min, &hour, &day, &wday, &month, &year);

	//XXX
	#define device_uid "DRP"

	sprintf_P(filename, PSTR("/%s/%02d-%02d%02d.IGC"), path, logger_flight_number, hour, min);
	DEBUG("IGC filename %s\n", filename);

	uint8_t res = f_open(log_fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
	assert(res == FR_OK);
	DEBUG("res == %02X\n", res);

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
	//H F PLT PILOT IN CHARGE XXX
	sprintf_P(line, PSTR("HFPLTPILOTINCHARGE:"));
	igc_writeline(line);
	//H F CM2 CREW 2
	sprintf_P(line, PSTR("HFCM2CREW2:"));
	igc_writeline(line);
	//H F GTY GLIDER TYPE XXX
	sprintf_P(line, PSTR("HFGTYFLIDERTYPE:"));
	igc_writeline(line);
	//H F GID GLIDER ID XXX
	sprintf_P(line, PSTR("HFGIDGLIDERID:"));
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
	//H F FSP
	sprintf_P(line, PSTR("HFFSP:AL4"));
	igc_writeline(line);
	//H F ALG GEO
	sprintf_P(line, PSTR("HFALGGEO"));
	igc_writeline(line);
	//H F ALP
	sprintf_P(line, PSTR("HFALPISA"));
	igc_writeline(line);

#ifdef IGC_NO_PRIVATE_KEY
	// Developer note: we cann't publish the private key for signing the IGC file

	//H F FRS
	sprintf_P(line, PSTR("HFFRSSECSUSPECTUSEVALIPROG:This file is not valid. Private key not available!"));
	igc_writeline(line);
#endif

	return true;
}

void igc_step()
{
	char line[79];

	uint8_t sec;
	uint8_t min;
	uint8_t hour;

	time_from_epoch(fc.gps_data.utc_time, &sec, &min, &hour);

	float decimal;
	float deg;
	float fmin;
	char c;
	char tmp1[32];

	c = (fc.gps_data.latitude < 0) ? 'S' : 'N';
	decimal = abs(fc.gps_data.latitude);
	deg = floor(decimal);
	fmin = (decimal - deg) * 60 * 1000;
	sprintf_P(tmp1, PSTR("%02.0f%05.0f%c"), deg, fmin, c);

	char tmp2[32];

	c = (fc.gps_data.longtitude < 0) ? 'W' : 'E';
	decimal = abs(fc.gps_data.longtitude);
	deg = floor(decimal);
	fmin = (decimal - deg) * 60 * 1000;
	sprintf_P(tmp2, PSTR("%03.0f%05.0f%c"), deg, fmin, c);

	c = (fc.gps_data.valid) ? 'A' : 'V';

	uint16_t alt = fc_press_to_alt(fc.pressure, 101325);

	//B record
	sprintf_P(line, PSTR("B%02d%02d%02d%s%s%c%05d%05.0f"), hour, min, sec, tmp1, tmp2, c, alt, fc.gps_data.altitude);
	igc_writeline(line);
}

void igc_stop()
{
	char line[79];

	//G record
	uint8_t * res = sha256.result();
	strcpy(line, "G");
	for (uint8_t i = 0; i < 20; i++)
	{
		char tmp[3];

		sprintf_P(tmp, PSTR("%02X"), res[i]);
		strcat(line, tmp);
	}

	igc_writeline(line);

	assert(f_close(log_fil) == FR_OK);
}
