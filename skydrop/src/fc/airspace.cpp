#include "airspace.h"
#include "agl.h"

#include "../drivers/storage/storage.h"

#include "debug_off.h"

//#index
//#   1000 0000 - inside (128, 0x80)
//#   -III IIII - 0 - 125 index
//#             - 126 no airspace (126, 0x7F)
//#a  1000 0000 - mode A
//#b  1000 0000 - mode B
//
//# A B
//# 0 0 mode0 offset with mul OFFSET_MUL_0
//# 0 1 mode1 offset with mul OFFSET_MUL_1
//# 1 0 mode2 offset with mul OFFSET_MUL_2
//# 1 1 mode3 offset with normalised vector
//#
//# The lower 7 bits of A or B are:
//#   -SII IIII
//#   where S is a sign bit: 1=negative, 0=positive
//#   and 6 bit with "I" are seen as a integer.
//#
//# For A this is a latitude offset and for B it is a longitude offset
//# multiplied depending on mode. This allows for different resolutions
//# depending on distance (and mode).

struct airspace_data_level_t
{
	uint8_t index;
	uint8_t a;
	uint8_t b;
};

struct airspace_data_point_t
{
	airspace_data_level_t level[AIR_LEVELS];
};

struct airspace_index_t
{
	uint16_t floor;
	uint16_t ceil;
	uint8_t airspace_class;
};

// This is the file handle to the current opened airspace file
FIL airspace_data_file;

