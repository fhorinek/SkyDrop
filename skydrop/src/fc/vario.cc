#include "vario.h"

#include "fc.h"
#include "kalman.h"

KalmanFilter * kalmanFilter;

void vario_init()
{
	kalmanFilter = new KalmanFilter(1.0);
	flight_data.baro_valid = false;
	flight_data.avg_vario = 0;
	flight_data.digital_vario = 0;
}

int16_t	vario_get_altitude(uint8_t flags, uint8_t index)
{
	switch (flags)
	{
	case(ALT_ABS_QNH1):
		return fc_press_to_alt(flight_data.pressure, flight_data.QNH1);
	case(ALT_ABS_QNH2):
		return fc_press_to_alt(flight_data.pressure, flight_data.QNH2);
	default:
		if (flags & ALT_DIFF)
		{
			uint8_t a_index = flags & 0x0F;
			if (a_index == 1)
				return flight_data.altitude1 + flight_data.altimeter[index].delta;
			else
			{
				a_index -= 2;
				return flight_data.altimeter[a_index].altitude + flight_data.altimeter[index].delta;
			}
		}

	}
}

//drop few first measurements (12s)
uint16_t vario_drop = 0;
#define VARIO_DROP	1200ul

void vario_calc(float pressure)
{
	float rawAltitude = fc_press_to_alt(pressure, flight_data.QNH1);

	kalmanFilter->update(rawAltitude);

	float vario = kalmanFilter->getXVel();
	float altitude = kalmanFilter->getXAbs();

	flight_data.pressure = fc_alt_to_press(altitude, flight_data.QNH1);

	flight_data.vario = vario;

	if (vario_drop < VARIO_DROP)
	{
		vario_drop++;
		if (vario_drop == VARIO_DROP)
			flight_data.baro_valid = true;
		return;
	}
	//AVG vario and alt shoud start only on valid vario data
	flight_data.digital_vario += (vario - flight_data.digital_vario) * flight_data.digital_vario_dampening;
	flight_data.avg_vario += (vario - flight_data.avg_vario) * flight_data.avg_vario_dampening;

	flight_data.altitude1 = altitude;

	for (uint8_t i = 0; i < NUMBER_OF_ALTIMETERS; i++)
		flight_data.altimeter[i].altitude = vario_get_altitude(flight_data.altimeter[i].flags, i);
}
