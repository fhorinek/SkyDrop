/*
 * kalman.cpp
 *
 *  Created on: Mar 2, 2017
 */




#include "common.h"
#include <math.h>
#include "kalman.h"

// Tracks the position z and velocity v of an object moving in a straight line,
// (here assumed to be vertical) that is perturbed by random accelerations.
// sensor measurement of z is assumed to have constant measurement noise
// variance zVariance,
// This can be calculated offline for the specific sensor, and is supplied
// as an initialization parameter.

KalmanFilter::KalmanFilter()
{

}


void KalmanFilter::Reset(float abs_value)
{
	z_ = abs_value;
//	v_ = 0.0;
//	aBias_ = 0.0;
//	Pzz_ = 1.0f;
//	Pzv_ = 0.0f;
//	Pza_ = 0.0f;
//
//	Pvz_ = 0.0f;
//	Pvv_ = 1.0f;
//	Pva_ = 0.0f;
//
//	Paz_ = 0.0f;
//	Pav_ = 0.0;
//	//Paa_ = 100000.0f;
//	Paa_ = 1.0e10f;
}


void KalmanFilter::Configure(float zVariance, float zAccelVariance, float zAccelBiasVariance, float zInitial, float vInitial, float aBiasInitial)
{
	zAccelVariance_ = zAccelVariance;
    zAccelBiasVariance_ = zAccelBiasVariance;
	zVariance_ = zVariance;

	z_ = zInitial;
	v_ = vInitial;
	aBias_ = aBiasInitial;
	Pzz_ = 1.0f;
	Pzv_ = 0.0f;
	Pza_ = 0.0f;

	Pvz_ = 0.0f;
	Pvv_ = 1.0f;
	Pva_ = 0.0f;

	Paz_ = 0.0f;
	Pav_ = 0.0;
	//Paa_ = 100000.0f;
	Paa_ = 1.0e10f;
	}


// Updates state given a sensor measurement of z, acceleration a,
// and the time in seconds dt since the last measurement.
// 19uS on Navspark @81.84MHz
void KalmanFilter::Update(float z, float a, float dt, float* pZ, float* pV)
{

	// Predict state
    float accel = a - aBias_;
	v_ += accel * dt;
	z_ += v_ * dt;

    zAccelVariance_ = fabs(accel)/50.0f;
    zAccelVariance_ = CLAMP(zAccelVariance_, 1.0f, 50.0f);

    // Predict State Covariance matrix
	float t00,t01,t02;
    float t10,t11,t12;
    float t20,t21,t22;

    float dt2div2 = dt*dt/2.0f;
    float dt3div2 = dt2div2*dt;
    float dt4div4 = dt2div2*dt2div2;

	t00 = Pzz_ + dt*Pvz_ - dt2div2*Paz_;
	t01 = Pzv_ + dt*Pvv_ - dt2div2*Pav_;
	t02 = Pza_ + dt*Pva_ - dt2div2*Paa_;

	t10 = Pvz_ - dt*Paz_;
	t11 = Pvv_ - dt*Pav_;
	t12 = Pva_ - dt*Paa_;

	t20 = Paz_;
	t21 = Pav_;
	t22 = Paa_;

	Pzz_ = t00 + dt*t01 - dt2div2*t02;
	Pzv_ = t01 - dt*t02;
	Pza_ = t02;

	Pvz_ = t10 + dt*t11 - dt2div2*t12;
	Pvv_ = t11 - dt*t12;
	Pva_ = t12;

	Paz_ = t20 + dt*t21 - dt2div2*t22;
	Pav_ = t21 - dt*t22;
	Paa_ = t22;

    Pzz_ += dt4div4*zAccelVariance_;
    Pzv_ += dt3div2*zAccelVariance_;

    Pvz_ += dt3div2*zAccelVariance_;
    Pvv_ += dt*dt*zAccelVariance_;

    Paa_ += zAccelBiasVariance_;

	// Error
	float innov = z - z_;
	float sInv = 1.0f / (Pzz_ + zVariance_);

    // Kalman gains
	float kz = Pzz_ * sInv;
	float kv = Pvz_ * sInv;
	float ka = Paz_ * sInv;

	// Update state
	z_ += kz * innov;
	v_ += kv * innov;
	aBias_ += ka * innov;

	*pZ = z_;
	*pV = v_;

	// Update state covariance matrix
	Pzz_ -= kz * Pzz_;
	Pzv_ -= kz * Pzv_;
	Pza_ -= kz * Pza_;

	Pvz_ -= kv * Pzz_;
	Pvv_ -= kv * Pzv_;
	Pva_ -= kv * Pza_;

	Paz_ -= ka * Pzz_;
	Pav_ -= ka * Pzv_;
	Paa_ -= ka * Pza_;
}

