#ifndef RING_H_
#define RING_H_

#include "common.h"

/**
 * FIFO circular buffer for uint8_t data
 * \note maximum length of buffer is 255
 */
class RingBufferSmall
{
private:
public:

	volatile uint8_t * buffer;		//!< pointer to buffer array

	volatile uint8_t size;			//!< size of buffer
	volatile uint8_t read_index;	//!< array index of next read
	volatile uint8_t write_index;	//!< array index of next write
	volatile uint8_t length;		//!< count of used bytes

//public:
	RingBufferSmall(uint8_t size);
	~RingBufferSmall();

	void Write(uint8_t byte);
	void Write(uint8_t len, uint8_t * data);

	uint8_t Read();
	uint8_t Length();

	void Clear();
};

#endif /* RING_H_ */