void airspace_init()
{
    DEBUG("airspace_init\n");
    fc.airspace.file_valid = false;

    fc.airspace.airspace_name_index = AIR_INDEX_INVALID;
    fc.airspace.airspace_name[0] = 0;

	fc.airspace.min_alt = AIRSPACE_INVALID;
	fc.airspace.max_alt = AIRSPACE_INVALID;

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
 * Takes a (raw) altitude in feet and converts into feet.
 *
 * @param altitude in feet
 *
 * @return same altitude in feet
 */
uint16_t airspace_convert_alt_ft(uint16_t raw_alt)
{
	return ~AIR_AGL_FLAG & raw_alt;
}

/**
 * Takes a (raw) altitude in feet and converts into meter.
 *
 * @param altitude in feet
 *
 * @return same altitude in meter
 */
uint16_t airspace_convert_alt_m(uint16_t raw_alt)
{
	return (~AIR_AGL_FLAG & raw_alt) / FC_METER_TO_FEET;
}

//is device altitude below raw_alt?
bool airspace_alt_is_below(uint16_t raw_alt, uint16_t gps_alt, uint16_t msl_alt)
{
	if (airspace_convert_alt_m(raw_alt) > ((raw_alt & AIR_AGL_FLAG) ? gps_alt : msl_alt))
		return true;

	return false;
}

//is device altitude above raw_alt?
bool airspace_alt_is_above(uint16_t raw_alt, uint16_t gps_alt, uint16_t msl_alt)
{
	if (airspace_convert_alt_m(raw_alt) < ((raw_alt & AIR_AGL_FLAG) ? gps_alt : msl_alt))
		return true;

	return false;
}

bool airspace_is_inside(uint16_t raw_min, uint16_t raw_max, uint16_t gps_alt, uint16_t msl_alt)
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
    airspace_data_point_t as_point;
    int i;

    uint16_t rd;

    uint16_t x = int((NDIV(lon, GPS_COORD_MUL)) * AIR_RESOLUTION / GPS_COORD_MUL);
	uint16_t y = int((NDIV(lat, GPS_COORD_MUL)) * AIR_RESOLUTION / GPS_COORD_MUL);

	DEBUG("x %u\n", x);
	DEBUG("y %u\n", y);

    //seek to position
    uint32_t index = x * AIR_RESOLUTION + y;

    DEBUG("index %lu\n", index);

    if (fc.airspace.cache_index != index)
    {
    	// read data, as cache does not contain the right values.
		assert(f_lseek(&airspace_data_file, index * AIR_LEVELS * AIR_LEVEL_SIZE) == FR_OK);
		assert(f_read(&airspace_data_file, &as_point, sizeof(as_point), &rd) == FR_OK);
		assert(rd == sizeof(as_point));

		for (i = 0; i < AIR_LEVELS; i++)
		{
//			DEBUG("AS %i\n", i);
			if (as_point.level[i].index == AIR_INDEX_INVALID)
			{
				fc.airspace.cache[i].flags = 0x00;
				continue;
			}

			fc.airspace.cache[i].flags = AIR_CACHE_VALID;

			if (as_point.level[i].index & 0x80)
				fc.airspace.cache[i].flags |= AIR_CACHE_INSIDE;

			fc.airspace.cache[i].index = as_point.level[i].index & 0x7F;

			uint32_t airspace_index_pos = (uint32_t)((uint32_t)AIR_RESOLUTION * (uint32_t)AIR_RESOLUTION * (uint32_t)AIR_LEVELS * (uint32_t)AIR_LEVEL_SIZE) +
					(uint32_t)(fc.airspace.cache[i].index) * AIR_INDEX_SIZE;

			airspace_index_t as_index_data;
			assert(f_lseek(&airspace_data_file, airspace_index_pos) == FR_OK);
			assert(f_read(&airspace_data_file, &as_index_data, sizeof(as_index_data), &rd) == FR_OK);
			assert(rd == sizeof(as_index_data));

			fc.airspace.cache[i].floor = as_index_data.floor;
			fc.airspace.cache[i].ceil = as_index_data.ceil;
			fc.airspace.cache[i].airspace_class = as_index_data.airspace_class;

			uint8_t mode = (as_point.level[i].a & 0x80) >> 6 | (as_point.level[i].b & 0x80) >> 7;

			int16_t lon_n = (lon / GPS_COORD_MUL) - (lon < 0 ? 1 : 0);
			int16_t lat_n = (lat / GPS_COORD_MUL) - (lat < 0 ? 1 : 0);

			int32_t origin_lat = lat_n * GPS_COORD_MUL + (((y * 2 + 1) * (GPS_COORD_MUL / 2)) / AIR_RESOLUTION);
			int32_t origin_lon = lon_n * GPS_COORD_MUL + (((x * 2 + 1) * (GPS_COORD_MUL / 2)) / AIR_RESOLUTION);

	        DEBUG(" origin_lat: %ld\n", origin_lat);
	        DEBUG(" origin_lon: %ld\n", origin_lon);

			int8_t lat_offset = (as_point.level[i].a & 0x3F) * ((as_point.level[i].a & 0x40) ? -1 : 1);
	        int8_t lon_offset = (as_point.level[i].b & 0x3F) * ((as_point.level[i].b & 0x40) ? -1 : 1);

	        int32_t target_lat;
	        int32_t target_lon;

	        switch(mode)
	        {
				case(0):
					target_lat = origin_lat + lat_offset * OFFSET_MUL_0 * GPS_COORD_MUL;
					target_lon = origin_lon + lon_offset * OFFSET_MUL_0 * GPS_COORD_MUL;
					if ((as_point.level[i].a & 0x3F) == 0x3F || (as_point.level[i].b & 0x3F) == 0x3F)
						fc.airspace.cache[i].flags |= AIR_CACHE_FAR;
				break;
				case(1):
					target_lat = origin_lat + lat_offset * OFFSET_MUL_1 * GPS_COORD_MUL;
					target_lon = origin_lon + lon_offset * OFFSET_MUL_1 * GPS_COORD_MUL;
				break;
				case(2):
					target_lat = origin_lat + lat_offset * OFFSET_MUL_2 * GPS_COORD_MUL;
					target_lon = origin_lon + lon_offset * OFFSET_MUL_2 * GPS_COORD_MUL;
				break;
				case(3):
					target_lat = origin_lat;
					target_lon = origin_lon;
					fc.airspace.cache[i].flags |= AIR_CACHE_NORMAL;
				break;
	        }

	        fc.airspace.cache[i].latitude  = target_lat;
	        fc.airspace.cache[i].longitude = target_lon;

	        fc.airspace.cache[i].lat_offset = lat_offset;
	        fc.airspace.cache[i].lon_offset = lon_offset;

	        fc.airspace.cache_index = index;

	        DEBUG(" flags: %02X\n", fc.airspace.cache[i].flags);

	        DEBUG(" latitude: %ld\n", fc.airspace.cache[i].latitude);
	        DEBUG(" longitude: %ld\n", fc.airspace.cache[i].longitude);

	        DEBUG(" lat_offset: %d\n", fc.airspace.cache[i].lat_offset);
	        DEBUG(" lon_offset: %d\n", fc.airspace.cache[i].lon_offset);

	        DEBUG(" floor: %u %c\n", fc.airspace.cache[i].floor & 0x7FFF, fc.airspace.cache[i].floor & 0x8000 ? 'A' : 'M');
	        DEBUG(" ceil: %u %c\n", fc.airspace.cache[i].ceil & 0x7FFF, fc.airspace.cache[i].ceil & 0x8000 ? 'A' : 'M');

	        DEBUG(" index: %u\n", fc.airspace.cache[i].index);
	        DEBUG(" airspace_class: %u\n", fc.airspace.cache[i].airspace_class);
	        DEBUG("\n");
		}
    }

	fc.airspace.forbidden = false;
	fc.airspace.angle = AIRSPACE_INVALID;

	//limits
	fc.airspace.min_alt = AIRSPACE_INVALID;
	fc.airspace.max_alt = AIRSPACE_INVALID;

	//info
	fc.airspace.floor = 0;
	fc.airspace.ceiling = 0;

	uint16_t msl_alt = fc_press_to_alt(fc.vario.pressure, 101325);
	uint16_t gps_alt = fc.gps_data.altitude;

	DEBUG("msl_alt: %d\n", msl_alt);
	DEBUG("gps_alt: %d\n", gps_alt);

	uint16_t nearest_dist;
	uint8_t name_i = 0xFF;

	bool have_data = false;
	bool inside;

    for (i = 0; i < AIR_LEVELS; i++)
    {
    	if (fc.airspace.cache[i].flags == 0)
    		break; //no data

        DEBUG("level %d\n", i);

    	have_data = true;

    	int32_t tx, ty;

        if (fc.airspace.cache[i].lon_offset == 0)
        {
            ty = fc.airspace.cache[i].latitude;
            tx = lon;
        }
        else if (fc.airspace.cache[i].lat_offset == 0)
        {
            tx = fc.airspace.cache[i].longitude;
            ty = lat;
        }
        else
        {
            float k = fc.airspace.cache[i].lat_offset / (float)fc.airspace.cache[i].lon_offset;
            float kn = -fc.airspace.cache[i].lon_offset / (float)fc.airspace.cache[i].lat_offset;

            DEBUG("k %0.5f\n", k);
            DEBUG("kn %0.5f\n", kn);

            int64_t q1 = lat - k * lon;
            int64_t q2 = fc.airspace.cache[i].latitude - kn * fc.airspace.cache[i].longitude;

            DEBUG("q1 %ld\n", q1);
            DEBUG("q2 %ld\n", q2);

            tx = (-q1 + q2) / (k - kn);
            ty = k * tx + q1;
        }

//		DEBUG(" lat: %ld\n", lat);
//		DEBUG(" lon: %ld\n", lon);
//		DEBUG(" latitude: %ld\n", fc.airspace.cache[i].latitude);
//		DEBUG(" longitude: %ld\n", fc.airspace.cache[i].longitude);

        int8_t dx = (tx - lon) >= 0 ? 1 : -1;
        int8_t dy = (ty - lat) >= 0 ? 1 : -1;

//        DEBUG(" dx %d\n", dx);
//        DEBUG(" dy %d\n", dy);

        inside = (fc.airspace.cache[i].flags & AIR_CACHE_INSIDE) > 0;

        if (dx * fc.airspace.cache[i].lon_offset < 0 || dy * fc.airspace.cache[i].lat_offset < 0)
            inside = !inside;

        uint16_t angle = gps_bearing(ty, tx, lat, lon);


        uint16_t distance;

        if (fc.airspace.cache[i].flags & AIR_CACHE_FAR)
        	distance = AIRSPACE_TOO_FAR;
        else
        	distance = gps_distance_2d(lat, lon, ty, tx) / 100;

//        DEBUG(" target x %ld\n", tx);
//        DEBUG(" target y %ld\n", ty);
//        DEBUG(" inside %u\n", inside);
//        DEBUG(" angle %u\n", angle);
//        DEBUG(" dist %u km\n", distance);
//        DEBUG("\n");

        DEBUG(">>%d %ddeg %0.3fkm %c\n", i, angle, distance / 1000.0, inside ? 'I' : ' ');

    	//is inside the as_point floor and ceil
    	if (airspace_is_inside(fc.airspace.cache[i].floor, fc.airspace.cache[i].ceil, gps_alt, msl_alt))
    	{
    		//check if inside
    		if (inside)
    		{
    			fc.airspace.forbidden = true;
    			name_i = fc.airspace.cache[i].index;

    			fc.airspace.angle = angle;
    			fc.airspace.distance_m = distance;
    			fc.airspace.ceiling = fc.airspace.cache[i].ceil;
    			fc.airspace.floor = fc.airspace.cache[i].floor;
    		}
    	}

    	//arrow point to nearest or outside of forbidden
    	if (!fc.airspace.forbidden)
    	{
			if ((i == 0) || ((nearest_dist > distance) && !inside))
			{
				nearest_dist = distance;
    			name_i = fc.airspace.cache[i].index;

    			fc.airspace.angle = angle;
    			fc.airspace.distance_m = distance;
    			fc.airspace.ceiling = fc.airspace.cache[i].ceil;
    			fc.airspace.floor = fc.airspace.cache[i].floor;
			}
    	}

		//inside the as_point
		if (inside)
		{
			if (airspace_alt_is_above(fc.airspace.cache[i].floor, gps_alt, msl_alt))
			{
				if (fc.airspace.min_alt == AIRSPACE_INVALID)
					fc.airspace.min_alt = fc.airspace.cache[i].ceil;
				else
					fc.airspace.min_alt = max(fc.airspace.min_alt, fc.airspace.cache[i].ceil);
			}
			if (airspace_alt_is_below(fc.airspace.cache[i].ceil, gps_alt, msl_alt))
			{
				if (fc.airspace.max_alt == AIRSPACE_INVALID)
					fc.airspace.max_alt = fc.airspace.cache[i].floor;
				else
					fc.airspace.max_alt = min(fc.airspace.max_alt, fc.airspace.cache[i].floor);
			}
		}
    }

    if (have_data)
    {
		if (name_i != fc.airspace.airspace_name_index)
		{
			fc.airspace.airspace_name_index = name_i;

			if (name_i != AIR_INDEX_INVALID)
			{
				uint32_t airspace_index_pos = (uint32_t)((uint32_t)AIR_RESOLUTION * (uint32_t)AIR_RESOLUTION * (uint32_t)AIR_LEVELS * (uint32_t)AIR_LEVEL_SIZE) +
						(uint32_t)(name_i) * AIR_INDEX_SIZE + 5;

				assert(f_lseek(&airspace_data_file, airspace_index_pos) == FR_OK);
				assert(f_read(&airspace_data_file, (void *)fc.airspace.airspace_name, sizeof(fc.airspace.airspace_name), &rd) == FR_OK);
			}
			else
			{
				fc.airspace.airspace_name[0] = 0;
			}
		}

//		DEBUG("airspace_name_index: %u\n", name_i);
//		DEBUG("airspace_name: %s\n", fc.airspace.airspace_name);
//
//		DEBUG("forbidden: %u\n", fc.airspace.forbidden);
//		DEBUG("angle: %u\n", fc.airspace.angle);
//		DEBUG("distance_m: %u\n", fc.airspace.distance_m);
//		DEBUG("ceiling: %u\n", fc.airspace.ceiling);
//		DEBUG("floor: %u\n", fc.airspace.floor);
//		DEBUG("min_alt: %u\n", fc.airspace.min_alt);
//		DEBUG("max_alt: %u\n", fc.airspace.max_alt);
//		DEBUG("\n");
    }
    else
    {
		DEBUG("AIRSPACE_INVALID\n");
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
        	DEBUG("AIR: lat/lon: %f %f FORBIDDEN: %d ANGLE: %d DIST: %u\n", fc.gps_data.latitude * 1.0 / GPS_COORD_MUL, fc.gps_data.longtitude * 1.0 / GPS_COORD_MUL, fc.airspace.forbidden, fc.airspace.angle, fc.airspace.distance_m);
        }
    }
}

