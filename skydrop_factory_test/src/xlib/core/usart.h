#ifndef USART_H_
#define USART_H_

#include "../common.h"
#include "../stdio.h"
#include "gpio.h"

//		name	usart,    port tx	 n
#define usart0	&USARTE0, usart0_tx, 0
#define usart1	&USARTE1, usart1_tx, 1
#define usart2	&USARTF0, usart2_tx, 2

//portc
#define usart3	&USARTC0, usart3_tx, 3
#define usart4	&USARTC1, usart4_tx, 4

//spi
#define usart5	&USARTD1, usart5_tx, 5
#define usart6	&USARTD0, usart6_tx, 6

//manual - as in datasheet
#define usartc0 usart3
#define usartc1 usart4
#define usartd0 usart6
#define usartd1 usart5
#define usarte0 usart0
#define usarte1 usart1
#define usartf0 usart2

typedef enum xlib_core_usart_events_e //! usart events
{
	usart_event_rxcomplete, //!< execute after RX complete routine
	usart_event_txcomplete	//!< execute after TX complete routine
} xlib_core_usart_events;

#define xlib_core_usart_events_count	2

class Usart;

typedef void (*usart_event_cb_t)(Usart *);

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

	usart_event_cb_t events[xlib_core_usart_events_count];

	volatile bool dbg;

public:

	volatile uint8_t rx_ovf;

	Usart(uint16_t rx_size, uint8_t * rx_buffer, uint16_t tx_size, uint8_t * tx_buffer);

	void Init();
	void Init(uint32_t baud);
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

	void RegisterEvent(xlib_core_usart_events, usart_event_cb_t cb);
};

#endif /* USART_H_ */
