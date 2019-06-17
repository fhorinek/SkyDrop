/*
 * compass.cpp
 *
 *  Created on: Mar 27, 2017
 *      Author: fiala
 */

#include "compass.h"
#include "fc.h"
#include "math.h"

// Converts degrees to radians.
#define degreesToRadians(angleDegrees) (angleDegrees * M_PI / 180.0)
// Converts radians to degrees.
#define radiansToDegrees(angleRadians) (angleRadians * 180.0 / M_PI)

#define SENSOR_TO_COMPASS_OFFSET 90.0
#define LPF_Beta 0.75

float compass_atan2(float x, float y)
{	// calculates atan2, returns: angle in deg, range < 0 ; 360 ), max error 0.162 deg
	if(x < 0 and y == 0)   	return 270.0;
    if(x > 0 and y == 0)   	return 90.0;
    if(x == 0 and y == 0)   return (-1.0);//error condition
    if(x == 0 and y > 0)    return 0.0;
    if(x == 0 and y < 0)    return 180.0;

    ///arctan aproximation
    float fi = fabs( x / y );
    float fi2 = fi * fi;
    fi = ((0.596227 * fi + fi2) / (1 + 2 * 0.596227 * fi + fi2 )) * 90;

    /// cover all quadrants
    if(x >= 0 and y > 0)   	return (fi);
    if(x < 0 and y > 0)    	return (360.0 - fi);
    if(x >= 0 and y < 0)   	return (180.0 - fi);
    if(x < 0 and y < 0)    	return (fi + 180.0);
//  if(x < 0 and y == 0)   	return (fi - 180.0);
    return (-1.0); //error condition
}

void compass_init()
{

	//load declination value from EEPROM
	int16_t value;
	eeprom_busy_wait();
	eeprom_read_block(&value, &config_ro.magnetic_declination, sizeof(config_ro.magnetic_declination));
	fc.compass.declination = value;
}


void compass_calc()
{

	float quat[] = { fc.imu.quat[0], fc.imu.quat[1], fc.imu.quat[2], fc.imu.quat[3] };
	float azimut = 0;

	if (abs(fc.acc.vector.y + fc.acc.vector.z) * 2 < abs(fc.acc.vector.x)) //riser position
	{
		//roll
		float a31 =   2.0f * (quat[0] * quat[1] + quat[2] * quat[3]);
		float a33 =   quat[0] * quat[0] - quat[1] * quat[1] - quat[2] * quat[2] + quat[3] * quat[3];

		azimut = -compass_atan2(a31, a33);
	}

	//yaw
	float a12 =   2.0f * (quat[1] * quat[2] + quat[0] * quat[3]);
	float a22 =   quat[0] * quat[0] + quat[1] *quat [1] - quat[2] * quat[2] - quat[3] * quat[3];

	azimut += compass_atan2(a12, a22);

    azimut += fc.compass.declination;

    if( config.gui.disp_flags & CFG_DISP_FLIP ) //correction when device is flipped
    	azimut += 180.0;

    while(azimut > 360.0)
    	azimut -= 360.0;
    while(azimut < 0.0)
    	azimut += 360.0;

    fc.compass.azimuth_filtered = fc.compass.azimuth_filtered - (LPF_Beta * (fc.compass.azimuth_filtered - azimut));

    fc.compass.azimuth = azimut;
}

void compass_step()
{
	compass_calc();
}
