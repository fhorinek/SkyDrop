#include "agl.h"

#include "../drivers/storage/storage.h"

// This is the file handle to the current opened HGT file
FIL agl_data_file;

// The SRMT HGT file format is described in detail in
//     http://dds.cr.usgs.gov/srtm/version2_1/Documentation/SRTM_Topo.pdf

// Some HGT files contain 1201 x 1201 points (3 arc/90m resolution)
#define HGT_DATA_WIDTH_3	1201ul

// Some HGT files contain 3601 x 3601 points (1 arc/30m resolution)
#define HGT_DATA_WIDTH_1	3601ul

// Some HGT files contain 3601 x 1801 points (1 arc/30m resolution)
#define HGT_DATA_WIDTH_1_HALF	1801ul

void agl_init()
{
    DEBUG("agl_init\n");
    fc.agl.file_valid = false;

    fc.agl.ground_level = AGL_INVALID;
    memset((void *) fc.agl.filename, 0, sizeof(fc.agl.filename));
}

void agl_get_filename(char * fn, int32_t lat, int32_t lon)
{
    char lat_c, lon_c;

    uint8_t lat_n = abs(lat / GPS_COORD_MUL);
    uint16_t lon_n = abs(lon / GPS_COORD_MUL);

    if (lat >= 0)
    {
        lat_c = 'N';
    }
    else
    {
        lat_c = 'S';
        lat_n++;
    }
    if (lon >= 0)
    {
        lon_c = 'E';
    }
    else
    {
        lon_c = 'W';
        lon_n++;
    }

    sprintf_P(fn, PSTR("%c%02u%c%03u"), lat_c, lat_n, lon_c, lon_n);
}

void agl_open_file(char * fn)
{
    //copy filename to fc struct
    strcpy((char *) fc.agl.filename, fn);

    char path[20];
    sprintf_P(path, PSTR("/AGL/%s.HGT"), fc.agl.filename);

    DEBUG("AGL data path is '%s'\n", path);

    if (fc.agl.file_valid)
        assert(f_close(&agl_data_file) == FR_OK);

    uint8_t ret = f_open(&agl_data_file, path, FA_READ);

    if (ret == FR_OK)
    {
        //file is valid
        fc.agl.file_valid = true;
    }
    else
    {
        //file is not available
        fc.agl.file_valid = false;
        fc.agl.ground_level = AGL_INVALID;
        DEBUG("AGL file not found.\n");
    }

}

/**
 * Return the height of ground level for the given lat/lon above sea level.
 * This function assumes, that the right file containing lat/lon is already
 * opened and accessible by agl_data_file.
 *
 * @param lat the latitude of the point
 * @param lon the longitude of the point
 *
 * @return the height of the ground above sea level in m or AGL_INVALID
 */
