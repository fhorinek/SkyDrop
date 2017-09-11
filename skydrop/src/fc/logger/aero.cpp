#include "raw.h"
#include "logger.h"
#include "../../drivers/storage/storage.h"

uint32_t aero_last_time = 0;
#define AERO_SAMLE_TIME		(10 * 1000)
							//1.2 * 1316 ^ 2
#define AERO_G_THOLD_RAW	2493872l

bool aero_start(char * path)
{
	char filename[128];

	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t day;
	uint8_t wday;
	uint8_t month;
	uint16_t year;

	datetime_from_epoch(time_get_utc(), &sec, &min, &hour, &day, &wday, &month, &year);

	sprintf_P(filename, PSTR("%sAER"), path);
	DEBUG("RAW filename %s\n", filename);

	uint8_t res = f_open(&log_file, filename, FA_WRITE | FA_CREATE_ALWAYS);
	assert(res == FR_OK);

	//cannot create file
	if (res != FR_OK)
		return false;

	aero_last_time = 0;

	return LOGGER_ACTIVE;
}

void aero_step()
{
	uint32_t time = task_get_ms_tick();
	uint16_t diff = time - aero_last_time;

	uint32_t g_value;

	g_value =  (int32_t)fc.acc.raw.x * (int32_t)fc.acc.raw.x;
	g_value += (int32_t)fc.acc.raw.y * (int32_t)fc.acc.raw.y;
	g_value += (int32_t)fc.acc.raw.z * (int32_t)fc.acc.raw.z;

//	DEBUG("aero_step\n");
//	DEBUG(" diff %u\n", diff);
//	DEBUG(" g_value %ld %ld\n", g_value, AERO_G_THOLD_RAW);

	if (diff < AERO_SAMLE_TIME && g_value < AERO_G_THOLD_RAW && aero_last_time != 0)
		return;

//	DEBUG("writing\n");

	uint8_t line[5];
	uint16_t wl;
	uint8_t l = 5;

	if (aero_last_time == 0)
		diff = 0;

	aero_last_time = time;

	line[0] = 0xAA;
	memcpy(line + 1, (void *)&diff, 2);

	g_value >>= 16;
	memcpy(line + 3, (void *)(&g_value), 2);

//	for (uint8_t i = 0; i < 5; i++)
//		DEBUG("%02X ", line[i]);
//	DEBUG("\n");

	assert(f_write(&log_file, line, l, &wl) == FR_OK);
	assert(wl == l);
	assert(f_sync(&log_file) == FR_OK);
}

void aero_stop()
{
	assert(f_close(&log_file) == FR_OK);
}
