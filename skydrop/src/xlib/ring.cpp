#include "ring.h"

#include "../drivers/uart.h"

/**
 * Create FIFO object and allocate memory for buffer
 *
 * \param size Size of bufer
 */
RingBuffer::RingBuffer(uint16_t size, uint8_t * buffer)
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
uint8_t RingBuffer::Read()
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
void RingBuffer::Write(uint8_t data)
{
	if (this->length < this->size)
		this->length++;

	this->buffer[this->write_index] = data;
	this->write_index++;

	if (this->write_index >= this->size)
		this->write_index = 0;

//	DEBUG(">> %02X %d %d %d\n", data, this->length, this->write_index, this->size);
}

/**
 * Write array of bytes to buffer
 *
 * \param len Length of input array
 * \param data Pointer to input array
 */
void RingBuffer::Write(uint16_t len, uint8_t * data)
{
	for (uint16_t i = 0; i < len; i++)
		this->Write(data[i]);
}

/**
 * Return buffer length
 */
uint16_t RingBuffer::Length()
{
	return this->length;
}

/**
 * Clear buffer
 */
void RingBuffer::Clear()
{
	this->length = 0;
	this->read_index = 0;
	this->write_index = 0;
}

void RingBuffer::Rewind(uint16_t len)
{
	if (this->length + len > this->size)
		return;

	if (this->read_index < len)
		this->read_index = this->size - (len - this->read_index);
	else
		this->read_index -= len;

	this->length += len;
}

void RingBuffer::Forward(uint16_t len)
{
	if (this->length < len)
		return;

	this->read_index = (this->read_index + len) % this->size;
	this->length -= len;
}
