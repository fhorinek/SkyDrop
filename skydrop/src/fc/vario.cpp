#include "vario.h"
#include "kalman.h"
#include "fc.h"

KalmanFilter kalmanFilter;

//#include "../debug_on.h"

void vario_init()
{
	fc.vario.valid = false;
	fc.vario.fake = NAN;

	fc.vario.avg = 0;
	fc.vario.digital = 0;

	// This would result in QNH1 (and altitude1) as having an update looong ago, so with a big error.
	// This would mean, that QNH1 (and altitude1) will be calibrated to GPS on the first fix.
	fc.vario.time_of_last_error_update = 0;
	fc.vario.error_over_time = 100;

	for (uint8_t i = 0; i < VARIO_HISTORY_SIZE; i++)
		fc.vario.history[i] = 0;

	vario_update_history_delay();
	fc.vario.history_pointer = 0;
	fc.vario.history_step = 0;
}

void vario_update_history_delay()
{
	fc.vario.history_delay = (mul_to_sec(config.vario.avg_vario_dampening) * 100) / VARIO_HISTORY_SIZE;
}

int16_t	vario_get_altitude(uint8_t flags, uint8_t index)
{
	switch (flags & ALT_MODE_MASK)
	{
		case(ALT_ABS_QNH1):
			return fc_press_to_alt(fc.vario.pressure, config.altitude.QNH1);
		case(ALT_ABS_QNH2):
			return fc_press_to_alt(fc.vario.pressure, config.altitude.QNH2);
		case(ALT_ABS_GPS):
			if (fc.gps_data.valid)
				return fc.gps_data.altitude;
			else
				return 0;
		case(ALT_DIFF):
			uint8_t a_index = (flags & 0b00001111) + 1;
			if (a_index == 1)
				return fc.altitude1 + config.altitude.altimeter[index].delta;
			else
			{
				a_index -= 2;
				return fc.altitudes[a_index] + config.altitude.altimeter[index].delta;
			}
	}
	return 0;
}

//drop few first measurements (5s)
uint16_t vario_drop = 0;
bool kalman_ready = false;
#define VARIO_DROP	500ul

void vario_calc(float pressure)
{
	//if pressure is NaN do not compute!
	if (isnan(pressure))
	{
		DEBUG("pressure is NaN\n");
		return;
	}

	float rawAltitude = fc_press_to_alt(pressure, config.altitude.QNH1);

	//if rawAltitude is NaN do not compute!
	if (isnan(rawAltitude))
	{
		DEBUG("rawAltitude is NaN\n");
		return;
	}

	float vario, altitude;


	if (not kalman_ready)
	{
		kalmanFilter.Configure(30.0f, 4.0f, rawAltitude);
		kalman_ready = true;
	}

	if (config.vario.flags & VARIO_USE_ACC)
		kalmanFilter.Update_Propagate(rawAltitude, fc.acc.zGCA, &altitude, &vario);
	else
		kalmanFilter.Update_Propagate(rawAltitude, 0.0, &altitude, &vario);

//	DEBUG("S;%0.4f;%0.4f;", rawAltitude, fc.acc.zGCA );
//	DEBUG("%0.4f;%0.4f\n", altitude, vario);
//	kalmanFilter.Debug();
//	DEBUG("%lu;%lu;%0.2f;%ld\n", ms5611.raw_pressure, ms5611.raw_temperature, ms5611.pressure, ms5611.temperature);

	fc.vario.pressure = fc_alt_to_press(altitude, config.altitude.QNH1);

	if (!isnan(fc.vario.fake))
		vario = fc.vario.fake;

	fc.vario.vario = vario;

	if (vario_drop < VARIO_DROP)
	{
		vario_drop++;
		if (vario_drop == VARIO_DROP)
		{
			fc.vario.valid = true;
			fc.flight.autostart_altitude = altitude;
		}
		else
			return;
	}

	if (fc.vario.history_step % fc.vario.history_delay == 0)
	{
		float val = vario * VARIO_HISTORY_SCALE;
		if (val > 127) val = 127;
		if (val < -126) val = -126;
		fc.vario.history[fc.vario.history_pointer] = val;

		fc.vario.history_pointer = (fc.vario.history_pointer + 1) % VARIO_HISTORY_SIZE;
		fc.vario.history_step = 1;
	}
	else
		fc.vario.history_step++;

	//AVG vario and alt shoud start only on valid vario data
	fc.vario.digital += (vario - fc.vario.digital) * config.vario.digital_vario_dampening;
	fc.vario.avg += (vario - fc.vario.avg) * config.vario.avg_vario_dampening;

	fc.altitude1 = altitude;

	for (uint8_t i = 0; i < NUMBER_OF_ALTIMETERS; i++)
		fc.altitudes[i] = vario_get_altitude(config.altitude.altimeter[i].flags, i);
}
