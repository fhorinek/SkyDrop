#include "accelerometer.h"

extern I2c * i2cs[2];
/**
 * Convert complement of 2 to dec
 */
int8_t to_dec(int8_t c)
{
	if (c<32) return c;
	return c - 64;
}

/**
 * Initialize on board mma7660FC  (Quick Init)
 * Set accelerometer to active mode & initialize default i2c module
 *
 * \param i2c Pointer to i2c object
 */
void Accelerometer::Init()
{
	if (i2cs[0] == NULL)
	{
		i2cs[0] = new I2c();
		i2cs[0]->InitMaster();
	}

	this->Init(i2cs[0]);
}

/**
 * Set accelerometer to active mode
 *
 * \param i2c Pointer to i2c object
 */
void Accelerometer::Init(I2c * i2c)
{
	this->i2c = i2c;
	this->Set(ACC_MODE, 0x01);
}

/**
 * Get X value of accelerometer
 *
 * \return signed 6-bit value of X axis representing +-1.5g
 */
int8_t Accelerometer::GetX()
{
	uint8_t tmp;
	do
		tmp = this->Get(ACC_XOUT);
	while (tmp & 0b01000000); //alert

	return to_dec(tmp);
}

/**
 * Get Y value of accelerometer
 *
 * \return signed 6-bit value of Y axis representing +-1.5g
 */
int8_t Accelerometer::GetY()
{
	uint8_t tmp;
	do
		tmp = this->Get(ACC_YOUT);
	while (tmp & 0b01000000); //alert

	return to_dec(tmp);
}

/**
 * Get Y value of accelerometer
 *
 * \return signed 6-bit value of Y axis representing +-1.5g
 */
int8_t Accelerometer::GetZ()
{
	uint8_t tmp;
	do
		tmp = this->Get(ACC_ZOUT);
	while (tmp & 0b01000000); //alert

	return to_dec(tmp);
}

/**
 * Get register value
 *
 * \param reg Register address
 * \return Registe Value
 */
uint8_t Accelerometer::Get(uint8_t reg)
{
	this->i2c->Wait();

	this->i2c->Write(reg);
	this->i2c->StartTransmittion(ACC_ADDRESS, 1);

	this->i2c->Wait();
	return this->i2c->Read();
}

/**
 * Set register value
 *
 * \param reg Register address
 * \param data Registe Value
 */
void Accelerometer::Set(uint8_t reg, uint8_t data)
{
	this->i2c->Wait();

	this->i2c->Write(reg);
	this->i2c->Write(data);
	this->i2c->StartTransmittion(ACC_ADDRESS, 0);
}
