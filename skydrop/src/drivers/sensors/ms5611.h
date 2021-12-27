/*
 * ms5611.H
 *
 *  Created on: 29.11.2013
 *      Author: horinek
 */

#ifndef MS5611_H_
#define MS5611_H_

#include "xlib/core/i2c.h"
#include "xlib/core/usart.h"
#include "common.h"

#define MS5611_ADDRESS_CSB_HI	0b1110110
#define MS5611_ADDRESS_CSB_LO	0b1110111



// registers of the device
#define MS5611_D1		0x40
#define MS5611_D2		0x50
#define MS5611_RESET	0x1E
#define MS5611_READ		0x00

// OSR (Over Sampling Ratio) constants
#define MS5611_OSR_256 	0x00
#define MS5611_OSR_512 	0x02
#define MS5611_OSR_1024 0x04
#define MS5611_OSR_2048 0x06
#define MS5611_OSR_4096 0x08

#define MS5611_PROM 0xA2 // by adding ints from 0 to 6 we can read all the prom configuration values.

#define MS5611_RESET	0x1E

class MS5611
{
public:
	Timer timer;

	uint16_t calibration_C1;
	uint16_t calibration_C2;
	uint16_t calibration_C3;
	uint16_t calibration_C4;
	uint16_t calibration_C5;
	uint16_t calibration_C6;

	uint8_t press_osr;
	uint8_t temp_osr;

	int32_t dT;

	I2c * i2c;

	uint8_t address;

	int32_t temperature;
	float pressure;

	uint32_t raw_temperature;
	uint32_t raw_pressure;

	void Init(I2c * i2c, uint8_t address);
	void ReadPROM();
	void Reset();

	bool SelfTest();

	void Write(uint8_t cmd);
	uint16_t Read16(uint8_t cmd);
	uint32_t Read24(uint8_t cmd);

	void StartPressure();
	void StartTemperature();

	void ReadPressure();
	void ReadTemperature();

	void CompensatePressure();
	void CompensateTemperature();

	float GetAltitude(float currentSeaLevelPressureInPa, float pressure);
};



#endif /* MS5611_H_ */
