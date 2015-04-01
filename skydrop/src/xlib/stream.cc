#include "stream.h"

/**
 * Initialize Stream object using standard buffer size
 *
 * \param output StdOut FILE for buffer output
 */
void Stream::Init(FILE * output)
{
	this->Init(output, BUFFER_SIZE);
}

/**
 * Initialize Stream object
 *
 * \param output StdOut FILE for buffer output
 * \param buffer_length Buffer size
 *
 */
void Stream::Init(FILE * output, uint8_t buffer_length)
{
	this->output = output;

	this->rx_next = next_startbyte;

	this->tx_length = 0;
	this->rx_length = 0;
	this->tx_crc = 0x00;
	this->rx_crc = 0x00;

	this->buffer = new RingBufferSmall(buffer_length);
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
		fputc(this->tx_crc, this->output);
}

/**
 * Decode incoming byte from incoming packet
 *
 * \param data Byte from incoming packet
 */
void Stream::Decode(uint8_t data)
{
	switch(this->rx_next)
	{
		case(next_startbyte):
			if (data == xlib_stream_startbyte)
				this->rx_next++;
				this->buffer->Clear();
		return;
		case(next_length):
			this->rx_length = data;
			if (data == 0)
				this->rx_next = next_startbyte;
			else
				this->rx_next++;
		return;
		case(next_length_check):
			if (data == this->rx_length)
			{
				this->rx_crc = CalcCRC(0x00, xlib_stream_crc_key, data);
				this->rx_next++;
			}
			else
				this->rx_next = next_startbyte;
		return;
		case(next_data):
			this->rx_length--;
			this->buffer->Write(data);
			this->rx_crc = CalcCRC(this->rx_crc, xlib_stream_crc_key, data);
			if (this->rx_length == 0)
				this->rx_next++;
		return;
		case(next_crc):
			if (this->rx_crc != data)
			{
				this->buffer->Clear();
			}
			else
			{
				if (Stream::onPacket != NULL)
					Stream::onPacket();
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

/**
 * Start outgoing packet with specified length
 *
 * \param length Length of outgoing packet
 */
void Stream::StartPacket(uint8_t length)
{
	while(this->tx_length > 0)
		this->Write(0x00);

	//header
	fputc(xlib_stream_startbyte ,this->output);
	fputc(length ,this->output);
	fputc(length ,this->output);

	this->tx_crc = CalcCRC(0x00, xlib_stream_crc_key, length);
	this->tx_length = length;
}


/**
 * Register callback function. This function will be called from method Decode when packet is successfully decoded and crc matched
 *
 * \param cb Pointer to callback function
 */
void Stream::RegisterOnPacket(onPacket_cb_t cb)
{
	Stream::onPacket = cb;
}

onPacket_cb_t Stream::onPacket = NULL;
