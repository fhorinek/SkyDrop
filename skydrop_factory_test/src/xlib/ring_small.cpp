#include "ring_small.h"

#include "../drivers/uart.h"

/**
 * Create FIFO object and allocate memory for buffer
 *
 * \param size Size of bufer
 */
RingBufferSmall::RingBufferSmall(uint8_t size, uint8_t * buffer)
{
	this->size = size;
	this->buffer = buffer;

	this->length = 0;
	this->read_index = 0;
	this->write_index = 0;
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
	for (uint16_t i = 0; i < len; i++)
		this->Write(data[i]);
}

/**
 * Return buffer length
 */
uint8_t RingBufferSmall::Length()
{
	return this->length;
}


uint8_t RingBufferSmall::Size()
{
	return this->size;
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
