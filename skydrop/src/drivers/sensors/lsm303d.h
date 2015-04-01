/*
 * lsm303d.h
 *
 *  Created on: 1.8.2014
 *      Author: horinek
 */

// DS: http://www.st.com/web/en/resource/technical/document/datasheet/DM00057547.pdf

#ifndef LSM303D_H_
#define LSM303D_H_

#define LSM_ADDRESS	0x1D
#define LSM_ID		0b01001001

#define LSM303D_ACC_FIFO_TRESHOLD 16
class Lsm303d;

#include "devices.h"



enum lsm303d_acc_odr
{
	lsm_acc_pd		= 0b00000000, //power down
	lsm_acc_3Hz		= 0b00010000, //3.125 Hz
	lsm_acc_6Hz		= 0b00100000, // 6.25 Hz
	lsm_acc_12Hz	= 0b00110000, // 12.5 Hz
	lsm_acc_25Hz	= 0b01000000,
	lsm_acc_50Hz	= 0b01010000,
	lsm_acc_100Hz	= 0b01100000,
	lsm_acc_200Hz	= 0b01110000,
	lsm_acc_400Hz	= 0b10000000,
	lsm_acc_800Hz	= 0b10010000,
	lsm_acc_1600Hz	= 0b10100000,
};

enum lsm303f_acc_scale
{
	lsm_acc_2g		= 0b00000000,
	lsm_acc_4g		= 0b00001000,
	lsm_acc_6g		= 0b00010000,
	lsm_acc_8g		= 0b00011000,
	lsm_acc_16g		= 0b00100000,
};

enum lsm303d_mag_odr
{
	lsm_mag_pd		= 0b11111111,
	lsm_mag_3Hz		= 0b00000000, //3.125 Hz
	lsm_mag_6Hz		= 0b00000100, // 6.25 Hz
	lsm_mag_12Hz	= 0b00001000, // 12.5 Hz
	lsm_mag_25Hz	= 0b00001100,
	lsm_mag_50Hz	= 0b00010000,
	lsm_mag_100Hz	= 0b00010100, //only when acc ODR > 50 || in PD
};

enum lsm303d_mag_scale
{
	lsm_mag_2g		= 0b00000000,
	lsm_mag_4g		= 0b00100100,
	lsm_mag_8g		= 0b01000000,
	lsm_mag_12g		= 0b01100100,
};


struct lsm303d_settings
{
	bool enabled;

	lsm303d_acc_odr accOdr;
	lsm303f_acc_scale accScale;

	lsm303d_mag_odr magOdr;
	lsm303d_mag_scale magScale;
	bool magHiRes;

	bool tempEnable;
};


class Lsm303d
{
//private:
public:
	I2c * i2c;
	struct lsm303d_settings settings;

	uint8_t Read(uint8_t adr);
	uint16_t Read16(uint8_t adr);
	void Write(uint8_t adr, uint8_t data);
	void WriteOr(uint8_t adr, uint8_t data);
	void WriteAnd(uint8_t adr, uint8_t data);

//public:
	void Reset();
	bool SelfTest();

	void EnableAcc(lsm303d_acc_odr odr, lsm303f_acc_scale scale);
	void DisableAcc();
	void EnableMag(lsm303d_mag_odr odr, lsm303d_mag_scale scale, bool hi_resolution);
	void DisableMag();
	void EnableTemp();
	void DisableTemp();

	void EnableAccFIFO(uint8_t thold);
	void DisableAccFIFO();
	void EnableMagIrq();

	void ReadAcc(int16_t * x, int16_t * y, int16_t * z);
	uint8_t ReadAccStream(int16_t * buff, uint8_t len);
	uint8_t ReadAccStreamAvg(volatile int16_t * x, volatile int16_t * y, volatile int16_t * z, uint8_t len);
	uint8_t AccStreamLen();

	void ReadMag(volatile int16_t * x, volatile int16_t * y, volatile int16_t * z);
	int16_t ReadTemp();

	void Set(struct lsm303d_settings settings);

	bool INT1();
	bool INT2();

	//common control
	void Init(I2c *i2c, struct lsm303d_settings settings);
	void Deinit();

	void Start();
	void Stop();

	void Store();
};


#endif /* LSM303D_H_ */
