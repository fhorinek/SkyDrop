/*
 * kalman.h
 *
 *  Created on: 4.3.2015
 *      Author: horinek
 */

#ifndef KALMAN_H_
#define KALMAN_H_


class KalmanFilter {
public:
    // The state we are tracking, namely:
    float x_abs;  // The absolute value of x.
    float x_vel;  // The rate of change of x.

    // Covariance matrix for the state.
    float p_abs_abs;
    float p_abs_vel;
    float p_vel_vel;

    // The variance of the acceleration noise input in the system model.
    float var_accel;

    //calculation variables
    float y;
    float s_inv;
    float k_abs;
    float k_vel;

    KalmanFilter();
    KalmanFilter(float var_accel);
    void reset();
    void reset(float abs_value);
    void reset(float abs_value, float vel_value);
    void setAccelerationVariance(float var_accel);
    void update(float z_abs, float var_z_abs, float dt);
    float getXAbs();
    float getXVel();
    float getCovAbsAbs();
    float getCovAbsVel();
    float getCovVelVel();
};

#endif /* KALMAN_H_ */
