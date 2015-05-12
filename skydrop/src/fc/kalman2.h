#ifndef KALMAN_FILTER_H_
#define KALMAN_FILTER_H_


class KalmanFilter2 {

public :

	KalmanFilter2();
void Configure(float zVariance, float zAccelVariance, float zAccelBiasVariance, float zInitial, float vInitial, float aBiasInitial);
void Update(float z, float a, float dt, float* pZ, float* pV);

private :

// State being tracked
	float z_;  // position
	float v_;  // velocity
	float aBias_;  // acceleration

// 3x3 State Covariance matrix
	float Pzz_;
	float Pzv_;
	float Pza_;
	float Pvz_;
	float Pvv_;
	float Pva_;
	float Paz_;
	float Pav_;
	float Paa_;

    float zAccelBiasVariance_; // assumed fixed.
	float zAccelVariance_;  // dynamic acceleration variance
	float zVariance_; //  z measurement noise variance fixed

};

void imu_MadgwickQuaternionUpdate(int bUseAccel, float dt, float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz);
float imu_GravityCompensatedAccel(float ax, float ay, float az, float* q);

extern float quat[4];

void imu10dof01_mahonyUpdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
void imu10dof01_getQuaternion(float *qw, float *qx, float *qy, float *qz);
void imu10dof01_getRollPitchYaw(float *roll, float *pitch, float *yaw);

#endif
