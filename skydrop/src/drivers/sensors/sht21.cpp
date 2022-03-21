/*
 * sht21.cc
 *
 *  Created on: 29.11.2013
 *      Author: horinek
 */

#include "sht21.h"
#include "../uart.h"


void SHT21::Init(I2c * i2c, struct sht21_settings settings)
{
	this->i2c = i2c;
	this->settings = settings;

	if (!(this->settings.temp_enabled && this->settings.rh_enabled))
		return;

	this->present = this->SelfTest();
	if (!this->present)
		return;

	this->Reset();
}

void SHT21::Deinit()
{
}

bool SHT21::SelfTest()
{
	this->i2c->StartTransmittion(SHT21_ADDRESS, 0);
	this->i2c->Wait();

	if (this->i2c->Error())
	{
		return false;
	}

	return true;
}

void SHT21::Reset()
{
	this->Write(SHT21_RESET);
}

void SHT21::Write(uint8_t cmd)
{
	this->i2c->Write(cmd);
	this->i2c->StartTransmittion(SHT21_ADDRESS, 0);
	this->i2c->Wait();
}

void SHT21::StartHumidity()
{
	this->Write(SHT21_MEASURE_RH);
}

void SHT21::StartTemperature()
{
	this->Write(SHT21_MEASURE_T);
}

bool SHT21::Read()
{
	this->i2c->StartTransmittion(SHT21_ADDRESS, 2);
	this->i2c->Wait();
	if (this->i2c->Error())
	{
		assert(0);
		return false;
	}

    byte2 data;

    data.uint8[1] = this->i2c->Read();
    data.uint8[0] = this->i2c->Read();

//    DEBUG("data: %02X %02X\n", data.uint8[1], data.uint8[0]);

    bool humidity = (data.uint8[0] & 0b00000010);

    data.uint8[0] &= 0b11111100;

    if (humidity)
    	this->raw_humidity = data.uint16;
    else
    	this->raw_temperature = data.uint16;

    return true;
}

void SHT21::CompensateTemperature()
{
//	DEBUG("RT -- %u\n", this->raw_temperature);
	float tmp = -46.85 -5 + 175.72 * ((float)this->raw_temperature / 65536.0);
//	DEBUG("  %0.2f\n", tmp);
	this->temperature = tmp * 10;
}


void SHT21::CompensateHumidity()
{
	this->humidity = -6.0 + 125.0/655.36 * (float)this->raw_humidity;
}

