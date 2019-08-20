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

#define dt 0.01
#define beta 0.1

float twoKp = (2.0f * 5.0f) ;											// 2 * proportional gain (Kp)
float twoKi = (2.0f * 0.0f) ;											// 2 * integral gain (Ki)
float integralFBx = 0.0f,  integralFBy = 0.0f, integralFBz = 0.0f;	// integral error terms scaled by Ki

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

	float q0 = fc.imu.quat[0];
	float q1 = fc.imu.quat[1];
	float q2 = fc.imu.quat[2];
	float q3 = fc.imu.quat[3];

	float invNorm;
	float q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;
	float hx, hy, bx, bz;
	float halfvx, halfvy, halfvz, halfwx, halfwy, halfwz;
	float halfex, halfey, halfez;
	float qa, qb, qc;

	bool bUseAccel = fc.acc.total > 0.75 && fc.acc.total < 1.25;

	if(bUseAccel) {
		// Normalise accelerometer measurement
		invNorm = 1.0f/sqrt(ax * ax + ay * ay + az * az);
		ax *= invNorm;
		ay *= invNorm;
		az *= invNorm;

		// Normalise magnetometer measurement
		invNorm = 1.0f/sqrt(mx * mx + my * my + mz * mz);
		mx *= invNorm;
		my *= invNorm;
		mz *= invNorm;

        // Auxiliary variables to avoid repeated arithmetic
        q0q0 = q0 * q0;
        q0q1 = q0 * q1;
        q0q2 = q0 * q2;
        q0q3 = q0 * q3;
        q1q1 = q1 * q1;
        q1q2 = q1 * q2;
        q1q3 = q1 * q3;
        q2q2 = q2 * q2;
        q2q3 = q2 * q3;
        q3q3 = q3 * q3;

        // Reference direction of Earth's magnetic field
        hx = 2.0f * (mx * (0.5f - q2q2 - q3q3) + my * (q1q2 - q0q3) + mz * (q1q3 + q0q2));
        hy = 2.0f * (mx * (q1q2 + q0q3) + my * (0.5f - q1q1 - q3q3) + mz * (q2q3 - q0q1));
        bx = sqrt(hx * hx + hy * hy);
        bz = 2.0f * (mx * (q1q3 - q0q2) + my * (q2q3 + q0q1) + mz * (0.5f - q1q1 - q2q2));

		// Estimated direction of gravity and magnetic field
		halfvx = q1q3 - q0q2;
		halfvy = q0q1 + q2q3;
		halfvz = q0q0 - 0.5f + q3q3;
        halfwx = bx * (0.5f - q2q2 - q3q3) + bz * (q1q3 - q0q2);
        halfwy = bx * (q1q2 - q0q3) + bz * (q0q1 + q2q3);
        halfwz = bx * (q0q2 + q1q3) + bz * (0.5f - q1q1 - q2q2);

		// Error is sum of cross product between estimated direction and measured direction of field vectors
		halfex = (ay * halfvz - az * halfvy) + (my * halfwz - mz * halfwy);
		halfey = (az * halfvx - ax * halfvz) + (mz * halfwx - mx * halfwz);
		halfez = (ax * halfvy - ay * halfvx) + (mx * halfwy - my * halfwx);

		// Compute and apply integral feedback if enabled
		if(twoKi > 0.0f) {
			integralFBx += twoKi * halfex * dt;	// integral error scaled by Ki
			integralFBy += twoKi * halfey * dt;
			integralFBz += twoKi * halfez * dt;
			gx += integralFBx;	// apply integral feedback
			gy += integralFBy;
			gz += integralFBz;
		}
		else {
			integralFBx = 0.0f;	// prevent integral windup
			integralFBy = 0.0f;
			integralFBz = 0.0f;
		}

		// Apply proportional feedback
		gx += twoKp * halfex;
		gy += twoKp * halfey;
		gz += twoKp * halfez;
	}

	// Integrate rate of change of quaternion
	gx *= (0.5f * dt);		// pre-multiply common factors
	gy *= (0.5f * dt);
	gz *= (0.5f * dt);
	qa = q0;
	qb = q1;
	qc = q2;
	q0 += (-qb * gx - qc * gy - q3 * gz);
	q1 += (qa * gx + qc * gz - q3 * gy);
	q2 += (qa * gy - qb * gz + q3 * gx);
	q3 += (qa * gz + qb * gy - qc * gx);

	// Normalise quaternion
	invNorm = 1.0f/sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
	q0 *= invNorm;
	q1 *= invNorm;
	q2 *= invNorm;
	q3 *= invNorm;


    fc.imu.quat[0] = q0;
    fc.imu.quat[1] = q1;
    fc.imu.quat[2] = q2;
    fc.imu.quat[3] = q3;

}


