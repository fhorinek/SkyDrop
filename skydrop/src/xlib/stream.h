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
	next_length = 1,		//!< Waiting for length
	next_length_check = 2,	//!< Waiting for length confirmation
	next_data = 3,			//!< Waiting for data
	next_crc = 4			//!< Waiting for CRC
} xlib_stream_next;

typedef void (*onPacket_cb_t)(void);

class Stream //! Object for creating packet communication with error detection
{
private:
	RingBufferSmall * buffer;		//!< buffer for incoming packet

	volatile uint8_t rx_next;		//!< state of incoming packet

	volatile uint8_t rx_length;		//!< length of incoming packet
	volatile uint8_t tx_length;		//!< length of outgoing packet

	volatile uint8_t rx_crc;		//!< crc byte for incoming packet
	volatile uint8_t tx_crc;		//!< crc byte for outgoing packet

	FILE * output;					//!< output file for stream

public:
	void Init(FILE * output);
	void Init(FILE * output, uint8_t buffer_length);

	void StartPacket(uint8_t length);
	void Write(uint8_t data);
	uint8_t Read();
	void Decode(uint8_t data);

	static void RegisterOnPacket(onPacket_cb_t cb);
	static onPacket_cb_t onPacket;
};


#endif /* STREAM_H_ */
