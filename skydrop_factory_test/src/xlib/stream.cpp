#include "stream.h"

#include "../drivers/uart.h"

/**
 * Initialize Stream object
 *
 * \param output StdOut FILE for buffer output
 * \param buffer_length Buffer size
 *
 */
void Stream::Init(FILE * output, RingBuffer * buffer)
{
	this->output = output;

	this->rx_next = next_startbyte;

	this->tx_length = 0;
	this->rx_length = 0;
	this->tx_crc = 0x00;
	this->rx_crc = 0x00;

	this->buffer = buffer;

	this->onPacket = NULL;
}

/**
 * Write one byte to outgoing packet
 *
 * \param data Data byte to be send
 * \note method StartPacket() should be used before using this method
 */
void Stream::Write(uint8_t data)
{
	if (this->tx_length == 0)
		return;

	this->tx_crc = CalcCRC(this->tx_crc, xlib_stream_crc_key, data);
	fputc(data, this->output);
	this->tx_length--;

	if (this->tx_length == 0)
	{
		fputc(this->tx_crc, this->output);
	}
}

void Stream::Write(uint16_t len, uint8_t * data)
{
	for (uint16_t i = 0; i < len; i++)
		this->Write(data[i]);
}


/**
 * Decode incoming byte from incoming packet
 *
 * \param data Byte from incoming packet
 */
void Stream::Decode(uint8_t data)
{
//	DEBUG(">%d %d\n", data, this->rx_next);

	switch(this->rx_next)
	{
		case(next_startbyte):
			if (data == xlib_stream_startbyte)
			{
				this->rx_next++;
				this->buffer->Clear();
			}
		return;

		case(next_length_lo):
			this->rx_length = data;
			this->rx_crc = CalcCRC(0x00, xlib_stream_crc_key, data);
			this->rx_next++;
		return;

		case(next_length_hi):
			this->rx_length |= data << 8;
			if (this->rx_length > 0)
			{
				this->rx_crc = CalcCRC(this->rx_crc, xlib_stream_crc_key, data);
				this->rx_next++;
			}
			else
			{
				this->rx_next = next_startbyte;
			}
		return;

		case(next_head_crc):
			if (this->rx_crc == data)
				this->rx_next++;
			else
				this->rx_next = next_startbyte;
		return;

		case(next_data):
			this->rx_length--;
			this->buffer->Write(data);
//			this->Debug();
			this->rx_crc = CalcCRC(this->rx_crc, xlib_stream_crc_key, data);
			if (this->rx_length == 0)
				this->rx_next++;
		return;

		case(next_crc):
			if (this->rx_crc != data)
			{
				DEBUG("\n ** ");
				while (this->buffer->length)
					DEBUG("%02X ", this->buffer->Read());
				this->buffer->Clear();
				DEBUG("**\nWRONG CRC %02X %02X\n", this->rx_crc, data);
			}
			else
			{
//				DEBUG("*** CRC OK *** %u\n", this->onPacket);
				if (this->onPacket != NULL)
					this->onPacket();
			}
			this->rx_next = next_startbyte;
		return;

	}
}

/**
 * Read one byte from received message
 *
 * \note new incoming packet will clear buffer so all unread data from received packet will be lost
 * \return one byte from received message
 */
uint8_t Stream::Read()
{
	return this->buffer->Read();
}

void Stream::Debug()
{
	DEBUG("BL %d: ", this->buffer->length);
	for (uint16_t i = 0; i < this->buffer->length; i++)
	{
		uint8_t index = (this->buffer->read_index + i) % this->buffer->size;
		DEBUG("%02X ", this->buffer->buffer[index]);
	}
	DEBUG("\n");
}

/**
 * Start outgoing packet with specified length
 *
 * \param length Length of outgoing packet
 */
void Stream::StartPacket(uint16_t length)
{
	while(this->tx_length > 0)
		this->Write(0x00);

	//header
	fputc(xlib_stream_startbyte ,this->output);
	fputc(length & 0x00FF ,this->output);
	fputc((length & 0xFF00) >> 8 ,this->output);

	this->tx_crc = CalcCRC(0x00, xlib_stream_crc_key, (length & 0x00FF) >> 0);
	this->tx_crc = CalcCRC(this->tx_crc, xlib_stream_crc_key, (length & 0xFF00) >> 8);

	//head crc
	fputc(this->tx_crc ,this->output);

	this->tx_length = length;
}


/**
 * Register callback function. This function will be called from method Decode when packet is successfully decoded and crc matched
 *
 * \param cb Pointer to callback function
 */
void Stream::RegisterOnPacket(onPacket_cb_t cb)
{
	this->onPacket = cb;
}

