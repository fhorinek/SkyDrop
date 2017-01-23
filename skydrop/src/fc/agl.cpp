#include "agl.h"

#include "fc.h"
#include "../drivers/storage/storage.h"

FIL agl_data_file;

#define HGT_COORD_MUL	10000000ul
#define HGT_COORD_DIV	8340ul
#define HGD_DATA_WIDTH	1201ul

void agl_init()
{
	DEBUG("agl_init\n");
	fc.agl.valid = false;
	fc.agl.file_valid = false;

	fc.agl.ground_level = 0;
	memset((void *)fc.agl.filename, 0, sizeof(fc.agl.filename));
}

void agl_get_filename(char * fn, int32_t lat, int32_t lon)
{
	char lat_c = (lat >= 0) ? 'N' : 'S';
	char lon_c = (lon >= 0) ? 'E' : 'W';

    uint8_t lat_n = abs(lat / HGT_COORD_MUL);
	uint16_t lon_n = abs(lon / HGT_COORD_MUL);

	sprintf_P(fn, PSTR("%c%02u%c%03u"), lat_c, lat_n, lon_c, lon_n);
}

void agl_open_file(char * fn)
{
	//copy filename to fc struct
	strcpy((char *)fc.agl.filename, fn);

	char path[20];
	sprintf_P(path, PSTR("/AGL/%s.HGT"), fc.agl.filename);

	DEBUG("AGL data path is '%s'\n", path);

	if (fc.agl.file_valid)
		assert(f_close(&agl_data_file) == FR_OK);

	uint8_t ret = f_open(&agl_data_file, path, FA_READ);

	DEBUG("ret = %u\n", ret);

	if (ret == FR_OK)
	{
		//file is valid
		fc.agl.file_valid = true;
	}
	else
	{
		//file is not availible
		fc.agl.file_valid = false;
		fc.agl.valid = false;
	}

}

int16_t agl_get_alt(int32_t lat, int32_t lon)
{
	//get file position
    uint16_t y = (lat % HGT_COORD_MUL) / HGT_COORD_DIV;
	uint16_t x = (lon % HGT_COORD_MUL) / HGT_COORD_DIV;

	uint16_t rd;
	uint8_t tmp[4];
	byte2 alt11, alt12, alt21, alt22;

	//seek to position
    uint32_t pos = ((uint32_t)x + HGD_DATA_WIDTH * (uint32_t)((HGD_DATA_WIDTH - y) - 1)) * 2;
    assert(f_lseek(&agl_data_file, pos) == FR_OK);
    assert(f_read(&agl_data_file, tmp, 4, &rd) == FR_OK);
    assert(rd == 4);

    //switch big endian to little
    alt11.uint8[0] = tmp[1];
    alt11.uint8[1] = tmp[0];

    alt21.uint8[0] = tmp[3];
    alt21.uint8[1] = tmp[2];

    //seek to position
    pos = ((uint32_t)x + HGD_DATA_WIDTH * (uint32_t)((HGD_DATA_WIDTH - y) - 2)) * 2;
    assert(f_lseek(&agl_data_file, pos) == FR_OK);
    assert(f_read(&agl_data_file, tmp, 4, &rd) == FR_OK);
    assert(rd == 4);

    //switch big endian to little
    alt12.uint8[0] = tmp[1];
    alt12.uint8[1] = tmp[0];

    alt22.uint8[0] = tmp[3];
    alt22.uint8[1] = tmp[2];

	//get point displacement
	float lat_dr = ((lat % HGT_COORD_MUL) % HGT_COORD_DIV) / float(HGT_COORD_DIV);
	float lon_dr = ((lon % HGT_COORD_MUL) % HGT_COORD_DIV) / float(HGT_COORD_DIV);

    float alt1 = alt11.int16 + float(alt12.int16 - alt11.int16) * lat_dr;
    float alt2 = alt21.int16 + float(alt22.int16 - alt21.int16) * lat_dr;

    return alt1 + float(alt2 - alt1) * lon_dr;
}

void agl_step()
{
	if (!storage_ready())
		return;

	if (fc.gps_data.new_sample & FC_GPS_NEW_SAMPLE_AGL)
	{
		char tmp_name[10];
		agl_get_filename(tmp_name, fc.gps_data.latitude, fc.gps_data.longtitude);

		if (strcmp(tmp_name, (char *)fc.agl.filename) == 0) //data file is the same as actual
		{
			//clear this flag only after dat file was opened
			fc.gps_data.new_sample &= ~FC_GPS_NEW_SAMPLE_AGL;

			if (!fc.agl.file_valid) //file was not found
				return;

			//get ground level
			fc.agl.ground_level = agl_get_alt(fc.gps_data.latitude, fc.gps_data.longtitude);
			fc.agl.valid = true;
		}
		else //data file is diffrent than actual
		{
			//try to open data file
			agl_open_file(tmp_name);
		}
	}
}

