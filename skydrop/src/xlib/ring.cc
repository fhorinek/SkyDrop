#include "ring.h"

/**
 * Create FIFO object and allocate memory for buffer
 *
 * \param size Size of bufer
 */
RingBufferSmall::RingBufferSmall(uint8_t size)
{
	this->size = size;
	this->buffer = new uint8_t[size];

	this->length = 0;
	this->read_index = 0;
	this->write_index = 0;
}

/**
 * Destroy FIFO object and free buffer memory
 */
RingBufferSmall::~RingBufferSmall()
{
	delete[] this->buffer;
}

/**
 * Read one byte from buffer, return 0 if buffer is empty
 */
uint8_t RingBufferSmall::Read()
{
	uint8_t a;

	if (this->length <= 0) return 0;

	this->length--;
	a = this->buffer[this->read_index];
	this->read_index++;
	if (this->read_index >= this->size)
		this->read_index = 0;

	return a;
}
/**
 * Write one byte to buffer
 *
 * \param data Data to be written
 */
void RingBufferSmall::Write(uint8_t data)
{
	if (this->length < this->size)
		this->length++;

	this->buffer[this->write_index] = data;
	this->write_index++;

	if (this->write_index >= this->size)
		this->write_index = 0;
}

/**
 * Write array of bytes to buffer
 *
 * \param len Length of input array
 * \param data Pointer to input array
 */
void RingBufferSmall::Write(uint8_t len, uint8_t * data)
{
	while (len > 0)
		this->Write(data[--len]);
}

/**
 * Return buffer length
 */
uint8_t RingBufferSmall::Length()
{
	return this->length;
}

/**
 * Clear buffer
 */
void RingBufferSmall::Clear()
{
	this->length = 0;
	this->read_index = 0;
	this->write_index = 0;
}