float imu_GravityCompensatedAccel(float ax, float ay, float az, volatile float* q)
{
	float za;
	za = 2.0*(q[1]*q[3] - q[0]*q[2])*ax + 2.0*(q[0]*q[1] + q[2]*q[3])*ay + (q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3])*az - 1.0;

	return (abs(za) < 0.1) ? 0 : za * 9.81f;
}

//#include "../../debug_on.h"

//uint8_t cnt;
//
//void imu_debug()
//{
//    cnt = (cnt + 1) % 5;
//    if (cnt != 0)
//    	return;
//
//	#define _180_DIV_PI         57.2957795f
//
//	float q0 = fc.imu.quat[0];
//	float q1 = fc.imu.quat[1];
//	float q2 = fc.imu.quat[2];
//	float q3 = fc.imu.quat[3];
//
//	float sinr_cosp = +2.0 * (q0 * q1 + q2 * q3);
//	float cosr_cosp = +1.0 - 2.0 * (q1 * q1 + q2 * q2);
//	float roll = atan2(sinr_cosp, cosr_cosp);
//
//	float pitch;
//
//	// pitch (y-axis rotation)
//	float sinp = +2.0 * (q0 * q2 - q3 * q1);
//	if (fabs(sinp) >= 1)
//		pitch = copysign(M_PI / 2, sinp); // use 90 degrees if out of range
//	else
//		pitch = asin(sinp);
//
//	// yaw (z-axis rotation)
//	float siny_cosp = +2.0 * (q0 * q3 + q1 * q2);
//	float cosy_cosp = +1.0 - 2.0 * (q2 * q2 + q3 * q3);
//	float yaw = atan2(siny_cosp, cosy_cosp);
//
//	yaw *= _180_DIV_PI;
//	pitch *= _180_DIV_PI;
//	roll *= _180_DIV_PI;
//
////	DEBUG("%0.2f;%0.2f;%0.2f;%0.2f;", fc.imu.quat[0], fc.imu.quat[1], fc.imu.quat[2], fc.imu.quat[3]);
//
////	DEBUG("GYRO: %7d %7d %7d\n", fc.gyro.raw.x, fc.gyro.raw.y, fc.gyro.raw.z);
////	DEBUG("ACC: %7d %7d %7d\n", fc.acc.raw.x, fc.acc.raw.y, fc.acc.raw.z);
////	DEBUG("MAG: %7d %7d %7d\n", fc.mag.raw.x, fc.mag.raw.y, fc.mag.raw.z);
//
//
////	DEBUG("%0.2f;%0.2f;%0.2f;%0.2f;%0.2f;%0.2f;%0.2f;%0.2f;%0.2f;"
////				, fc.acc.vector.x, fc.acc.vector.y, fc.acc.vector.z
////				, fc.gyro.vector.x, fc.gyro.vector.y, fc.gyro.vector.z
////				, fc.mag.vector.x, fc.mag.vector.y, fc.mag.vector.z);
//
//   	DEBUG("%0.1f;%0.1f;%0.1f;", yaw, pitch, roll);
//
//   	DEBUG("%0.3f;", fc.acc.zGCA);
//   	DEBUG("\n");
//}

void imu_step()
{
	imu_MadgwickQuaternionUpdate();
	fc.acc.zGCA = imu_GravityCompensatedAccel(fc.acc.vector.x, fc.acc.vector.y, fc.acc.vector.z, fc.imu.quat );

//	imu_debug();
}



