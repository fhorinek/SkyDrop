#include "fc.h"

#include "../drivers/sensors/devices.h"
#include "../drivers/uart.h"

#include "vario.h"
#include "kalman2.h"

volatile flight_data_t flight_data;

Timer fc_meas_timer;


#define Z_VARIANCE		    1000.0f
#define ZACCEL_VARIANCE	    2.0f
#define ZACCELBIAS_VARIANCE 2.0f
#define PI_DIV_180			(M_PI / 180.0)
KalmanFilter2 kal2;

void fc_init()
{
	DEBUG(" *** Flight computer init ***\n");

	//load configuration
	DEBUG("Loading data form EEPROM\n");
	eeprom_busy_wait();

	flight_data.QNH1 = eeprom_read_float(&config.altitude.QNH1);
	DEBUG("QNH1 %0.1f\n", flight_data.QNH1);

	flight_data.QNH2 = eeprom_read_float(&config.altitude.QNH2);
	DEBUG("QNH2 %0.1f\n", flight_data.QNH2);

	flight_data.digital_vario_dampening = eeprom_read_float(&config.vario.digital_vario_dampening);
	DEBUG("digital_vario_dampening %0.2f\n", flight_data.digital_vario_dampening);

	flight_data.avg_vario_dampening = eeprom_read_float(&config.vario.avg_vario_dampening);
	DEBUG("avg_vario_dampening %0.2f\n", flight_data.avg_vario_dampening);


	for (uint8_t i=0; i<NUMBER_OF_ALTIMETERS; i++)
	{
		DEBUG("altimeter[%d]\n", i);
		flight_data.altimeter[i].altitude = 0;

		flight_data.altimeter[i].flags = eeprom_read_byte(&config.altitude.altimeter[i].flags);
		DEBUG(" flags %02X\n", flight_data.altimeter[i].flags);

		eeprom_read_block((int16_t *) &flight_data.altimeter[i].delta, &config.altitude.altimeter[i].delta, sizeof(int16_t));
		DEBUG(" delta %d\n", flight_data.altimeter[i].delta);
	}

	eeprom_read_block((void *)&flight_data.mag_bias, &config.calibration.mag_bias, sizeof(vector_i16_t));
	DEBUG("mag_bias\n");
	DEBUG(" x %d\n", flight_data.mag_bias.x);
	DEBUG(" y %d\n", flight_data.mag_bias.y);
	DEBUG(" z %d\n", flight_data.mag_bias.z);

	eeprom_read_block((void *)&flight_data.mag_sensitivity, &config.calibration.mag_sensitivity, sizeof(vector_i16_t));
	DEBUG("mag_sensitivity\n");
	DEBUG(" x %d\n", flight_data.mag_sensitivity.x);
	DEBUG(" y %d\n", flight_data.mag_sensitivity.y);
	DEBUG(" z %d\n", flight_data.mag_sensitivity.z);

	eeprom_read_block((void *)&flight_data.acc_bias, &config.calibration.acc_bias, sizeof(vector_i16_t));
	DEBUG("acc_bias\n");
	DEBUG(" x %d\n", flight_data.acc_bias.x);
	DEBUG(" y %d\n", flight_data.acc_bias.y);
	DEBUG(" z %d\n", flight_data.acc_bias.z);

	eeprom_read_block((void *)&flight_data.acc_sensitivity, &config.calibration.acc_sensitivity, sizeof(vector_i16_t));
	DEBUG("acc_sensitivity\n");
	DEBUG(" x %d\n", flight_data.acc_sensitivity.x);
	DEBUG(" y %d\n", flight_data.acc_sensitivity.y);
	DEBUG(" z %d\n", flight_data.acc_sensitivity.z);


	DEBUG("\n");
	//init calculators
//	vario_init();
	kal2.Configure(Z_VARIANCE, ZACCEL_VARIANCE, ZACCELBIAS_VARIANCE, 20000,0.0f,0.0f);


	//gps_init();

	//VCC to baro, acc/mag gyro
	MEMS_POWER_ON;

	GpioSetDirection(IO0, OUTPUT);
	GpioWrite(IO0, HIGH);

	//Enable I2c pull-ups
	I2C_POWER_ON;
	//Enable I2C peripheral
	MEMS_I2C_PWR_ON;

	//stabilize power
	_delay_ms(1);

	mems_i2c.InitMaster(MEMS_I2C, 800000ul, 100, 8);
	mems_i2c.Scan();

	//Barometer
	ms5611.Init(&mems_i2c, MS5611_ADDRESS_CSB_LO);

	//Magnetometer + Accelerometer
	lsm303d_settings lsm_cfg;

	lsm_cfg.enabled = true;
	lsm_cfg.accOdr = lsm_acc_1600Hz;
	lsm_cfg.accScale = lsm_acc_16g;

	lsm_cfg.magOdr = lsm_mag_100Hz;
	lsm_cfg.magScale = lsm_mag_4g;
	lsm_cfg.magHiRes = true;

	lsm_cfg.tempEnable = false;

	//Gyro
	l3gd20_settings l3g_cfg;
	l3g_cfg.enabled = true;
	l3g_cfg.bw = l3g_50Hz;
	l3g_cfg.odr = l3g_760Hz;
	l3g_cfg.scale = l3g_2000dps;

	//XXX: do self-test?
	lsm303d.Init(&mems_i2c, lsm_cfg);
	lsm303d.Start();

	l3gd20.Init(&mems_i2c, l3g_cfg);
	l3gd20.Start();

	//Measurement timer
	FC_MEAS_TIMER_PWR_ON;

	fc_meas_timer.Init(FC_MEAS_TIMER, timer_div256); //125 == 1ms
	fc_meas_timer.SetInterruptPriority(MEDIUM);
	fc_meas_timer.EnableInterrupts(timer_overflow | timer_compareA | timer_compareB | timer_compareC);
	fc_meas_timer.SetTop(125 * 10); // == 10ms
	fc_meas_timer.SetCompare(timer_A, 100); // == 0.64ms
	fc_meas_timer.SetCompare(timer_B, 430); // == 2.7ms
	fc_meas_timer.SetCompare(timer_C, 555); // == 3.7ms
	fc_meas_timer.Start();

	DEBUG(" *** FC init done ***\n");

}

