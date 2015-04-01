#ifndef ACCELEROMETER_H_
#define ACCELEROMETER_H_

#include "i2c.h"

#define ACC_XOUT		0x00
#define ACC_YOUT		0x01
#define ACC_ZOUT		0x02
#define ACC_TILT		0x03
#define ACC_SAMPLE_RATE	0x04
#define ACC_SLEEP		0x05
#define ACC_INTERRUPT	0x06
#define ACC_MODE		0x07
#define ACC_WAKE		0x08
#define ACC_DETECTION	0x09

#define ACC_ADDRESS		0b1001100	//!< default mma7660FC address

class Accelerometer
{
private:
	I2c * i2c;
	void Set(uint8_t reg, uint8_t data);
	uint8_t Get(uint8_t reg);

public:
	void Init();
	void Init(I2c * i2c);

	int8_t GetX();
	int8_t GetY();
	int8_t GetZ();
};

#endif /* ACCELEROMETER_H_ */
