#include "airspace.h"
#include "agl.h"

#include "../drivers/storage/storage.h"

#include "debug_on.h"

#define NO_OF_AIRSPACE_HEIGHTS 5

typedef struct airspace_filedata1
{
	int16_t upper_border;
	uint8_t angle;
	uint8_t  distance_m;
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
    fc.airspace.max_height_m = 0;
	fc.airspace.forbidden = false;
	fc.airspace.angle = AIRSPACE_INVALID;
    for ( i = 0; i < NO_OF_AIRSPACE_HEIGHTS; i++ )
    {
    	DEBUG("airspace_get_data_on_opened_file: i=%d alt1=%fm up=%dft, angle=%d; dist=%d\n", i, fc.altitude1, airspace.air[i].upper_border, airspace.air[i].angle, airspace.air[i].distance_m);

    	if ( airspace.air[i].upper_border == 0 ) break;

    	if ( airspace.air[i].angle < 128 )
    	    fc.airspace.max_height_m = airspace.air[i].upper_border / FC_METER_TO_FEET;
    	if ( fc.altitude1 * FC_METER_TO_FEET <= airspace.air[i].upper_border )
    	{
    		if ( fc.airspace.angle == AIRSPACE_INVALID )       // Only set the lowest airspace...
    		{
				if ( airspace.air[i].angle >= 128 )
				{
					fc.airspace.forbidden = true;
					airspace.air[i].angle -= 128;
				}
				else
				{
					fc.airspace.forbidden = false;
				}
				fc.airspace.angle = airspace.air[i].angle * 3;                          // the multiplication is used, because
				fc.airspace.distance_m = (uint16_t)airspace.air[i].distance_m * 64;     // space in the data file is saved by dividing before.
    		}
        }
    }
    if ( i == 0 )
    {
    	// No CTR.
        fc.airspace.max_height_m = 10000 / FC_METER_TO_FEET;    // FL100
        if ( fc.agl.valid && fc.agl.ground_level != AGL_INVALID )
        {
        	fc.airspace.max_height_m = max(fc.airspace.max_height_m, fc.agl.ground_level + 762);     // 762m = 2500 ft
        }
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

