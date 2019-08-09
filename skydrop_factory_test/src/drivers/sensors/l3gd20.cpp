/*
 * L3gd20.cc
 *
 *  Created on: 1.8.2014
 *      Author: horinek
 */

#include "l3gd20.h"


void L3gd20::Init(I2c *i2c, struct l3gd20_settings settings)
{
	if (!settings.enabled)
		return;

	this->i2c = i2c;

//	GpioSetDirection(GY_INT1, INPUT);
//	GpioSetDirection(GY_INT2, INPUT);

	this->Set(settings);
}

void L3gd20::Deinit()
{
	if (!settings.enabled)
		return;

	this->Stop();

//	GpioSetInterrupt(GY_INT2, gpio_clear);

	this->settings.enabled = false;
}

void L3gd20::Write(uint8_t adr, uint8_t data)
{
	this->i2c->Wait();

	this->i2c->Write(adr);
	this->i2c->Write(data);
	this->i2c->StartTransmittion(L3G_ADDRESS, 0);
}

uint8_t L3gd20::Read(uint8_t adr)
{
	this->i2c->Wait();

	this->i2c->Write(adr);
	this->i2c->StartTransmittion(L3G_ADDRESS, 1);
	this->i2c->Wait();

	return this->i2c->Read();
}

uint16_t L3gd20::Read16(uint8_t adr)
{
	this->i2c->Wait();

	this->i2c->Write(adr | 0b10000000); // + auto increment
	this->i2c->StartTransmittion(L3G_ADDRESS, 2);
	this->i2c->Wait();

	uint16_t tmp = this->i2c->Read() << 8;
	tmp |= this->i2c->Read();

	return tmp;
}

void L3gd20::WriteOr(uint8_t adr, uint8_t data)
{
	uint8_t tmp = this->Read(adr);

	this->Write(adr, tmp | data);
}

void L3gd20::WriteAnd(uint8_t adr, uint8_t data)
{
	uint8_t tmp = this->Read(adr);

	this->Write(adr, tmp & data);
}

void L3gd20::Reset()
{
	if (!this->settings.enabled)
		return;

	this->Write(0x24, 0b10000000); //CTRL_REG5 <- BOOT

	_delay_ms(1);

	this->Write(0x24, 0b00000000); //CTRL_REG5 -> BOOT
}

bool L3gd20::SelfTest()
{
	if (!this->settings.enabled)
		return false;

	uint8_t id = this->Read(0x0F);

	return (id == L3G_ID);
}

void L3gd20::EnableGyro(l3g_odr odr, l3g_scale scale, l3g_bw bw)
{
	if (!this->settings.enabled)
		return;

	this->Write(0x20, 0b00001111 | odr | bw); //CTRL_REG1 <- DR | BW
	this->WriteOr(0x23, scale); //CTRL_REG4 -< scale
}

void L3gd20::EnableGyroFIFO(uint8_t thold)
{
	if (!this->settings.enabled)
		return;

	//enable FIFO
	this->WriteOr(0x24, 0b01000000); //CTRL_REG5 <- FIFO_EN
	//FIFO watermark on INT2
//	this->WriteOr(0x22, 0b00000100); //CTRL_REG3 <-  I2_WTM
	//FIFO mode to Stream mode
	this->Write(0x2E, 0b01000000 | (thold & 0b00011111)); //FIFO_CTRL_REG

//	GpioSetInterrupt(GY_INT2, gpio_interrupt1, gpio_rising);
}

void L3gd20::Start()
{
	if (!this->settings.enabled)
		return;

	uint8_t odrValue = this->settings.odr;
	uint8_t scaleValue = this->settings.scale;
	uint8_t bwValue = this->settings.bw;

	this->EnableGyro((l3g_odr)odrValue, (l3g_scale)scaleValue, (l3g_bw)bwValue);
	this->EnableGyroFIFO(L3GD20_GYRO_FIFO_TRESHOLD);
}

void L3gd20::Stop()
{
	if (!this->settings.enabled)
		return;

	this->Write(0x20, 0b00000000); //CTRL1
}

uint8_t L3gd20::GyroStreamLen()
{
	return this->Read(0x2F) & 0b00011111;
}

void L3gd20::StartReadGyroStream(uint8_t len)
{
	this->i2c->Wait();
	this->i2c->Write(0x28 | 0b10000000);
	this->i2c->StartTransmittion(L3G_ADDRESS, 6 * len);
}

uint8_t L3gd20::ReadGyroStreamAvg(volatile int16_t * x, volatile int16_t * y, volatile int16_t * z, uint8_t len)
{
	int32_t acc_x = 0;
	int32_t acc_y = 0;
	int32_t acc_z = 0;

	this->i2c->Wait();

	for (uint8_t i = 0; i < len; i++)
	{
		byte2 tmp;

		tmp.uint8[0] = this->i2c->Read();
		tmp.uint8[1] = this->i2c->Read();
		acc_x += tmp.int16;
//		DEBUG("%d;", tmp.int16);

		tmp.uint8[0] = this->i2c->Read();
		tmp.uint8[1] = this->i2c->Read();
		acc_y += tmp.int16;
//		DEBUG("%d;", tmp.int16);

		tmp.uint8[0] = this->i2c->Read();
		tmp.uint8[1] = this->i2c->Read();
		acc_z += tmp.int16;
//		DEBUG("%d;\n", tmp.int16);
	}

	*x = (int32_t)acc_x / (int32_t)len;
	*y = (int32_t)acc_y / (int32_t)len;
	*z = (int32_t)acc_z / (int32_t)len;

	return len;
}

int8_t L3gd20::ReadTemp()
{
	if (!this->settings.enabled)
		return 0xFF;

	return to_dec_1(this->Read(0x26)); // TEMP_OUT
}

void L3gd20::Set(struct l3gd20_settings settings)
{
	this->settings = settings;

	if (!this->settings.enabled)
			return;

	this->Reset();
}
