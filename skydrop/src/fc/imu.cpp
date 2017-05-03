/*
 * imu.cpp
 *
 *  Created on: Jan 31, 2017
 *      Author: fiala
 */

#include "imu.h"
#include "fc.h"
#include "math.h"

// Converts degrees to radians.
#define degreesToRadians(angleDegrees) (angleDegrees * M_PI / 180.0)
// Converts radians to degrees.
#define radiansToDegrees(angleRadians) (angleRadians * 180.0 / M_PI)

#define dt 1.0
#define beta (sqrt(3.0 / 4.0) * degreesToRadians(10.0))
#define bUseAccel true

void imu_init()
{
	fc.imu.quat[0] = 1.0;
	fc.imu.quat[1] = 0.0;
	fc.imu.quat[2] = 0.0;
	fc.imu.quat[3] = 0.0;
}

void imu_MadgwickQuaternionUpdate()
{

	float ax = fc.acc.vector.x;
	float ay = fc.acc.vector.y;
	float az = fc.acc.vector.z;

	float gx = degreesToRadians(fc.gyro.vector.x);
	float gy = degreesToRadians(fc.gyro.vector.y);
	float gz = degreesToRadians(fc.gyro.vector.z);

	float mx = fc.mag.vector.x;
	float my = fc.mag.vector.y;
	float mz = fc.mag.vector.z;

	float q1 = fc.imu.quat[0];
	float q2 = fc.imu.quat[1];
	float q3 = fc.imu.quat[2];
	float q4 = fc.imu.quat[3];

	float norm;
    float hx, hy, _2bx, _2bz;
    float s1, s2, s3, s4;
    float qDot1, qDot2, qDot3, qDot4;

    // Compute rate of change of quaternion
    qDot1 = 0.5 * (-q2 * gx - q3 * gy - q4 * gz);
    qDot2 = 0.5 * (q1 * gx + q3 * gz - q4 * gy);
    qDot3 = 0.5 * (q1 * gy - q2 * gz + q4 * gx);
    qDot4 = 0.5 * (q1 * gz + q2 * gy - q3 * gx);

    if (bUseAccel)
    {
    	// Auxiliary variables to avoid repeated arithmetic
    	float _2q1mx;
    	float _2q1my;
    	float _2q1mz;
    	float _2q2mx;
    	float _4bx;
    	float _4bz;
    	float _2q1 = 2.0 * q1;
    	float _2q2 = 2.0 * q2;
    	float _2q3 = 2.0 * q3;
    	float _2q4 = 2.0 * q4;
    	float _2q1q3 = 2.0 * q1 * q3;
    	float _2q3q4 = 2.0 * q3 * q4;
    	float q1q1 = q1 * q1;
    	float q1q2 = q1 * q2;
    	float q1q3 = q1 * q3;
    	float q1q4 = q1 * q4;
    	float q2q2 = q2 * q2;
    	float q2q3 = q2 * q3;
    	float q2q4 = q2 * q4;
    	float q3q3 = q3 * q3;
    	float q3q4 = q3 * q4;
    	float q4q4 = q4 * q4;

    	// Normalise accelerometer measurement
    	norm = sqrt(ax * ax + ay * ay + az * az);
    	if (norm == 0.0)
    		return; // handle NaN
    	norm = 1.0 / norm;
    	ax *= norm;
    	ay *= norm;
    	az *= norm;

    	// Normalise magnetometer measurement
    	norm = sqrt(mx * mx + my * my + mz * mz);
    	if (norm == 0.0)
    		return; // handle NaN
    	norm = 1.0 / norm;
    	mx *= norm;
    	my *= norm;
    	mz *= norm;

    	// Reference direction of Earth's magnetic field
    	_2q1mx = 2.0 * q1 * mx;
    	_2q1my = 2.0 * q1 * my;
    	_2q1mz = 2.0 * q1 * mz;
    	_2q2mx = 2.0 * q2 * mx;
    	hx = mx * q1q1 - _2q1my * q4 + _2q1mz * q3 + mx * q2q2 + _2q2 * my * q3 + _2q2 * mz * q4 - mx * q3q3 - mx * q4q4;
    	hy = _2q1mx * q4 + my * q1q1 - _2q1mz * q2 + _2q2mx * q3 - my * q2q2 + my * q3q3 + _2q3 * mz * q4 - my * q4q4;
    	_2bx = sqrt(hx * hx + hy * hy);
    	_2bz = -_2q1mx * q3 + _2q1my * q2 + mz * q1q1 + _2q2mx * q4 - mz * q2q2 + _2q3 * my * q4 - mz * q3q3 + mz * q4q4;
    	_4bx = 2.0 * _2bx;
    	_4bz = 2.0 * _2bz;

    	// Gradient descent algorithm corrective step
    	s1 = -_2q3 * (2.0 * q2q4 - _2q1q3 - ax) + _2q2 * (2.0 * q1q2 + _2q3q4 - ay) - _2bz * q3 * (_2bx * (0.5 - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q4 + _2bz * q2) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q3 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5 - q2q2 - q3q3) - mz);
    	s2 = _2q4 * (2.0 * q2q4 - _2q1q3 - ax) + _2q1 * (2.0 * q1q2 + _2q3q4 - ay) - 4.0 * q2 * (1.0 - 2.0 * q2q2 - 2.0 * q3q3 - az) + _2bz * q4 * (_2bx * (0.5 - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q3 + _2bz * q1) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q4 - _4bz * q2) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5 - q2q2 - q3q3) - mz);
    	s3 = -_2q1 * (2.0 * q2q4 - _2q1q3 - ax) + _2q4 * (2.0 * q1q2 + _2q3q4 - ay) - 4.0 * q3 * (1.0 - 2.0 * q2q2 - 2.0 * q3q3 - az) + (-_4bx * q3 - _2bz * q1) * (_2bx * (0.5 - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q2 + _2bz * q4) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q1 - _4bz * q3) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5 - q2q2 - q3q3) - mz);
    	s4 = _2q2 * (2.0 * q2q4 - _2q1q3 - ax) + _2q3 * (2.0 * q1q2 + _2q3q4 - ay) + (-_4bx * q4 + _2bz * q2) * (_2bx * (0.5 - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q1 + _2bz * q3) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q2 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5 - q2q2 - q3q3) - mz);
    	norm = sqrt(s1 * s1 + s2 * s2 + s3 * s3 + s4 * s4);    // normalise step magnitude
    	norm = 1.0 / norm;
    	s1 *= norm;
    	s2 *= norm;
    	s3 *= norm;
    	s4 *= norm;

    	// Compute rate of change of quaternion
    	qDot1 -= beta * s1;
    	qDot2 -= beta * s2;
    	qDot3 -= beta * s3;
    	qDot4 -= beta * s4;
    }

    // Integrate to yield quaternion
    q1 += qDot1 * dt;
    q2 += qDot2 * dt;
    q3 += qDot3 * dt;
    q4 += qDot4 * dt;
    norm = sqrt( q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4 );    // normalise quaternion
    norm = 1.0 / norm;
    fc.imu.quat[0] = q1 * norm;
    fc.imu.quat[1] = q2 * norm;
    fc.imu.quat[2] = q3 * norm;
    fc.imu.quat[3] = q4 * norm;

}


