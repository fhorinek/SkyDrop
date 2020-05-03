#ifndef USART_H_
#define USART_H_

#include "../common.h"
#include "../stdio.h"
#include "gpio.h"


//manual - as in datasheet
//debug		- TX
#define usartc0		&USARTC0, usart3_tx, 0
//bluetooth - RX/TX
#define usartd0 	&USARTD0, usart6_tx, 1
//gps - RX/TX
#define usarte0 	&USARTE0, usart0_tx, 2
//SPI - RX/TX
#define usartf0 	&USARTF0, usart2_tx, 3


class Usart //! Object for communication using usart modules
{
private:
public:
	USART_t * usart;

	volatile uint8_t * rx_buffer;
	volatile uint8_t * tx_buffer;

	volatile uint16_t rx_len;
	volatile uint16_t rx_index;
	volatile uint16_t rx_buffer_size;

	volatile uint16_t tx_len;
	volatile uint16_t tx_index;
	volatile uint16_t tx_buffer_size;

	volatile uint16_t read_index;
	volatile uint16_t write_index;

	volatile DMA_CH_t * dma_rx_ch;

	bool dma;

	uint8_t irq_priority;

	PORT_t * sck_port;
	uint8_t sck_pin;
	uint8_t spi_mode;

	volatile bool dbg;

public:

	volatile uint8_t rx_ovf;

	Usart(uint16_t rx_size, uint8_t * rx_buffer, uint16_t tx_size, uint8_t * tx_buffer);

	void Init(USART_t * usart, PORT_t * port, uint8_t tx, uint8_t n, uint32_t baud);
	void Stop();

	void BecomeSPI(uint8_t mode, uint8_t dataorder, uint32_t speed);
	uint8_t SendRaw(uint8_t data);
	void SendRaw1(uint8_t data);

	void SetupRxDMA(DMA_CH_t * ch, uint8_t trig);
	void DumpDMA();

	uint16_t GetRxLen();

	void RxComplete();
	void TxComplete();

	bool isTxBufferEmpty();
	bool isRxBufferEmpty();

	uint8_t Read();
	uint8_t Peek();
	void Write(uint8_t c);

	void FlushTxBuffer();
	void SetInterruptPriority(uint8_t p);
	void ClearRxBuffer();
};

#endif /* USART_H_ */
