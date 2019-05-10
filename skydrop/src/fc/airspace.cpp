#include "airspace.h"
#include "agl.h"

#include "../drivers/storage/storage.h"

#include "debug_on.h"

#define NO_OF_AIRSPACE_HEIGHTS 5
#define AIR_INSIDE_FLAG 0x80


typedef struct airspace_filedata1
{
	uint8_t floor;
	uint8_t ceiling;
	uint8_t angle;
	uint8_t  distance;
} airspace_filedata1_t;

typedef struct airspace_filedata
{
	airspace_filedata1_t air[NO_OF_AIRSPACE_HEIGHTS];
} airspace_filedata_t;

// This is the file handle to the current opened airspace file
FIL airspace_data_file;

// The SRMT HGT file format is described in detail in
//     http://dds.cr.usgs.gov/srtm/version2_1/Documentation/SRTM_Topo.pdf

// All lat/lon values are multiplied by HGT_COORD_MUL, so that we can use
// fixed point integer arithmetic instead of floating points:
#define HGT_COORD_MUL	10000000l

void airspace_init()
{
    DEBUG("airspace_init\n");
    fc.airspace.file_valid = false;

    fc.airspace.angle = AIRSPACE_INVALID;
    memset((void *) fc.airspace.filename, 0, sizeof(fc.airspace.filename));
}

void airspace_open_file(char * fn)
{
    //copy filename to fc struct
    strcpy((char *) fc.airspace.filename, fn);

    char path[20];
    sprintf_P(path, PSTR("/AIR/%s.AIR"), fc.airspace.filename);

    DEBUG("AIR data path is '%s'\n", path);

    if (fc.airspace.file_valid)
        assert(f_close(&airspace_data_file) == FR_OK);

    uint8_t ret = f_open(&airspace_data_file, path, FA_READ);

    if (ret == FR_OK)
    {
        //file is valid
        fc.airspace.file_valid = true;
    }
    else
    {
        //file is not available
        fc.airspace.file_valid = false;
        DEBUG("AIR file not found.\n");
    }
}

uint16_t airspace_convert_alt_ft(uint8_t raw_alt)
{
	return AIR_250ft_to_m * (0x7F & raw_alt);
}

uint16_t airspace_convert_alt_m(uint8_t raw_alt)
{
	return AIR_250ft_to_m * (0x7F & raw_alt);
}

//is device altitude below raw_alt?
bool airspace_alt_is_below(uint8_t raw_alt, uint16_t gps_alt, uint16_t msl_alt)
{
	if (airspace_convert_alt_m(raw_alt) > ((raw_alt & AIR_AGL_FLAG) ? gps_alt : msl_alt))

		return true;

	return false;
}

//is device altitude above raw_alt?
bool airspace_alt_is_above(uint8_t raw_alt, uint16_t gps_alt, uint16_t msl_alt)
{
	if (airspace_convert_alt_m(raw_alt) < ((raw_alt & AIR_AGL_FLAG) ? gps_alt : msl_alt))
		return true;

	return false;
}

bool airspace_is_inside(uint8_t raw_min, uint8_t raw_max, uint16_t gps_alt, uint16_t msl_alt)
{
	if (airspace_alt_is_below(raw_min, gps_alt, msl_alt))
		return false;

	if (airspace_alt_is_above(raw_max, gps_alt, msl_alt))
		return false;

	return true;
}

/**
 * Read the airspace data for the given lat/lon into fc.airspace.
 * This function assumes, that the right file containing lat/lon is already
 * opened and accessible by airspace_data_file.
 *
 * @param lat the latitude of the point
 * @param lon the longitude of the point
 */
