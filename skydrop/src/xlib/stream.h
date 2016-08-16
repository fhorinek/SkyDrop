/**
 * This example shows ....
 *
 * | Start byte (0xC0) | length (N) | length (N) | data(0) | data(1) | ... | data(N-1) | crc(data) |
 *
 * every cell represent 8-bit value
 *
 * PC side
 * \include core/stream/client.py
 *
 * Stax side
 * \example core/stream/stax.cc
 *
 */

#ifndef STREAM_H_
#define STREAM_H_

#include "ring.h"
#include "common.h"
#include "stdio.h"

#define xlib_stream_startbyte	0xC0	//!< Defined start byte
#define xlib_stream_crc_key		0xD5	//!< Defined CRC key

/**
 * Stream state definition
 */
typedef enum xlib_stream_next_e
{
	next_startbyte = 0,		//!< Waiting for startbyte
	next_length_lo = 1,		//!< Waiting for length low
	next_length_hi = 2,		//!< Waiting for length high
	next_head_crc = 3,
	next_data = 4,			//!< Waiting for data
	next_crc = 5			//!< Waiting for CRC
} xlib_stream_next;

typedef void (*onPacket_cb_t)(void);

class Stream //! Object for creating packet communication with error detection
{
private:
	RingBuffer * buffer;		//!< buffer for incoming packet

	volatile uint8_t rx_next;		//!< state of incoming packet

	volatile uint16_t rx_length;		//!< length of incoming packet
	volatile uint16_t tx_length;		//!< length of outgoing packet

	volatile uint8_t rx_crc;		//!< crc byte for incoming packet
	volatile uint8_t tx_crc;		//!< crc byte for outgoing packet

	FILE * output;					//!< output file for stream

public:
	void Init(FILE * output, RingBuffer * buffer);

	void StartPacket(uint16_t length);
	void Write(uint8_t data);
	void Write(uint16_t len, uint8_t * data);

	uint8_t Read();
	void Decode(uint8_t data);
	void Debug();

	void RegisterOnPacket(onPacket_cb_t cb);
	onPacket_cb_t onPacket;
};


#endif /* STREAM_H_ */
