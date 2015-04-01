/*
 * lsm303d.cc
 *
 *  Created on: 1.8.2014
 *      Author: horinek
 */

#include "lsm303d.h"

bool Lsm303d::INT1()
{
	return GpioRead(AM_INT1) == HIGH;
}

bool Lsm303d::INT2()
{
	return GpioRead(AM_INT2) == HIGH;
}

void Lsm303d::Init(I2c *i2c, struct lsm303d_settings settings)
{
	if (!settings.enabled)
			return;

	this->i2c = i2c;

	GpioSetDirection(AM_INT1, INPUT);
	GpioSetDirection(AM_INT2, INPUT);

	this->Set(settings);
	//need time to boot up
	_delay_ms(10);
}

void Lsm303d::Deinit()
{
	if (!settings.enabled)
			return;

	this->Stop();

	DisableAccFIFO();
	GpioSetInterrupt(AM_INT2, gpio_clear);

	this->settings.enabled = false;
}

void Lsm303d::Write(uint8_t adr, uint8_t data)
{
	this->i2c->Wait();

	this->i2c->Write(adr);
	this->i2c->Write(data);
	this->i2c->StartTransmittion(LSM_ADDRESS, 0);
}



uint8_t Lsm303d::Read(uint8_t adr)
{
	this->i2c->Wait();

	this->i2c->Write(adr);
	this->i2c->StartTransmittion(LSM_ADDRESS, 1);
	this->i2c->Wait();

	return this->i2c->Read();
}

uint16_t Lsm303d::Read16(uint8_t adr)
{
	this->i2c->Wait();

	this->i2c->Write(adr | 0b10000000); // + auto increment
	this->i2c->StartTransmittion(LSM_ADDRESS, 2);
	this->i2c->Wait();

	uint16_t tmp = this->i2c->Read() << 8;
	tmp |= this->i2c->Read();

	return tmp;
}

void Lsm303d::WriteOr(uint8_t adr, uint8_t data)
{
	uint8_t tmp = this->Read(adr);

	this->Write(adr, tmp | data);

	DUMP_REG(tmp | data);
	DUMP_REG(this->Read(adr));
}

void Lsm303d::WriteAnd(uint8_t adr, uint8_t data)
{
	uint8_t tmp = this->Read(adr);

	this->Write(adr, tmp & data);
}

void Lsm303d::Reset()
{
	if (!this->settings.enabled)
		return;

	this->Write(0x1F, 0b10000000); //CTRL1 <- BOOT

	_delay_ms(1);
	this->Write(0x1F, 0b00000000); //CTRL1 -> BOOT
}

bool Lsm303d::SelfTest()
{
	if (!this->settings.enabled)
		return false;

	uint8_t id = this->Read(0x0F);

	return (id == LSM_ID);
}

void Lsm303d::EnableAcc(lsm303d_acc_odr odr, lsm303f_acc_scale scale)
{
	if (!this->settings.enabled)
		return;

	//set sampling freq
	this->WriteOr(0x20, odr); //CTRL1
	//set scale
	this->WriteOr(0x21, scale); //CTRL2
}

void Lsm303d::DisableAcc()
{
	if (!this->settings.enabled)
		return;

	this->WriteAnd(0x20, 0b00001111); // CTRL1
}

void Lsm303d::EnableMag(lsm303d_mag_odr odr, lsm303d_mag_scale scale, bool hi_resolution)
{
	if (!this->settings.enabled)
		return;

	//set sampling freq & resolution
	this->Write(0x24, odr | (hi_resolution ? 0b01100000 : 0b00000000)); //CTRL5
	//set scale
	this->Write(0x25, scale); //CTRL6
	//enable mag
	this->WriteAnd(0x26, 0b00000000); //CTRL7
}

void Lsm303d::DisableMag()
{
	if (!this->settings.enabled)
		return;

	this->WriteOr(0x26, 0b00000010); //CTRL7
}

void Lsm303d::EnableTemp()
{
	if (!this->settings.enabled)
		return;

	this->WriteOr(0x24, 0b10000000); //CTRL5
}

void Lsm303d::DisableTemp()
{
	if (!this->settings.enabled)
		return;

	this->WriteAnd(0x24, 0b01111111);
}

void Lsm303d::EnableAccFIFO(uint8_t thold)
{
	if (!this->settings.enabled)
		return;

	//enable fifo
	this->WriteOr(0x1F, 0b01100000); //CTRL0 <- FIFO_EN | FTH_EN
	//FIFO thold on INT2
//	this->WriteOr(0x23, 0b00000001); //CTRL4 <- INT2_FTH
	//FIFO mode to Stream mode
	this->Write(0x2E, 0b01000000 | (thold & 0b00011111));
}

void Lsm303d::DisableAccFIFO()
{
	if (!this->settings.enabled)
		return;

	this->WriteAnd(0x1F, 0b10011111); //CTRL0 <- FIFO_EN | FTH_EN
}

void Lsm303d::EnableMagIrq()
{
	if (!this->settings.enabled)
		return;

	//enable magnetometer drdy on INT1
//	this->WriteOr(0x22, 0b00000010); //CTRL3 <- INT1_DRDY_M
	this->Write(0x12, 0b00000100); //INT_CTRL_M <- MIEL // | MIEN
}

void Lsm303d::Start()
{
	if (!this->settings.enabled)
		return;

	if (this->settings.accOdr != lsm_acc_pd)
	{
		this->EnableAcc(this->settings.accOdr, this->settings.accScale);
		this->EnableAccFIFO(LSM303D_ACC_FIFO_TRESHOLD);
	}

	if (this->settings.magOdr != lsm_mag_pd)
	{
		this->EnableMag(this->settings.magOdr, this->settings.magScale, this->settings.magHiRes);
		this->EnableMagIrq();
	}

	if (this->settings.tempEnable)
		this->EnableTemp();
}

