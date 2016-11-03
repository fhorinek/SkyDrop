#ifndef RING_H_
#define RING_H_

#include "common.h"

/**
 * FIFO circular buffer for uint8_t data
 * \note maximum length of buffer is 255
 */
class RingBuffer
{
private:
public:

	volatile uint8_t * buffer;		//!< pointer to buffer array

	volatile uint16_t size;			//!< size of buffer
	volatile uint16_t read_index;	//!< array index of next read
	volatile uint16_t write_index;	//!< array index of next write
	volatile uint16_t length;		//!< count of used bytes

//public:
	RingBuffer(uint16_t size, uint8_t * buffer);

	void Write(uint8_t byte);
	void Write(uint16_t len, uint8_t * data);

	uint8_t Read();
	uint16_t Length();

	void Rewind(uint16_t len);
	void Forward(uint16_t len);

	void Clear();
};

#endif