void airspace_get_data_on_opened_file(int32_t lat, int32_t lon)
{
    uint16_t num_points_x;
    uint16_t num_points_y;
    airspace_filedata_t airspace;
    int i;

	if (lon < 0)
    {
        // we do not care above degree, only minutes are important
        // reverse the value, because file goes in opposite direction.
        lon = (HGT_COORD_MUL - 1) + (lon % HGT_COORD_MUL);   // lon is negative!
    }
    if (lat < 0)
    {
        // we do not care above degree, only minutes are important
        // reverse the value, because file goes in opposite direction.
        lat = (HGT_COORD_MUL - 1) + (lat % HGT_COORD_MUL);   // lat is negative!
    }

    num_points_x = num_points_y = (int)sqrt((double)(f_size(&airspace_data_file) / sizeof(airspace)));

    // "-2" is, because a file has a overlap of 1 point to the next file.
    uint32_t coord_div_x = HGT_COORD_MUL / num_points_x;
    uint32_t coord_div_y = HGT_COORD_MUL / num_points_y;
    uint16_t y = (lat % HGT_COORD_MUL) / coord_div_y;
    uint16_t x = (lon % HGT_COORD_MUL) / coord_div_x;

    uint16_t rd;

    //seek to position
    uint32_t pos = ((uint32_t) x + num_points_x * (uint32_t) ((num_points_y - y) - 1));
    DEBUG("airspace_get_data_on_opened_file: res=%u, lat=%ld, lon=%ld; x=%d, y=%d; index=%ld\n", num_points_x, lat, lon, x, y, pos);

    pos = pos * sizeof(airspace);

    assert(f_lseek(&airspace_data_file, pos) == FR_OK);
    assert(f_read(&airspace_data_file, &airspace, sizeof(airspace), &rd) == FR_OK);
    assert(rd == sizeof(airspace));

	fc.airspace.forbidden = false;
	fc.airspace.angle = AIRSPACE_INVALID;

	//limits
	fc.airspace.min_alt = 0;
	fc.airspace.max_alt = 0;

	//info
	fc.airspace.floor = 0;
	fc.airspace.ceiling = 0;

	uint16_t msl_alt = fc_press_to_alt(fc.vario.pressure, 101325);
	uint16_t gps_alt = fc.gps_data.altitude;

	DEBUG("msl_alt: %d\n", msl_alt);
	DEBUG("gps_alt: %d\n", gps_alt);

	uint16_t nearest_dist = 0xFFFF;
	uint8_t nearest_dist_i = 0xFF;

	uint8_t forbiden_i = 0;

	bool have_data = false;

    for (i = 0; i < NO_OF_AIRSPACE_HEIGHTS; i++)
    {
    	if (airspace.air[i].ceiling == 0) break; //no data

    	have_data = true;

    	DEBUG("airspace %d f=%d c=%d a=%d d=%d\n", i, airspace.air[i].floor, airspace.air[i].ceiling, airspace.air[i].angle, airspace.air[i].distance);

    	//is inside the airspace floor and ceil
    	if (airspace_is_inside(airspace.air[i].floor, airspace.air[i].ceiling, gps_alt, msl_alt))
    	{
    		//inside the airspace
    		if (airspace.air[i].angle & 0x80)
    		{
    			fc.airspace.forbidden = true;
    			forbiden_i = i;
    		}
    	}

    	//arrow point to nearest or outside of forbidden
    	if ((i == 0 || nearest_dist > airspace.air[i].distance) && !(airspace.air[i].angle & AIR_INSIDE_FLAG))
    	{
    		nearest_dist = airspace.air[i].distance;
    		nearest_dist_i = i;
    	}

		//inside the airspace
		if (airspace.air[i].angle & AIR_INSIDE_FLAG)
		{
			if (airspace_alt_is_above(airspace.air[i].floor, gps_alt, msl_alt))
				fc.airspace.min_alt = airspace.air[i].ceiling;

			if (airspace_alt_is_below(airspace.air[i].ceiling, gps_alt, msl_alt))
				fc.airspace.max_alt = airspace.air[i].floor;
		}
    }

    if (have_data)
    {
		if (fc.airspace.forbidden)
		{
			fc.airspace.angle = (airspace.air[forbiden_i].angle & 0b01111111) * 3;
			fc.airspace.distance_m = (uint16_t)airspace.air[forbiden_i].distance * 64;
			fc.airspace.ceiling = airspace.air[forbiden_i].ceiling;
			fc.airspace.floor = airspace.air[forbiden_i].floor;
		}
		else
		{
			if(nearest_dist_i != 0xFF)
			{
				fc.airspace.angle = (airspace.air[nearest_dist_i].angle & 0b01111111) * 3;
				fc.airspace.distance_m = (uint16_t)airspace.air[nearest_dist_i].distance * 64;
				fc.airspace.ceiling = airspace.air[nearest_dist_i].ceiling;
				fc.airspace.floor = airspace.air[nearest_dist_i].floor;
			}
		}

		DEBUG("\n");

		DEBUG("nearest_dist_i: %d\n", nearest_dist_i);
		DEBUG("forbiden_i: %d\n", forbiden_i);
		DEBUG("\n");

		DEBUG("fc.airspace.forbidden: %d\n", fc.airspace.forbidden);
		DEBUG("fc.airspace.angle: %d\n", fc.airspace.angle);
		DEBUG("fc.airspace.distance_m: %d\n", fc.airspace.distance_m);
		DEBUG("fc.airspace.ceiling: %d\n", fc.airspace.ceiling);
		DEBUG("fc.airspace.floor: %d\n", fc.airspace.floor);
		DEBUG("fc.airspace.min_alt: %d\n", fc.airspace.min_alt);
		DEBUG("fc.airspace.max_alt: %d\n", fc.airspace.max_alt);
		DEBUG("\n");
    }
    else
    {
		DEBUG("fc.airspace.angle: AIRSPACE_INVALID\n");
		DEBUG("\n");
    }
}

/**
 * Read the airspace data for the given lat/lon into fc.airspace. Open the
 * appropriate file and read data from it.
 *
 * @param lat the latitude of the point
 * @param lon the longitude of the point
 */
void airspace_read_pos(int32_t latitude, int32_t longtitude)
{
    char tmp_name[10];

    if (!storage_ready())
        return;

    agl_get_filename(tmp_name, latitude, longtitude);

    if (strcmp(tmp_name, (char *) fc.airspace.filename) != 0) //data file is different then previous
        airspace_open_file(tmp_name);

    if (!fc.airspace.file_valid) //file was not found
        return;

    airspace_get_data_on_opened_file(latitude, longtitude);
}

void airspace_step()
{
    if (fc.gps_data.new_sample & FC_GPS_NEW_SAMPLE_AIRSPACE)
    {
        fc.gps_data.new_sample &= ~FC_GPS_NEW_SAMPLE_AIRSPACE;

        fc.airspace.angle = AIRSPACE_INVALID;
        airspace_read_pos(fc.gps_data.latitude, fc.gps_data.longtitude);

        if (fc.airspace.angle != AIRSPACE_INVALID)
        {
        	DEBUG("AIR: lat/lon: %f %f FORBIDDEN: %d ANGLE: %d DIST: %u\n", fc.gps_data.latitude * 1.0 / HGT_COORD_MUL, fc.gps_data.longtitude * 1.0 / HGT_COORD_MUL, fc.airspace.forbidden, fc.airspace.angle, fc.airspace.distance_m);
        }
    }
}