int16_t agl_get_alt_on_opened_file(int32_t lat, int32_t lon)
{
    uint16_t num_points_x;
    uint16_t num_points_y;
    int16_t alt;

    if (lon < 0)
    {
        // we do not care above degree, only minutes are important
        // reverse the value, because file goes in opposite direction.
        lon = (GPS_COORD_MUL - 1) + (lon % GPS_COORD_MUL);   // lon is negative!
    }
    if (lat < 0)
    {
        // we do not care above degree, only minutes are important
        // reverse the value, because file goes in opposite direction.
        lat = (GPS_COORD_MUL - 1) + (lat % GPS_COORD_MUL);   // lat is negative!
    }

    // Check, if we have a 1201x1201 or 3601x3601 tile:
    switch (f_size(&agl_data_file))
    {
        case HGT_DATA_WIDTH_3 * HGT_DATA_WIDTH_3 * 2:
            num_points_x = num_points_y = HGT_DATA_WIDTH_3;
        break;
        case HGT_DATA_WIDTH_1 * HGT_DATA_WIDTH_1 * 2:
            num_points_x = num_points_y = HGT_DATA_WIDTH_1;
        break;
        case HGT_DATA_WIDTH_1 * HGT_DATA_WIDTH_1_HALF * 2:
            num_points_x = HGT_DATA_WIDTH_1_HALF;
            num_points_y = HGT_DATA_WIDTH_1;
        break;
        default:
            return AGL_INVALID;
    }

    // "-2" is, because a file has a overlap of 1 point to the next file.
    uint32_t coord_div_x = GPS_COORD_MUL / (num_points_x - 2);
    uint32_t coord_div_y = GPS_COORD_MUL / (num_points_y - 2);
    uint16_t y = (lat % GPS_COORD_MUL) / coord_div_y;
    uint16_t x = (lon % GPS_COORD_MUL) / coord_div_x;

    uint16_t rd;
    uint8_t tmp[4];
    byte2 alt11, alt12, alt21, alt22;

    //seek to position
    uint32_t pos = ((uint32_t) x + num_points_x * (uint32_t) ((num_points_y - y) - 1)) * 2;
    DEBUG("agl_get_alt_on_opened_file: lat=%ld, lon=%ld; x=%d, y=%d; pos=%ld\n", lat, lon, x, y, pos);

    assert(f_lseek(&agl_data_file, pos) == FR_OK);
    assert(f_read(&agl_data_file, tmp, 4, &rd) == FR_OK);
    assert(rd == 4);

    //switch big endian to little
    alt11.uint8[0] = tmp[1];
    alt11.uint8[1] = tmp[0];

    alt21.uint8[0] = tmp[3];
    alt21.uint8[1] = tmp[2];

    //seek to opposite position
    pos -= num_points_x * 2;
    assert(f_lseek(&agl_data_file, pos) == FR_OK);
    assert(f_read(&agl_data_file, tmp, 4, &rd) == FR_OK);
    assert(rd == 4);

    //switch big endian to little
    alt12.uint8[0] = tmp[1];
    alt12.uint8[1] = tmp[0];

    alt22.uint8[0] = tmp[3];
    alt22.uint8[1] = tmp[2];

    //get point displacement
    float lat_dr = ((lat % GPS_COORD_MUL) % coord_div_y) / float(coord_div_y);
    float lon_dr = ((lon % GPS_COORD_MUL) % coord_div_x) / float(coord_div_x);

    //compute height by using bilinear interpolation
    float alt1 = alt11.int16 + float(alt12.int16 - alt11.int16) * lat_dr;
    float alt2 = alt21.int16 + float(alt22.int16 - alt21.int16) * lat_dr;

    alt = alt1 + float(alt2 - alt1) * lon_dr;
    DEBUG_1("alt11=%d, alt21=%d, alt12=%d, alt22=%d, alt=%d\n", alt11.int16, alt21.int16, alt12.int16, alt22.int16, alt);

    /* Find the minimum altitude: */
    int16_t alt_min = min(alt11.int16, alt12.int16);
    alt_min = min(alt_min, alt21.int16);
    alt_min = min(alt_min, alt22.int16);

    /* Find the maximum altitude: */
    int16_t alt_max = max(alt11.int16, alt12.int16);
    alt_max = max(alt_max, alt21.int16);
    alt_max = max(alt_max, alt22.int16);

    /*
     * Compute the gradient of the area. This is done in a very simple (and not 100% correct way):
     * We simply take the ground level difference and the width of 1 degree, which is 111.3km.
     * We do not take into account, at which edge the minimum and maximum is and we do not care
     * about the fact, that 1 degree of lat is not always 111.3km and also do not take care of
     * coord_div_x != coord_div_y for some cases. So we only get a very rough idea of the gradient.
     *
     * However, the mistake which we do is quite constant, if we stay roughly at the same position
     * on earth. So it should be good enough for our needs.
     */
    fc.agl.ground_gradient = (alt_max - alt_min) / (111300.0 / num_points_x);

    return alt;
}

/**
 * Return the height of ground level for the given lat/lon above sea level.
 *
 * @param lat the latitude of the point
 * @param lon the longitude of the point
 *
 * @return the height of the ground above sea level in m or AGL_INVALID
 *
 * The correctness of this method can be checked with
 * http://api.geonames.org/srtm1?lat=47.59272333333333333333&lng=7.65589666666666666666&username=demo
 *
 * The access to the provided HGT files can be checked with
 * gdallocationinfo -wgs84 N47E007.HGT 7.65589666666666666666 47.59272333333333333333
 *   Location: (2361P,1466L), Value: 285
 */
int16_t agl_get_alt(int32_t latitude, int32_t longtitude)
{
    char tmp_name[10];

    if (!storage_ready())
        return AGL_INVALID;

    agl_get_filename(tmp_name, latitude, longtitude);

    if (strcmp(tmp_name, (char *) fc.agl.filename) != 0) //data file is different then previous
        agl_open_file(tmp_name);

    if (!fc.agl.file_valid) //file was not found
        return AGL_INVALID;

    return agl_get_alt_on_opened_file(fc.gps_data.latitude, fc.gps_data.longtitude);
}

void agl_step()
{
    if (fc.gps_data.new_sample & FC_GPS_NEW_SAMPLE_AGL)
    {
        fc.gps_data.new_sample &= ~FC_GPS_NEW_SAMPLE_AGL;

        fc.agl.ground_gradient = AGL_INVALID;
        fc.agl.ground_level = agl_get_alt(fc.gps_data.latitude, fc.gps_data.longtitude);
        if (fc.agl.ground_level != AGL_INVALID)
            DEBUG("AGL: lat/lon: %f %f GL: %d GL_GRAD: %f\n", fc.gps_data.latitude * 1.0 / GPS_COORD_MUL, fc.gps_data.longtitude * 1.0 / GPS_COORD_MUL, fc.agl.ground_level, fc.agl.ground_gradient);
    }
}

