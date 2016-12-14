#include "acc.h"
#include "fc.h"
#include "../common.h"

acc_data_t acc_data;

void accel_save_calibration(vector_float_t sens_vf, vector_float_t bias_vf)
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

void accel_load_calibration(vector_float_t * sens_vf, vector_float_t * bias_vf)
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


void accel_calc_init()
{
	acc_data.total_acc = 1.0;
	acc_data.hold_time = 0;
	accel_load_calibration( &acc_data.calibration.sens, &acc_data.calibration.bias );
	//DEBUG("accel init!\n");
}

float accel_calc_total()	//calculate total acceleration
{
	float acc_x = (float(fc.acc_data_raw.x) - acc_data.calibration.bias.x) / acc_data.calibration.sens.x;
	float acc_y = (float(fc.acc_data_raw.y) - acc_data.calibration.bias.y) / acc_data.calibration.sens.y;
	float acc_z = (float(fc.acc_data_raw.z) - acc_data.calibration.bias.z) / acc_data.calibration.sens.z;
	return ( sqrt(acc_x * acc_x + acc_y * acc_y + acc_z * acc_z) );
}
