/**
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <robin.lilja@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. - Robin Lilja
 *
 * @file altitude_kf.h
 * @author Robin Lilja
 * @date 23 Jul 2015
 */

#ifndef _ALTITUDE_KF_H_
#define _ALTITUDE_KF_H_

#include "../common.h"

/**
 * A linear Kalman filter estimator of altitude and vertical velocity.
 */
class KalmanFilter {

public:

	/**
	 * Constructor.
	 * @param Q_accel covariance of acceleration input signal (σ^2).
	 * @param R_altitude covariance of the altitude measurement (σ^2).
	 */
	void Configure(float Q_accel, float R_altitude, float alt);
	void Update_Propagate(float altitude, float accel, float * h, float * v);
	void Reset(float alt);
	void Debug();

	/**
	 * Propagate the state.
	 * @param acceleration vertical acceleration in Earth frame (positive in the zenith direction) [m/s^2].
	 * @param dt update/sampling period [s].
	 */
	void propagate(float acceleration);

	/**
	 * State correction update. Use this method if you use multiple sensors measuring the altitude.
	 * @param altitude measurement of altitude in Earth frame (positive in the zenith direction) [m].
	 */
	void update(float altitude);

	/**
	 * Estimated vertical height or altitude in Earth frame (positive in the zenith direction) [m].
	 */
	float h;

	/**
	 * Estimated vertical velocity (positive in the zenith direction) [m/s].
	 */
	float v;

	/**
	 * Accelerometer covariance.
	 */
	float Q_accel;

	/**
	 * Altitude measurement covariance.
	 */
	float R_altitude;

private:

	/**
	 * Predicted covariance matrix 'P'.
	 */
	float P[2][2] =
	{
		{ 1.0f, 0.0f },
		{ 0.0f, 1.0f }
	};

};

#endif /* _ALTITUDE_KF_H_ */
