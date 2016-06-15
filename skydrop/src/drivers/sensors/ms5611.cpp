/*
 * ms5611.cc
 *
 *  Created on: 29.11.2013
 *      Author: horinek
 */

#include "ms5611.h"
#include "devices.h"

void MS5611::Init(I2c * i2c, uint8_t address)
{
	this->i2c = i2c;
	this->address = address;

	this->press_osr = MS5611_OSR_4096;
	this->temp_osr = MS5611_OSR_256;

	this->Reset();
	_delay_ms(10);
	this->ReadPROM();
}

uint16_t MS5611::Read16(uint8_t cmd)
{
	this->i2c->Write(cmd);
	this->i2c->StartTransmittion(this->address, 2);
	this->i2c->Wait();

	uint8_t a = this->i2c->Read();
	uint8_t b = this->i2c->Read();

	return (a << 8) | b;
}

uint32_t MS5611::Read24(uint8_t cmd)
{
	union {uint32_t val; uint8_t raw[4];} res;

	this->i2c->Write(cmd);
	this->i2c->StartTransmittion(this->address, 3);
	this->i2c->Wait();

	res.raw[2] = this->i2c->Read();
	res.raw[1] = this->i2c->Read();
	res.raw[0] = this->i2c->Read();

	return res.val;
}

bool MS5611::SelfTest()
{
	this->i2c->StartTransmittion(this->address, 0);
	this->i2c->Wait();

	if (this->i2c->Error())
	{
		return false;
	}

	return true;
}

void MS5611::ReadPROM()
{
	this->calibration_C1 = this->Read16(MS5611_PROM + 0);
	this->calibration_C2 = this->Read16(MS5611_PROM + 2);
	this->calibration_C3 = this->Read16(MS5611_PROM + 4);
	this->calibration_C4 = this->Read16(MS5611_PROM + 6);
	this->calibration_C5 = this->Read16(MS5611_PROM + 8);
	this->calibration_C6 = this->Read16(MS5611_PROM + 10);

//	DEBUG("ms5611 calibration data\n");
//	DEBUG(" C1 %u\n", this->calibration_C1);
//	DEBUG(" C2 %u\n", this->calibration_C2);
//	DEBUG(" C3 %u\n", this->calibration_C3);
//	DEBUG(" C4 %u\n", this->calibration_C4);
//	DEBUG(" C5 %u\n", this->calibration_C5);
//	DEBUG(" C6 %u\n", this->calibration_C6);
}

void MS5611::Write(uint8_t cmd)
{
	this->i2c->Write(cmd);
	this->i2c->StartTransmittion(this->address, 0);
	this->i2c->Wait();
};

void MS5611::Reset()
{
	this->Write(MS5611_RESET);
}

void MS5611::StartPressure()
{
	this->Write(MS5611_D1 | this->press_osr);

}

void MS5611::StartTemperature()
{
	this->Write(MS5611_D2 | this->temp_osr);

}

void MS5611::ReadPressure()
{
    this->raw_pressure = this->Read24(MS5611_READ);
    assert(this->raw_pressure != 0);
}

void MS5611::ReadTemperature()
{
	this->raw_temperature = this->Read24(MS5611_READ);
    assert(this->raw_temperature != 0);
}

void MS5611::CompensateTemperature()
{
	this->dT = this->raw_temperature - (this->calibration_C5 * (int32_t)256);
	this->temperature = (2000ul + ((int64_t)this->dT * (int64_t)this->calibration_C6) / (int64_t)8388608);
}


void MS5611::CompensatePressure()
{
	int64_t off = (int64_t)this->calibration_C2 * (int64_t)65536 + ((int64_t)this->calibration_C4 * (int64_t)this->dT) / 128;
	int64_t sens = (int64_t)this->calibration_C1 * (int64_t)32768 + ((int64_t)this->calibration_C3 * (int64_t)this->dT) / 256;

	if (this->temperature < 2000)
	{
		//low temperature
		uint64_t t2 = (uint64_t)this->dT * (uint64_t)this->dT / 2147483648ul;
		uint64_t temp = (this->temperature - 2000) * (this->temperature - 2000);
		uint64_t off2 = 5 * temp / 2;
		uint64_t sens2 = off2 / 2;

		if (this->temperature < -1500)
		{
			//very low temperature
			temp = (this->temperature + 1500) * (this->temperature + 1500);
			off2 = off2 + 7 * temp;
			sens2 = sens2 + 11 * temp / 2;
		}

		this->temperature -= t2;
		off -= off2;
		sens -= sens2;
	}

	this->pressure = (float)((int32_t)this->raw_pressure * sens / (int32_t)2097152 - off) / 32768.0;
}

float MS5611::GetAltitude(float currentSeaLevelPressureInPa, float pressure)
{
    // Calculate altitude from sea level.
    float altitude = 44330.0 * (1.0 - pow(pressure / currentSeaLevelPressureInPa, 0.1902949571836346));
    return altitude;
}
