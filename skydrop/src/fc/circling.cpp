
#include "circling.h"
#include "fc.h"

// #include "../debug_on.h"

void circling_reset()
{
    fc.flight.total_heading_change = 0;
    fc.flight.avg_heading_change = 0;
    fc.flight.last_heading = 0;

    fc.flight.circling = false;
    fc.flight.circling_stop = 0;
    fc.flight.circling_time = 0;
    fc.flight.circling_gain = 0;

	for (uint8_t i = 0; i < 8; i++)
		fc.flight.circling_history[i] = 0;
}

void circling_step()
{
	if (fc.vario.valid && fc.gps_data.valid)
	{
		uint8_t cindex = (((fc.gps_data.heading + 360 - 22) / 45) + 1) % 8;

		float val = fc.vario.vario * VARIO_CIRCLING_HISTORY_SCALE;
		if (val > 127) val = 127;
		if (val < -126) val = -126;

//		DEBUG("ch %u %u %i\n", fc.gps_data.heading, cindex, int8_t(val));
		fc.flight.circling_history[cindex] = int8_t(val);
	}

	if (fc.gps_data.new_sample & FC_GPS_NEW_SAMPLE_CIRCLE)
	{
	    fc.gps_data.new_sample &= ~FC_GPS_NEW_SAMPLE_CIRCLE;

	    int16_t heading_change = fc.gps_data.heading - fc.flight.last_heading;
	    if (heading_change > 180)
	    	heading_change -= 360;
	    if (heading_change < -180)
	    	heading_change += 360;

        fc.flight.last_heading = fc.gps_data.heading;

//        DEBUG("\nhch %d\n", heading_change);


        fc.flight.avg_heading_change += (heading_change - fc.flight.avg_heading_change) / PAGE_AUTOSET_CIRCLING_AVG;
        fc.flight.total_heading_change += heading_change;

        //if avg have different sign than total zero the total
        if (fc.flight.avg_heading_change > 0 && fc.flight.total_heading_change < 0)
        	fc.flight.total_heading_change = 0;
        else if (fc.flight.avg_heading_change < 0 && fc.flight.total_heading_change > 0)
        	fc.flight.total_heading_change = 0;

        fc.flight.total_heading_change = CLAMP(fc.flight.total_heading_change, -400, 400);

//        DEBUG("ach %0.2f\n", fc.flight.avg_heading_change);
//        DEBUG("tch %d\n", fc.flight.total_heading_change);
//        DEBUG("cr %d\n", fc.flight.circling);
	}
}