void Lsm303d::Stop()
{
	if (!this->settings.enabled)
		return;

	this->DisableAcc();
	this->DisableMag();

	if (this->settings.tempEnable)
		this->DisableTemp();
}

void Lsm303d::ReadAcc(int16_t * x, int16_t * y, int16_t * z)
{
	this->i2c->Wait();
	this->i2c->Write(0x28 | 0b10000000);
	this->i2c->StartTransmittion(LSM_ADDRESS, 6);

	byte2 tmp;

	while (this->i2c->rx_buffer->length == 0);
	tmp.uint8[0] = this->i2c->Read();

	while (this->i2c->rx_buffer->length == 0);
	tmp.uint8[1] = this->i2c->Read();
	*x = tmp.int16;

	while (this->i2c->rx_buffer->length == 0);
	tmp.uint8[0] = this->i2c->Read();

	while (this->i2c->rx_buffer->length == 0);
	tmp.uint8[1] = this->i2c->Read();
	*y = tmp.int16;

	while (this->i2c->rx_buffer->length == 0);
	tmp.uint8[0] = this->i2c->Read();

	while (this->i2c->rx_buffer->length == 0);
	tmp.uint8[1] = this->i2c->Read();
	*z = tmp.int16;
}

uint8_t Lsm303d::AccStreamLen()
{
	return this->Read(0x2F) & 0b00011111;
}

uint8_t Lsm303d::ReadAccStream(int16_t * buff, uint8_t len)
{
	this->i2c->Wait();
	this->i2c->Write(0x28 | 0b10000000);
	this->i2c->StartTransmittion(LSM_ADDRESS, 6 * len);

	for (uint8_t i = 0; i < len; i++)
	{
		int16_t x, y, z;

		byte2 tmp;

		while (this->i2c->rx_buffer->length == 0);
		tmp.uint8[0] = this->i2c->Read();

		while (this->i2c->rx_buffer->length == 0);
		tmp.uint8[1] = this->i2c->Read();
		x = tmp.int16;

		while (this->i2c->rx_buffer->length == 0);
		tmp.uint8[0] = this->i2c->Read();

		while (this->i2c->rx_buffer->length == 0);
		tmp.uint8[1] = this->i2c->Read();
		y = tmp.int16;

		while (this->i2c->rx_buffer->length == 0);
		tmp.uint8[0] = this->i2c->Read();

		while (this->i2c->rx_buffer->length == 0);
		tmp.uint8[1] = this->i2c->Read();
		z = tmp.int16;

		buff[i * 3 + 0] = x;
		buff[i * 3 + 1] = y;
		buff[i * 3 + 2] = z;
	}

	return len;
}

uint8_t Lsm303d::ReadAccStreamAvg(volatile int16_t * x, volatile int16_t * y, volatile int16_t * z, uint8_t len)
{
	int32_t acc_x = 0;
	int32_t acc_y = 0;
	int32_t acc_z = 0;

	this->i2c->Wait();
	this->i2c->Write(0x28 | 0b10000000);
	this->i2c->StartTransmittion(LSM_ADDRESS, 6 * len);


	for (uint8_t i = 0; i < len; i++)
	{
		byte2 tmp;

		while (this->i2c->rx_buffer->length == 0);
		tmp.uint8[0] = this->i2c->Read();

		while (this->i2c->rx_buffer->length == 0);
		tmp.uint8[1] = this->i2c->Read();
		acc_x += tmp.int16;

		while (this->i2c->rx_buffer->length == 0);
		tmp.uint8[0] = this->i2c->Read();

		while (this->i2c->rx_buffer->length == 0);
		tmp.uint8[1] = this->i2c->Read();
		acc_y += tmp.int16;

		while (this->i2c->rx_buffer->length == 0);
		tmp.uint8[0] = this->i2c->Read();

		while (this->i2c->rx_buffer->length == 0);
		tmp.uint8[1] = this->i2c->Read();
		acc_z += tmp.int16;
	}

	*x = acc_x / len;
	*y = acc_x / len;
	*z = acc_x / len;

	return len;
}

void Lsm303d::ReadMag(volatile int16_t * x, volatile int16_t * y, volatile int16_t * z)
{
	this->i2c->Wait();
	this->i2c->Write(0x08 | 0b10000000);
	this->i2c->StartTransmittion(LSM_ADDRESS, 6);

	byte2 tmp;

	while (this->i2c->rx_buffer->length == 0);
	tmp.uint8[0] = this->i2c->Read();

	while (this->i2c->rx_buffer->length == 0);
	tmp.uint8[1] = this->i2c->Read();
	*x = tmp.int16;

	while (this->i2c->rx_buffer->length == 0);
	tmp.uint8[0] = this->i2c->Read();

	while (this->i2c->rx_buffer->length == 0);
	tmp.uint8[1] = this->i2c->Read();
	*y = tmp.int16;

	while (this->i2c->rx_buffer->length == 0);
	tmp.uint8[0] = this->i2c->Read();

	while (this->i2c->rx_buffer->length == 0);
	tmp.uint8[1] = this->i2c->Read();

//	DEBUG("%02X %02X\n", tmp.bytes[0], tmp.bytes[1]);
	*z = tmp.int16;

//	//transfer test
//	*x = -512;
//	*y = 512;
//	*z = 0;
}

int16_t Lsm303d::ReadTemp()
{
	return to_dec_2(this->Read16(0x05)); // TEMP_OUT
}


void Lsm303d::Set(struct lsm303d_settings settings)
{
	this->settings = settings;

	if (!this->settings.enabled)
		return;

	this->Reset();
}
