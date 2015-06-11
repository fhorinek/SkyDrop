#include "vario.h"

#include "fc.h"
#include "kalman.h"

KalmanFilter * kalmanFilter;

void vario_init()
{
	kalmanFilter = new KalmanFilter(1.0);
	fc.baro_valid = false;
	fc.avg_vario = 0;
	fc.digital_vario = 0;
}

int16_t	vario_get_altitude(uint8_t flags, uint8_t index)
{
	switch (flags)
	{
	case(ALT_ABS_QNH1):
		return fc_press_to_alt(fc.pressure, fc.QNH1);
	case(ALT_ABS_QNH2):
		return fc_press_to_alt(fc.pressure, fc.QNH2);
	default:
		if (flags & ALT_DIFF)
		{
			uint8_t a_index = flags & 0x0F;
			if (a_index == 1)
				return fc.altitude1 + fc.altimeter[index].delta;
			else
			{
				a_index -= 2;
				return fc.altimeter[a_index].altitude + fc.altimeter[index].delta;
			}
		}

	}
}

//drop few first measurements (12s)
uint16_t vario_drop = 0;
#define VARIO_DROP	1200ul

void vario_calc(float pressure)
{
	float rawAltitude = fc_press_to_alt(pressure, fc.QNH1);

	kalmanFilter->update(rawAltitude);

	float vario = kalmanFilter->getXVel();
	float altitude = kalmanFilter->getXAbs();

	fc.pressure = fc_alt_to_press(altitude, fc.QNH1);

	fc.vario = vario;

	if (vario_drop < VARIO_DROP)
	{
		vario_drop++;
		if (vario_drop == VARIO_DROP)
		{
			fc.baro_valid = true;
			fc.start_altitude = altitude;
		}
		else
			return;
	}

	//AVG vario and alt shoud start only on valid vario data
	fc.digital_vario += (vario - fc.digital_vario) * fc.digital_vario_dampening;
	fc.avg_vario += (vario - fc.avg_vario) * fc.avg_vario_dampening;

	fc.altitude1 = altitude;

	for (uint8_t i = 0; i < NUMBER_OF_ALTIMETERS; i++)
		fc.altimeter[i].altitude = vario_get_altitude(fc.altimeter[i].flags, i);
}
