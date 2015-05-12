#include "common.h"
#include <math.h>
#include "kalman2.h"

// Tracks the position z and velocity v of an object moving in a straight line,
// (here assumed to be vertical) that is perturbed by random accelerations.
// sensor measurement of z is assumed to have constant measurement noise
// variance zVariance,
// This can be calculated offline for the specific sensor, and is supplied
// as an initialization parameter.

KalmanFilter2::KalmanFilter2() {
}

void KalmanFilter2::Configure(float zVariance, float zAccelVariance, float zAccelBiasVariance, float zInitial, float vInitial, float aBiasInitial) {
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
	Paa_ = 100000.0f;
	}


// Updates state given a sensor measurement of z, acceleration a,
// and the time in seconds dt since the last measurement.
// 19uS on Navspark @81.84MHz
void KalmanFilter2::Update(float z, float a, float dt, float* pZ, float* pV) {

	// Predict state
    float accel = a - aBias_;
	v_ += accel * dt;
	z_ += v_ * dt;

    zAccelVariance_ = fabs(accel)/50.0f;
    if (zAccelVariance_ < 1.0f)
    	zAccelVariance_ = 1.0f;
    else if (zAccelVariance_ > 50.0f)
    	zAccelVariance_ = 50.0f;

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

float quat[4] = {1.0f,0.0f,0.0f,0.0f};
float beta = 0.6f;//0.6

void imu_MadgwickQuaternionUpdate(int bUseAccel, float dt, float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz)        {
            float q1 = quat[0], q2 = quat[1], q3 = quat[2], q4 = quat[3];
            float norm;
            float hx, hy, _2bx, _2bz;
            float s1, s2, s3, s4;
            float qDot1, qDot2, qDot3, qDot4;

            // Compute rate of change of quaternion
            qDot1 = 0.5f * (-q2 * gx - q3 * gy - q4 * gz);
            qDot2 = 0.5f * (q1 * gx + q3 * gz - q4 * gy);
            qDot3 = 0.5f * (q1 * gy - q2 * gz + q4 * gx);
            qDot4 = 0.5f * (q1 * gz + q2 * gy - q3 * gx);

            if (bUseAccel) {
				// Auxiliary variables to avoid repeated arithmetic
				float _2q1mx;
				float _2q1my;
				float _2q1mz;
				float _2q2mx;
				float _4bx;
				float _4bz;
				float _2q1 = 2.0f * q1;
				float _2q2 = 2.0f * q2;
				float _2q3 = 2.0f * q3;
				float _2q4 = 2.0f * q4;
				float _2q1q3 = 2.0f * q1 * q3;
				float _2q3q4 = 2.0f * q3 * q4;
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
				if (norm == 0.0f) return; // handle NaN
				norm = 1.0f/norm;
				ax *= norm;
				ay *= norm;
				az *= norm;

				// Normalise magnetometer measurement
				norm = sqrt(mx * mx + my * my + mz * mz);
				if (norm == 0.0f) return; // handle NaN
				norm = 1.0f/norm;
				mx *= norm;
				my *= norm;
				mz *= norm;

				// Reference direction of Earth's magnetic field
				_2q1mx = 2.0f * q1 * mx;
				_2q1my = 2.0f * q1 * my;
				_2q1mz = 2.0f * q1 * mz;
				_2q2mx = 2.0f * q2 * mx;
				hx = mx * q1q1 - _2q1my * q4 + _2q1mz * q3 + mx * q2q2 + _2q2 * my * q3 + _2q2 * mz * q4 - mx * q3q3 - mx * q4q4;
				hy = _2q1mx * q4 + my * q1q1 - _2q1mz * q2 + _2q2mx * q3 - my * q2q2 + my * q3q3 + _2q3 * mz * q4 - my * q4q4;
				_2bx = sqrt(hx * hx + hy * hy);
				_2bz = -_2q1mx * q3 + _2q1my * q2 + mz * q1q1 + _2q2mx * q4 - mz * q2q2 + _2q3 * my * q4 - mz * q3q3 + mz * q4q4;
				_4bx = 2.0f * _2bx;
				_4bz = 2.0f * _2bz;

				// Gradient descent algorithm corrective step
				s1 = -_2q3 * (2.0f * q2q4 - _2q1q3 - ax) + _2q2 * (2.0f * q1q2 + _2q3q4 - ay) - _2bz * q3 * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q4 + _2bz * q2) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q3 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
				s2 = _2q4 * (2.0f * q2q4 - _2q1q3 - ax) + _2q1 * (2.0f * q1q2 + _2q3q4 - ay) - 4.0f * q2 * (1.0f - 2.0f * q2q2 - 2.0f * q3q3 - az) + _2bz * q4 * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q3 + _2bz * q1) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q4 - _4bz * q2) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
				s3 = -_2q1 * (2.0f * q2q4 - _2q1q3 - ax) + _2q4 * (2.0f * q1q2 + _2q3q4 - ay) - 4.0f * q3 * (1.0f - 2.0f * q2q2 - 2.0f * q3q3 - az) + (-_4bx * q3 - _2bz * q1) * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q2 + _2bz * q4) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q1 - _4bz * q3) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
				s4 = _2q2 * (2.0f * q2q4 - _2q1q3 - ax) + _2q3 * (2.0f * q1q2 + _2q3q4 - ay) + (-_4bx * q4 + _2bz * q2) * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q1 + _2bz * q3) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q2 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
				norm = sqrt(s1 * s1 + s2 * s2 + s3 * s3 + s4 * s4);    // normalise step magnitude
				norm = 1.0f/norm;
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
            norm = sqrt(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4);    // normalise quaternion
            norm = 1.0f/norm;
            quat[0] = q1 * norm;
            quat[1] = q2 * norm;
            quat[2] = q3 * norm;
            quat[3] = q4 * norm;

        }



