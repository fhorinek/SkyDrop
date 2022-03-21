/*
 * sht21.H
 *
 *  Created on: 29.11.2013
 *      Author: horinek
 */

#ifndef SHT21_H_
#define SHT21_H_

class SHT21;

#include "../../common.h"

struct sht21_settings
{
	bool temp_enabled;
	bool rh_enabled;
};

#define SHT21_ADDRESS			0x40

#define SHT21_READ_USER			0b11100111
#define SHT21_WRITE_USER		0b11100110

//we are using only no hold
#define SHT21_MEASURE_T			0b11110011
#define SHT21_MEASURE_RH		0b11110101

#define SHT21_RESET				0b11111110

class SHT21
{
public:
	struct sht21_settings settings;

	I2c * i2c;
	bool present;

	uint16_t raw_temperature;
	uint16_t raw_humidity;

	int16_t temperature;
	int16_t humidity;

	void Init(I2c * i2c, struct sht21_settings settings);
	void Reset();

	bool SelfTest();

	void Write(uint8_t cmd);

	void StartHumidity();
	void StartTemperature();

	bool Read();

	void CompensateHumidity();
	void CompensateTemperature();

	void Deinit();
};



#endif /* BMP180_H_ */
