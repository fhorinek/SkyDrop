#include "acc.h"
#include "fc.h"
#include "../common.h"

#define LPF_beta 		0.05		//low pass filter setting
#define max_hold_time	50	//0.5s	//hold time of last peak value ( 1 = 10ms )

void acc_widget_filter()
{
	float new_accel = fc.acc.total;
	if(new_accel >= fc.acc.filter_old)
	{
		fc.acc.filter_old = new_accel;
		fc.acc.filter_hold_time = 0;
	}
	else if((new_accel < fc.acc.filter_old) and (fc.acc.filter_hold_time < max_hold_time ) )
	{
		fc.acc.filter_hold_time += 1;
	}
	else
	{
		fc.acc.filter_old = (fc.acc.filter_old - (LPF_beta * (fc.acc.filter_old - new_accel)));
	}
	fc.acc.total_filtered = fc.acc.filter_old;
}


void acc_save_calibration(vector_float_t sens_vf, vector_float_t bias_vf)
{
	vector_i16_t bias;
	vector_i16_t sens;

	sens.x = int16_t(sens_vf.x * 2);
	sens.y = int16_t(sens_vf.y * 2);
	sens.z = int16_t(sens_vf.z * 2);
	bias.x = int16_t(bias_vf.x * 2);
	bias.y = int16_t(bias_vf.y * 2);
	bias.z = int16_t(bias_vf.z * 2);

	eeprom_busy_wait();
	eeprom_update_block(&bias, &config_ro.calibration.acc_bias, sizeof(config_ro.calibration.acc_bias));
	eeprom_update_block(&sens, &config_ro.calibration.acc_sensitivity, sizeof(config_ro.calibration.acc_sensitivity));

	//DEBUG("written float: bias %f %f %f sens %f %f %f\n", bias_vf.x, bias_vf.y, bias_vf.z, sens_vf.x, sens_vf.y, sens_vf.z);
	//DEBUG("written int: bias %d %d %d sens %d %d %d\n", bias.x, bias.y, bias.z, sens.x, sens.y, sens.z);
}

void acc_load_calibration(volatile vector_float_t * sens_vf, volatile vector_float_t * bias_vf)
{
	vector_i16_t bias;
	vector_i16_t sens;

	eeprom_busy_wait();
	eeprom_read_block(&bias, &config_ro.calibration.acc_bias, sizeof(config_ro.calibration.acc_bias));
	eeprom_read_block(&sens, &config_ro.calibration.acc_sensitivity, sizeof(config_ro.calibration.acc_sensitivity));

	sens_vf->x = (float(sens.x) / 2);
	sens_vf->y = (float(sens.y) / 2);
	sens_vf->z = (float(sens.z) / 2);
	bias_vf->x = (float(bias.x) / 2);
	bias_vf->y = (float(bias.y) / 2);
	bias_vf->z = (float(bias.z) / 2);

	//DEBUG("loaded float: bias %f %f %f sens %f %f %f\n", bias_vf->x, bias_vf->y, bias_vf->z, sens_vf->x, sens_vf->y, sens_vf->z);
	//DEBUG("loaded int: bias %d %d %d sens %d %d %d\n", bias.x, bias.y, bias.z, sens.x, sens.y, sens.z);
}

void acc_calc_init()
{
	fc.acc.total = 1.0;
	fc.acc.filter_hold_time = 0;
	acc_load_calibration(&fc.acc.sens, &fc.acc.bias);
}

void acc_calc_vector() //calculate real acceleration using calibration values
{
	fc.acc.vector.x = (float(fc.acc.raw.x) - fc.acc.bias.x) / fc.acc.sens.x;
	fc.acc.vector.y = (float(fc.acc.raw.y) - fc.acc.bias.y) / fc.acc.sens.y;
	fc.acc.vector.z = (float(fc.acc.raw.z) - fc.acc.bias.z) / fc.acc.sens.z;
}

void acc_calc_total()	//calculate total acceleration
{
	fc.acc.total = sqrt(fc.acc.vector.x * fc.acc.vector.x + fc.acc.vector.y * fc.acc.vector.y + fc.acc.vector.z * fc.acc.vector.z);
}
