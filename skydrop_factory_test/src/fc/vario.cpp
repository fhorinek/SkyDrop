#include "vario.h"
#include "kalman.h"
#include "fc.h"

KalmanFilter kalmanFilter;
KalmanFilter kalmanFilter_acc;

#include "../debug_on.h"

void vario_init()
{
	fc.vario.valid = false;

}

//drop few first measurements (5s)
uint16_t vario_drop = 0;
bool kalman_ready = false;
#define VARIO_DROP	500ul

#define DEFAULT_QNH		101325

void vario_calc(float pressure)
{
	//if pressure is NaN do not compute!
	if (isnan(pressure))
	{
		DEBUG("pressure is NaN\n");
		return;
	}

	float rawAltitude = fc_press_to_alt(pressure, DEFAULT_QNH);

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
		kalmanFilter_acc.Configure(30.0f, 4.0f, rawAltitude);
		kalman_ready = true;
	}


	kalmanFilter.Update_Propagate(rawAltitude, 0.0, &altitude, &vario);
	fc.vario.pressure = fc_alt_to_press(altitude, DEFAULT_QNH);
	fc.vario.vario = vario;

	kalmanFilter_acc.Update_Propagate(rawAltitude, fc.acc.zGCA, &altitude, &vario);
	fc.vario.vario_acc = vario;

	if (vario_drop < VARIO_DROP)
	{
		vario_drop++;
		if (vario_drop == VARIO_DROP)
		{
			fc.vario.valid = true;
		}
		else
			return;
	}
}