float imu_GravityCompensatedAccel(float ax, float ay, float az, volatile float* q)
{
	float za;
	za = 2.0*(q[1]*q[3] - q[0]*q[2])*ax + 2.0*(q[0]*q[1] + q[2]*q[3])*ay + (q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3])*az - 1.0;
	return za;
    }

void imu_step()
{

	//DEBUG("#QTN# % 03.3f % 03.3f % 03.3f % 03.3f\n", fc.imu.quat[0], fc.imu.quat[1], fc.imu.quat[2], fc.imu.quat[3]);

	/*DEBUG("#AGM# % 011.5f % 011.5f % 011.5f % 011.5f % 011.5f % 011.5f % 011.5f % 011.5f % 011.5f\n"
			, fc.acc.vector.x, fc.acc.vector.y, fc.acc.vector.z
			, fc.gyro.vector.x, fc.gyro.vector.y, fc.gyro.vector.z
			, fc.mag.vector.x, fc.mag.vector.y, fc.mag.vector.z);
	 */

	imu_MadgwickQuaternionUpdate();
	fc.acc.zGCA = imu_GravityCompensatedAccel(fc.acc.vector.x, fc.acc.vector.y, fc.acc.vector.z, fc.imu.quat );

	//DEBUG("#VERT_ACC# % 05.2f\n",fc.acc.zGCA);
}



