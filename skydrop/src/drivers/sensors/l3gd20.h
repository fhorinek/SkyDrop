/*
 * l3gb20.h
 *
 *  Created on: 1.8.2014
 *      Author: horinek
 */

// DS: http://www.st.com/st-web-ui/static/active/en/resource/technical/document/datasheet/DM00036465.pdf

#ifndef L3GD20_H_
#define L3GD20_H_

#define L3G_ADDRESS	0x6B
#define L3G_ID		0b11010100

#define L3GD20_FIFO_TRESHOLD 16

class L3gd20;

#include "devices.h"


enum l3g_scale
{
	l3g_250dps	= 0b00000000,
	l3g_500dps	= 0b00010000,
	l3g_2000dps	= 0b00100000,
};

enum l3g_odr
{
	l3g_95Hz 	= 0b00000000,
	l3g_190Hz 	= 0b01000000,
	l3g_380Hz 	= 0b10000000,
	l3g_760Hz 	= 0b11000000,
};

enum l3g_bw
{
	l3g_12Hz	= 0b00000000, //works for 95, 190
	l3g_25Hz	= 0b00010000, //works for 95, 190, 380
	l3g_50Hz	= 0b00100000, //works for 190, 380, 760
	l3g_70Hz	= 0b00110000, //works for 190
	l3g_100Hz	= 0b00110000, //works for 380, 760
};

struct l3gd20_settings
{
	bool enabled;

	l3g_odr  odr;
	l3g_scale scale;
	l3g_bw bw;
};

class L3gd20
{
//private:
public:
	I2c * i2c;
	struct l3gd20_settings settings;

	uint8_t Read(uint8_t adr);
	uint16_t Read16(uint8_t adr);
	void Write(uint8_t adr, uint8_t data);
	void WriteOr(uint8_t adr, uint8_t data);
	void WriteAnd(uint8_t adr, uint8_t data);

//public:
	void Reset();
	bool SelfTest();

	void EnableGyro(l3g_odr odr, l3g_scale scale, l3g_bw bw);
	void EnableGyroFIFO(uint8_t thold);

	void ReadGyro(int16_t * x, int16_t * y, int16_t * z);
	uint8_t GyroStreamLen();

	void StartReadGyroStream(uint8_t len);
	uint8_t ReadGyroStreamAvg(volatile int16_t * x, volatile int16_t * y, volatile int16_t * z, uint8_t len);
	int8_t ReadTemp();

	void Set(struct l3gd20_settings settings);

	bool INT1();
	bool INT2();

	//common control
	void Init(I2c *i2c, struct l3gd20_settings settings);
	void Deinit();

	void Start();
	void Stop();
};


#endif /* LSM303D_H_ */
