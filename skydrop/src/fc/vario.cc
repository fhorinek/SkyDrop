#include "vario.h"

#include "fc.h"
#include "kalman.h"

KalmanFilter * kalmanFilter;

void vario_init()
{
//	kalman_init();
	kalmanFilter = new KalmanFilter(1.0);
	flight_data.baro_valid = false;
	flight_data.avg_vario = 0;
	flight_data.digital_vario = 0;
}

//drop few first measurements (12s)
uint16_t vario_drop = 0;
#define VARIO_DROP	1200ul

void vario_calc(float pressure)
{
	float rawAltitude1 = 44330.0 * (1 - pow((pressure / flight_data.QNH1), 0.190295));
	float rawAltitude2 = 44330.0 * (1 - pow((pressure / flight_data.QNH2), 0.190295));
	float alt_delta = rawAltitude1 - rawAltitude2;

	kalmanFilter->update(rawAltitude1, 0.2, 0.01);

	float vario = kalmanFilter->getXVel();
	float altitude = kalmanFilter->getXAbs();

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
	flight_data.altitude2 = altitude +  alt_delta;
}
