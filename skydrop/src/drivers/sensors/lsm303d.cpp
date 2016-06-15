/*
 * lsm303d.cc
 *
 *  Created on: 1.8.2014
 *      Author: horinek
 */

#include "lsm303d.h"

void Lsm303d::Init(I2c *i2c, struct lsm303d_settings settings)
{
	if (!settings.enabled)
			return;

	this->i2c = i2c;

	//need time to boot up
	_delay_ms(10);

	this->Set(settings);

	_delay_ms(10);
}

void Lsm303d::Deinit()
{
	if (!settings.enabled)
			return;

	this->Stop();

	DisableAccFIFO();

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

//	DUMP_REG(tmp | data);
//	DUMP_REG(this->Read(adr));
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

	this->Write(0x1F, 0b10000000); //CTRL0 <- BOOT

	_delay_ms(1);
	this->Write(0x1F, 0b00000000); //CTRL0 -> BOOT
	_delay_ms(1);
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

//	DEBUG("0x21: ");
//	DUMP_REG(this->Read(0x21));

}

void Lsm303d::DisableAcc()
{
	if (!this->settings.enabled)
		return;

	this->WriteAnd(0x20, 0b00001111); // CTRL1 < BDU, AZEN, AYEN, AXEN
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

uint8_t Lsm303d::AccStreamLen()
{
	return this->Read(0x2F) & 0b00011111;
}

void Lsm303d::StartReadAccStream(uint8_t len)
{
	this->i2c->Wait();
	this->i2c->Write(0x28 | 0b10000000);
	this->i2c->StartTransmittion(LSM_ADDRESS, 6 * len);
}

uint8_t Lsm303d::ReadAccStreamAvg(volatile int16_t * x, volatile int16_t * y, volatile int16_t * z, uint8_t len)
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

void Lsm303d::StartReadMag()
{
	this->i2c->Wait();
	this->i2c->Write(0x08 | 0b10000000);
	this->i2c->StartTransmittion(LSM_ADDRESS, 6);

}

void Lsm303d::ReadMag(volatile int16_t * x, volatile int16_t * y, volatile int16_t * z)
{
	byte2 tmp;

	this->i2c->Wait();

	tmp.uint8[0] = this->i2c->Read();
	tmp.uint8[1] = this->i2c->Read();
	*x = tmp.int16;

	tmp.uint8[0] = this->i2c->Read();
	tmp.uint8[1] = this->i2c->Read();
	*y = tmp.int16;

	tmp.uint8[0] = this->i2c->Read();
	tmp.uint8[1] = this->i2c->Read();
	*z = tmp.int16;

//	//transfer test
//	*x = 10;
//	*y = 20;
//	*z = 30;
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