float imu_GravityCompensatedAccel(float ax, float ay, float az, float* q) {
   float za;
    za = 2.0f*(q[1]*q[3] - q[0]*q[2])*ax + 2.0f*(q[0]*q[1] + q[2]*q[3])*ay +
(q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3])*az - 1.0f;
    return za;
    }



















volatile float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;
volatile float integralFBx = 0.0f,  integralFBy = 0.0f, integralFBz = 0.0f;


#define imu10dof01_sampleFreq 100.0f // sample frequency in Hz
#define imu10dof01_twoKpDef (2.0f * 0.6f) // 2 * proportional gain
#define imu10dof01_twoKiDef (2.0f * 0.3f) // 2 * integral gain


/*
 * Mahony update function (for 9DOF)
 */
void imu10dof01_mahonyUpdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz) {
	float norm;
    float q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;
	float hx, hy, bx, bz;
	float halfvx, halfvy, halfvz, halfwx, halfwy, halfwz;
	float halfex, halfey, halfez;
	float qa, qb, qc;

	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
	if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {

		// Normalise accelerometer measurement
		norm = sqrt(ax * ax + ay * ay + az * az);
		ax /= norm;
		ay /= norm;
		az /= norm;

		//if magnetometer measurement invalid process only using accellerometer and gyroscope
		if((mx == 0.0f) && (my == 0.0f) && (mz == 0.0f)) {
			// Estimated direction of gravity and vector perpendicular to magnetic flux
			halfvx = q1 * q3 - q0 * q2;
			halfvy = q0 * q1 + q2 * q3;
			halfvz = q0 * q0 - 0.5f + q3 * q3;

			// Error is sum of cross product between estimated and measured direction of gravity
			halfex = (ay * halfvz - az * halfvy);
			halfey = (az * halfvx - ax * halfvz);
			halfez = (ax * halfvy - ay * halfvx);
		} else {
			// Normalise magnetometer measurement
			norm = sqrt(mx * mx + my * my + mz * mz);
			mx /= norm;
			my /= norm;
			mz /= norm;

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
		}

		// Compute and apply integral feedback if enabled
		if(imu10dof01_twoKiDef > 0.0f) {
			integralFBx += imu10dof01_twoKiDef * halfex * (1.0f / imu10dof01_sampleFreq); // integral error scaled by Ki
			integralFBy += imu10dof01_twoKiDef * halfey * (1.0f / imu10dof01_sampleFreq);
			integralFBz += imu10dof01_twoKiDef * halfez * (1.0f / imu10dof01_sampleFreq);
			gx += integralFBx;	// apply integral feedback
			gy += integralFBy;
			gz += integralFBz;
		} else {
			integralFBx = 0.0f;	// prevent integral windup
			integralFBy = 0.0f;
			integralFBz = 0.0f;
		}

		// Apply proportional feedback
		gx += imu10dof01_twoKpDef * halfex;
		gy += imu10dof01_twoKpDef * halfey;
		gz += imu10dof01_twoKpDef * halfez;
	}

	// Integrate rate of change of quaternion
	gx *= (0.5f * (1.0f / imu10dof01_sampleFreq)); // pre-multiply common factors
	gy *= (0.5f * (1.0f / imu10dof01_sampleFreq));
	gz *= (0.5f * (1.0f / imu10dof01_sampleFreq));
	qa = q0;
	qb = q1;
	qc = q2;
	q0 += (-qb * gx - qc * gy - q3 * gz);
	q1 += (qa * gx + qc * gz - q3 * gy);
	q2 += (qa * gy - qb * gz + q3 * gx);
	q3 += (qa * gz + qb * gy - qc * gx);

	// Normalise quaternion
	norm = sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
	q0 /= norm;
	q1 /= norm;
	q2 /= norm;
	q3 /= norm;
}

void imu10dof01_getQuaternion(float *qw, float *qx, float *qy, float *qz) {
	*qw = q0;
	*qx = q1;
	*qy = q2;
	*qz = q3;
}

void imu10dof01_getRollPitchYaw(float *roll, float *pitch, float *yaw) {
    *yaw = atan2(2*q1*q2 - 2*q0*q3, 2*q0*q0 + 2*q1*q1 - 1);
    *pitch = -asin(2*q1*q3 + 2*q0*q2);
    *roll = atan2(2*q2*q3 - 2*q0*q1, 2*q0*q0 + 2*q3*q3 - 1);
}