ISR(FC_MEAS_TIMER_OVF)
{
	IO0_HIGH
	ms5611.ReadPressure();
	ms5611.StartTemperature();
	lsm303d.StartReadMag(); //it takes 152us to transfer

	ms5611.CompensatePressure();
	IO0_LOW

}

ISR(FC_MEAS_TIMER_CMPA)
{
	IO0_HIGH
	lsm303d.ReadMag(&flight_data.mag_data.x, &flight_data.mag_data.y, &flight_data.mag_data.z);
	ms5611.ReadTemperature();
	ms5611.StartPressure();
	lsm303d.StartReadAccStream(16); //it take 1600us to transfer

//	vario_calc(ms5611.pressure);
	ms5611.CompensateTemperature();

	IO0_LOW
}

ISR(FC_MEAS_TIMER_CMPB)
{
	IO0_HIGH
	lsm303d.ReadAccStreamAvg(&flight_data.acc_data.x, &flight_data.acc_data.y, &flight_data.acc_data.z, 16);
	l3gd20.StartReadGyroStream(7); //it take 1000us to transfer
	IO0_LOW
}

ISR(FC_MEAS_TIMER_CMPC)
{
	IO0_HIGH
	l3gd20.ReadGyroStreamAvg(&flight_data.gyro_data.x, &flight_data.gyro_data.y, &flight_data.gyro_data.z, 7); //it take 1000us to transfer

//	DEBUG(";;;");
//	DEBUG("%d;%d;%d;", flight_data.mag_data.x, flight_data.mag_data.y, flight_data.mag_data.z);
//	DEBUG("%d;%d;%d;", flight_data.acc_data.x, flight_data.acc_data.y, flight_data.acc_data.z);
//	DEBUG("%d;%d;%d;", flight_data.gyro_data.x, flight_data.gyro_data.y, flight_data.gyro_data.z);


//	DEBUG("IN: %d\n", flight_data.mag_data.y);
//	DEBUG("1: %d\n", flight_data.mag_data.y - flight_data.mag_bias.y);
//	DEBUG("2: %0.1f\n", (float)(flight_data.mag_data.y - flight_data.mag_bias.y));

	//-Y, +X, +Z
	flight_data.mag_f.x = -(float)(flight_data.mag_data.y - flight_data.mag_bias.y) / (float)flight_data.mag_sensitivity.y;
	flight_data.mag_f.y = +(float)(flight_data.mag_data.x - flight_data.mag_bias.x) / (float)flight_data.mag_sensitivity.x;
	flight_data.mag_f.z = -(float)(flight_data.mag_data.z - flight_data.mag_bias.z) / (float)flight_data.mag_sensitivity.z;

	//-Y, +X, -Z
	flight_data.acc_f.x = -(float)(flight_data.acc_data.y - flight_data.acc_bias.y) / (float)flight_data.acc_sensitivity.y;
	flight_data.acc_f.y = +(float)(flight_data.acc_data.x - flight_data.acc_bias.x) / (float)flight_data.acc_sensitivity.x;
	flight_data.acc_f.z = -(float)(flight_data.acc_data.z - flight_data.acc_bias.z) / (float)flight_data.acc_sensitivity.z;

	//-X, -Y, +Z
	flight_data.gyro_f.x = -flight_data.gyro_data.x / (float)(0.070 * 1000);
	flight_data.gyro_f.y = -flight_data.gyro_data.y / (float)(0.070 * 1000);
	flight_data.gyro_f.z = +flight_data.gyro_data.z / (float)(0.070 * 1000);

    float fa = sqrt(flight_data.acc_f.x*flight_data.acc_f.x + flight_data.acc_f.y*flight_data.acc_f.y + flight_data.acc_f.z*flight_data.acc_f.z);
    int bUseAccel = ((fa > 0.5f) && (fa < 1.5f)) ? 1 : 0; //1:0

    imu_MadgwickQuaternionUpdate(bUseAccel, 0.1,
    		flight_data.acc_f.x, flight_data.acc_f.y, flight_data.acc_f.z,
    		flight_data.gyro_f.x * PI_DIV_180, flight_data.gyro_f.y * PI_DIV_180, flight_data.gyro_f.z * PI_DIV_180,
//    		0,0,0,
    		flight_data.mag_f.x, flight_data.mag_f.y, flight_data.mag_f.z
    		);

//    imu10dof01_mahonyUpdate(
////    		0,0,0,
//    		flight_data.gyro_f.x * PI_DIV_180, flight_data.gyro_f.y * PI_DIV_180, flight_data.gyro_f.z * PI_DIV_180,
//    		flight_data.acc_f.x, flight_data.acc_f.y, flight_data.acc_f.z,
//    		flight_data.mag_f.x, flight_data.mag_f.y, flight_data.mag_f.z
////    		0,0,0
//    		);

    float accel = 980.0f*imu_GravityCompensatedAccel(flight_data.acc_f.x, flight_data.acc_f.y, flight_data.acc_f.z, quat);
//
    float rawAltitude = fc_press_to_alt(ms5611.pressure, flight_data.QNH1) * 100;
//
    float zTrack, vTrack;
//
    kal2.Update((float)rawAltitude, accel, 0.1, &zTrack, &vTrack);


    flight_data.vario = vTrack / 100.0;
    flight_data.baro_valid = true;

//    imu10dof01_getRollPitchYaw(&quat[0], &quat[1], &quat[2]);
//    DEBUG("%0.2f;%0.2f;%0.2f;", quat[0], quat[1], quat[2]);


//    imu10dof01_getQuaternion(&quat[0], &quat[1], &quat[2], &quat[3]);
    DEBUG("%0.2f;%0.2f;%0.2f;%0.2f;", quat[0], quat[1], quat[2], quat[3]);

	DEBUG("%0.2f;%0.2f;%0.2f;", flight_data.acc_f.x, flight_data.acc_f.y, flight_data.acc_f.z);
	DEBUG("%0.2f;%0.2f;%0.2f;", flight_data.mag_f.x, flight_data.mag_f.y, flight_data.mag_f.z);
    DEBUG("%0.2f;%0.2f;%0.2f;", flight_data.gyro_f.x, flight_data.gyro_f.y, flight_data.gyro_f.z);
    DEBUG("%0.2f;%0.2f;\n", zTrack, vTrack);


	DEBUG("\n");

	IO0_LOW
}


void fc_step()
{
	//gps_step();

}

float fc_alt_to_qnh(float alt, float pressure)
{
	return pressure / pow(1.0 - (alt / 44330.0), 5.255);
}

float fc_press_to_alt(float pressure, float qnh)
{
	return 44330.0 * (1 - pow((pressure / qnh), 0.190295));
}

float fc_alt_to_press(float alt, float qnh)
{
	return qnh * pow(1.0 - (alt / 44330.0), 5.255);
}
