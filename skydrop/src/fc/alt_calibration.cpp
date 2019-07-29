#include "alt_calibration.h"
#include "fc.h"
#include "agl.h"

/**
 * We have a new GPS position and try to calibrate the ALT1 value.
 * We check different height sources: get their height and their associated height error.
 * At the end of the function use the best height source if the error is better than the existing error.
 *
 */
void alt_calibration_step()
{
    //if there is no gps fix, we do not have any data
    if (!(fc.gps_data.new_sample & FC_GPS_NEW_SAMPLE_ALT))
        return;

    fc.gps_data.new_sample &= ~FC_GPS_NEW_SAMPLE_ALT;


    enum BEST_ALT_SOURCE
    {
        BH_NONE, BH_HAGL, BH_GPS, BH_ALT1
    };

    float height_min_error = FLT_MAX;
    float best_altitude = fc.altitude1;
    float error;
    enum BEST_ALT_SOURCE best_altitude_source = BH_NONE;

    //
    // Option 1: Try vario height from barometer
    //
    if (fc.vario.valid)
    {
        // Because QNH1 changes all the time, the resulting altitude1 gets wrong over time.
        // We assume, that the error is VARIO_ERROR_PER_HOUR meter per hour.
        // This means, that with every hour going by, the altitude1 is assumed to be wrong
        // by VARIO_ERROR_PER_HOUR meters.
        #define VARIO_ERROR_PER_HOUR 30.0               // must be float!
        #define VARIO_ERROR_PER_MS  (VARIO_ERROR_PER_HOUR / (3600.0 * 1000.0))

        DEBUG_1("%s:%d: fc.vario.time_of_last_error_update=%ld ", __FILE__, __LINE__, fc.vario.time_of_last_error_update);
        DEBUG_1("%s:%d: task_get_ms_tick=%ld ", __FILE__, __LINE__, task_get_ms_tick());
        DEBUG_1("%s:%d: fc.vario.error_over_time=%f\n", __FILE__, __LINE__, fc.vario.error_over_time);

        fc.vario.error_over_time += (task_get_ms_tick() - fc.vario.time_of_last_error_update) * VARIO_ERROR_PER_MS;
        fc.vario.time_of_last_error_update = task_get_ms_tick();
        DEBUG_1("%s:%d: fc.vario.error_over_time=%f\n", __FILE__, __LINE__, fc.vario.error_over_time);

        height_min_error = fc.vario.error_over_time;
        best_altitude_source = BH_ALT1;
        best_altitude = fc.altitude1;
        DEBUG("calibrate_alt/vario: best_height=%f, height_min_error=%f\n", best_altitude, height_min_error);
    }

    // Find the best height, that we have:
    if (fc.gps_data.valid)
    {
        //
        // Option 1: Try ground level
        //
        if (fc.flight.state != FLIGHT_FLIGHT && fc.agl.ground_level != AGL_INVALID)
        {
            if (fc.gps_data.groud_speed < FC_GLIDE_MIN_KNOTS)
            {
                if (fc.gps_data.hdop != 0)
                {
                    error = fc.agl.ground_gradient * fc.gps_data.hdop * L80_HACCURACY;
                    if (error < height_min_error)
                    {
                        height_min_error = error;
                        best_altitude_source = BH_HAGL;
                        best_altitude = fc.agl.ground_level;
                        DEBUG("calibrate_alt/GL: best_height=%f, height_min_error=%f\n", best_altitude, height_min_error);
                    }
                }
            }
        }

        //
        // Option 2: Try GPS height
        //
        error = fc.gps_data.vdop * L80_VACCURACY;
        if (fc.gps_data.vdop != 0 && error < height_min_error)
        {
            height_min_error = error;
            best_altitude_source = BH_GPS;
            best_altitude = fc.gps_data.altitude;
            DEBUG("calibrate_alt/GPS height: best_height=%f, height_min_error=%f\n", best_altitude, height_min_error);
        }
    }

    DEBUG_1("calibrate_alt/Best altitude: now=%fm previous=%fm source=%d error=%fm\n", best_altitude, fc.altitude1, best_altitude_source, height_min_error);

    float height_delta = best_altitude - fc.altitude1;

    if (abs(height_delta) > 10 || height_min_error < fc.vario.error_over_time - 10)
    {
        DEBUG("Calibrating altitude: now=%fm previous=%fm source=%d error=%fm\n", best_altitude, fc.altitude1, best_altitude_source, height_min_error);
//        char message[80];
//        sprintf_P(message, PSTR("QNH1 cal'ed:\n%0.0fm -> %0.0fm"), fc.altitude1, best_altitude);
//        gui_showmessage(message);

        if (config.altitude.alt1_flags & ALT_AUTO_GPS)
        {
            fc_manual_alt0_change(best_altitude);
            config.altitude.QNH1 = fc_alt_to_qnh(best_altitude, fc.vario.pressure);
            fc.vario.error_over_time = height_min_error;
        }

        for (uint8_t i = 0; i < NUMBER_OF_ALTIMETERS; i++)
        {
            if (config.altitude.altimeter[i].flags & ALT_AUTO_GPS)
            {
                switch (config.altitude.altimeter[i].flags & ALT_MODE_MASK)
                {
                    case(ALT_ABS_QNH1):
                        fc_manual_alt0_change(best_altitude);
                        config.altitude.QNH1 = fc_alt_to_qnh(best_altitude, fc.vario.pressure);
                    break;

                    case(ALT_ABS_QNH2):
                        config.altitude.QNH2 = fc_alt_to_qnh(best_altitude, fc.vario.pressure);
                    break;
                }
            }
        }
    }
}
