#include "kalman.h"

// Constructor. Assumes a variance of 1.0 for the system model's
// acceleration noise input, in units per second squared.
KalmanFilter::KalmanFilter()
{
	setAccelerationVariance(1.0);
	reset();
}

// Constructor. Caller supplies the variance for the system model's
// acceleration noise input, in units per second squared.
KalmanFilter::KalmanFilter(float var_accel)
{
	setAccelerationVariance(var_accel);
	reset();
}

// The following three methods reset the filter. All of them assign a huge
// variance to the tracked absolute quantity and a var_accel variance to
// its derivative, so the very next measurement will essentially be
// copied directly into the filter. Still, we provide methods that allow
// you to specify initial settings for the filter's tracked state.

void KalmanFilter::reset()
{
	reset(0.0, 0.0);
}

void KalmanFilter::reset(float abs_value)
{
	reset(abs_value, 0.0);
}

void KalmanFilter::reset(float abs_value, float vel_value)
{
	x_abs = abs_value;
	x_vel = vel_value;
	p_abs_abs = 1.0e10;
	p_abs_vel = 0.0;
	p_vel_vel = var_accel;
}

// Sets the variance for the acceleration noise input in the system model,
// in units per second squared.
void KalmanFilter::setAccelerationVariance(float var_accel)
{
	this->var_accel = var_accel;
}

// Updates state given a sensor measurement of the absolute value of x,
// the variance of that measurement, and the interval since the last
// measurement in seconds. This interval must be greater than 0; for the
// first measurement after a reset(), it's safe to use 1.0.
void KalmanFilter::update(float z_abs)
{
	#define	var_z_abs	0.2
	#define	dt			0.01

	// Note: math is not optimized by hand. Let the compiler sort it out.
	// Predict step.
	// Update state estimate.
	x_abs += x_vel * dt;
	// Update state covariance. The last term mixes in acceleration noise.
	p_abs_abs += 2.0 * dt * p_abs_vel + dt * dt * p_vel_vel
			+ var_accel * dt * dt * dt * dt / 4.0;
	p_abs_vel += dt * p_vel_vel + var_accel * dt * dt * dt / 2.0;
	p_vel_vel += var_accel * dt * dt;

	// Update step.
	y = z_abs - x_abs;  // Innovation.
	s_inv = 1. / (p_abs_abs + var_z_abs);  // Innovation precision.
	k_abs = p_abs_abs * s_inv;  // Kalman gain
	k_vel = p_abs_vel * s_inv;
	// Update state estimate.
	x_abs += k_abs * y;
	x_vel += k_vel * y;
	// Update state covariance.
	p_vel_vel -= p_abs_vel * k_vel;
	p_abs_vel -= p_abs_vel * k_abs;
	p_abs_abs -= p_abs_abs * k_abs;
}

// Getters for the state and its covariance.
float KalmanFilter::getXAbs()
{
	return x_abs;
}

float KalmanFilter::getXVel()
{
	return x_vel;
}

float KalmanFilter::getCovAbsAbs()
{
	return p_abs_abs;
}

float KalmanFilter::getCovAbsVel()
{
	return p_abs_vel;
}

float KalmanFilter::getCovVelVel()
{
	return p_vel_vel;
}

