#include "igc.h"
#include "logger.h"
#include "../../drivers/storage/storage.h"
#include "sha256.h"

#include "private_key.h"

//#include "debug_on.h"

Sha256Class sha256;

uint32_t igc_last_timestamp = 0;

struct igc_pre_fix
{
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	char cache_igc_latitude[9];
	char cache_igc_longtitude[10];
	int16_t balt;
	int16_t galt;
};

#define IGC_PRE_START_BUFFER	30
igc_pre_fix igc_pre_start_cache[IGC_PRE_START_BUFFER];
uint8_t igc_pre_start_index = 0;
uint8_t igc_pre_start_len = 0;


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

uint8_t igc_start(char * path)
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

	sprintf_P(filename, PSTR("%sIGC"), path);
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
	sprintf_P(line, PSTR("HFPLTPILOTINCHARGE:%s"), config.logger.pilot);
	igc_writeline(line);
	//H F CM2 CREW 2
	sprintf_P(line, PSTR("HFCM2CREW2:NIL"));
	igc_writeline(line);
	//H F GTY GLIDER TYPE
	sprintf_P(line, PSTR("HFGTYGLIDERTYPE:%s"), config.logger.glider_type);
	igc_writeline(line);
	//H F GID GLIDER ID
	sprintf_P(line, PSTR("HFGIDGLIDERID:%s"), config.logger.glider_id);
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
	//H F TZN
	sprintf_P(line, PSTR("HFTZNTIMEZONE:%+0.1f"), config.system.time_zone / 2.0);
	igc_writeline(line);

#ifdef IGC_NO_PRIVATE_KEY
	//Developer note: we can't publish the private key for signing the IGC file

	//H F FRS
	sprintf_P(line, PSTR("HFFRSSECSUSPECTUSEVALIPROG:This file is not valid. Private key not available!"));
	igc_writeline(line);
#endif

	//dump the cache
//	DEBUG("IGC dump len %d\n", igc_pre_start_len);
	for (uint8_t i = igc_pre_start_len; i > 0; i--)
	{
		int8_t index = igc_pre_start_index - i;
		if (index < 0)
			index += IGC_PRE_START_BUFFER;

//		DEBUG("IGC dump %d\n", index);

		igc_pre_fix * pfix = &igc_pre_start_cache[index];

		int16_t galt = pfix->galt;
		char c = 'A';

		if (galt == 0x7FFF)
		{
			galt = 0;
			c = 'V';
		}

		sprintf_P(line, PSTR("B%02d%02d%02d%s%s%c%05d%05d"), pfix->hour, pfix->min, pfix->sec, pfix->cache_igc_latitude, pfix->cache_igc_longtitude, c, pfix->balt, galt);
		igc_writeline(line);
	}
//	igc_comment("End of cache");
	igc_write_grecord();

	return (fc.gps_data.valid) ? LOGGER_ACTIVE : LOGGER_WAIT_FOR_GPS;
}


void igc_pre_step()
{
	int16_t galt;
	uint8_t sec;
	uint8_t min;
	uint8_t hour;

	if (fc.gps_data.fix < 2) //If there is no 2D or 3D fix
		return;

	if (igc_last_timestamp >= fc.gps_data.utc_time)
		return;

	igc_last_timestamp = fc.gps_data.utc_time;

	time_from_epoch(fc.gps_data.utc_time, &sec, &min, &hour);

	//if there is no 3D fix store INT16_MAX
	galt = (fc.gps_data.fix == 3) ? fc.gps_data.altitude : 0x7FFF;

	uint16_t alt = fc_press_to_alt(fc.vario.pressure, 101325);

	//record
	igc_pre_start_cache[igc_pre_start_index].hour = hour;
	igc_pre_start_cache[igc_pre_start_index].min = min;
	igc_pre_start_cache[igc_pre_start_index].sec = sec;
	strcpy(igc_pre_start_cache[igc_pre_start_index].cache_igc_latitude, (char*)fc.gps_data.cache_igc_latitude);
	strcpy(igc_pre_start_cache[igc_pre_start_index].cache_igc_longtitude, (char*)fc.gps_data.cache_igc_longtitude);
	igc_pre_start_cache[igc_pre_start_index].balt = alt;
	igc_pre_start_cache[igc_pre_start_index].galt = galt;


//	DEBUG("IGC PRE %d/%d\n", igc_pre_start_index, igc_pre_start_len);
	igc_pre_start_index = (igc_pre_start_index + 1) % IGC_PRE_START_BUFFER;
	if (igc_pre_start_len < IGC_PRE_START_BUFFER)
		igc_pre_start_len++;
}

void igc_step()
{
	char line[79];

	uint8_t sec;
	uint8_t min;
	uint8_t hour;

	char c;

	int16_t galt;

	DEBUG("igc_step %d %d\n", fc.gps_data.valid, fc.gps_data.fix);
	if (fc.gps_data.valid && fc.gps_data.fix == 3)
	{
		if (fc.logger_state == LOGGER_WAIT_FOR_GPS)
			fc.logger_state = LOGGER_ACTIVE;

		if (igc_last_timestamp >= fc.gps_data.utc_time)
		{
//			DEBUG("igc_last_timestamp >= fc.gps_data.utc_time\n");
			return;
		}

		igc_last_timestamp = fc.gps_data.utc_time;

		time_from_epoch(fc.gps_data.utc_time, &sec, &min, &hour);

		//New igc specification require altitude above geoid
		//From L80_GPS_Protocol_Specification_V1.4.pdf
		//fc.gps_data.altitude 	- Altitude in meters according to WGS84 ellipsoid
		//fc.gps_data.geoid		- Height of geoid above WGS84 ellipsoid
		//BUT datasheet is lying !!! fc.gps_data.altitude is MSL !!!
		galt = fc.gps_data.altitude;
		c = 'A';
	}
	else
	{
		if (fc.logger_state == LOGGER_WAIT_FOR_GPS)
			return;

		if (igc_last_timestamp >= time_get_utc())
			return;

		igc_last_timestamp = time_get_utc();

		time_from_epoch(time_get_utc(), &sec, &min, &hour);

		galt = 0;
		c = 'V';
	}

	uint16_t alt = fc_press_to_alt(fc.vario.pressure, 101325);

	//B record
	sprintf_P(line, PSTR("B%02d%02d%02d%s%s%c%05d%05d"), hour, min, sec, fc.gps_data.cache_igc_latitude, fc.gps_data.cache_igc_longtitude, c, alt, galt);
	igc_writeline(line);
	igc_write_grecord();
}

void igc_comment(char * text)
{
	char line[79];

	sprintf_P(line, PSTR("L%S %s"), LOG_MID_P, text);
	igc_writeline(line, false);
	igc_write_grecord();
}

void igc_stop()
{
	igc_pre_start_len = 0;
	assert(f_close(&log_file) == FR_OK);
}
