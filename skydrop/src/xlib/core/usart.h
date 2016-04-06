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

#define XLIB_USE_CORE_USART_LARGEBUFFER

#ifndef XLIB_USE_CORE_USART_LARGEBUFFER
	#define XLIB_USE_CORE_USART_INIT_VAR uint8_t
#else
	#define XLIB_USE_CORE_USART_INIT_VAR uint16_t
#endif

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

	volatile DMA_CH_t * dma_rx_ch;

	PORT_t * cts_port;
	uint8_t cts_pin;
	uint8_t cts_active;
	XLIB_USE_CORE_USART_INIT_VAR cts_threshold;

	PORT_t * rts_port;
	uint8_t rts_pin;
	uint8_t rts_active;

	uint8_t irq_priority;

	PORT_t * sck_port;
	uint8_t sck_pin;
	uint8_t spi_mode;

	usart_event_cb_t events[xlib_core_usart_events_count];

	volatile bool dbg;

#ifndef XLIB_USE_CORE_USART_LARGEBUFFER
	volatile uint8_t rx_index, read_index, rx_len, rx_buffer_size;
	volatile uint8_t tx_index, write_index, tx_len, tx_buffer_size;
#else
	volatile uint16_t rx_index, read_index, rx_len, rx_buffer_size;
	volatile uint16_t tx_index, write_index, tx_len, tx_buffer_size;
#endif

public:

	volatile uint8_t rx_ovf;

	void Init();
	void Init(uint32_t baud);
	void Init(USART_t * usart, PORT_t * port, uint8_t tx, uint8_t n, uint32_t baud);
	void InitBuffers(XLIB_USE_CORE_USART_INIT_VAR rx_size, XLIB_USE_CORE_USART_INIT_VAR tx_size);
	void Stop();

	void BecomeSPI(uint8_t mode, uint8_t dataorder, uint32_t speed);
	uint8_t SendRaw(uint8_t data);
	void SendRaw1(uint8_t data);

	void SetupRxDMA(DMA_CH_t * ch, uint8_t trig);

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

//	void SetCtsPin(PORT_t * port, uint8_t pin, uint8_t active, XLIB_USE_CORE_USART_INIT_VAR threshold);
//	void SetRtsPin(PORT_t * port, uint8_t pin, uint8_t active);

	void RegisterEvent(xlib_core_usart_events, usart_event_cb_t cb);

	Usart();
	~Usart();

};

#endif /* USART_H_ */
