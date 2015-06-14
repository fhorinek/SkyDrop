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
		return false;

    byte2 data;

    data.uint8[1] = this->i2c->Read();
    data.uint8[0] = this->i2c->Read();

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
	this->temperature = -468.5 + 175.72/6553.6 * (float)this->raw_temperature;
}


void SHT21::CompensateHumidity()
{
	this->humidity = -6.0 + 125.0/655.36 * (float)this->raw_humidity;
}

