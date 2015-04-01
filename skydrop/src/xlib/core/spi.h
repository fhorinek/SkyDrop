#ifndef SPI_H_
#define SPI_H_

#include "../common.h"
#include "../ring.h"
#include "gpio.h"

//debug
#include "../stdio.h"

#define spi0	&SPID, &PORTD, 0
#define spi1	&SPIC, &PORTC, 1

#define spid	&SPID, &PORTD, 0
#define spic	&SPIC, &PORTC, 1


typedef enum xlib_core_spi_prescaler_e
{
	spi_div_2 	= 0b10000000,
	spi_div_4 	= 0b00000000,
	spi_div_8 	= 0b10000001,
	spi_div_16 	= 0b00000001,
	spi_div_32 	= 0b10000010,
	spi_div_64 	= 0b00000010,
	spi_div_128	= 0b00000011
} xlib_core_spi_prescaler;


class Spi;

typedef void (*spi_event_cb_t)(void);

class Spi
{
public:

	SPI_t * spi;
	PORT_t * port;

	RingBufferSmall * rx_buffer;
	RingBufferSmall * tx_buffer;

	volatile bool isbusy;
	PORT_t * ss_port;
	uint8_t ss_pin;




//public:
	void InitMaster();
	void InitMaster(SPI_t * spi, PORT_t * port, uint8_t n, uint8_t rx_size, uint8_t tx_size);
	void InitMaster(SPI_t * spi, PORT_t * port, uint8_t n, uint8_t size);
	void InitMaster(SPI_t * spi, PORT_t * port, uint8_t n);

	void InitSlave(SPI_t * spi, PORT_t * port, uint8_t n, uint8_t rx_size, uint8_t tx_size);
	void InitSlave(SPI_t * spi, PORT_t * port, uint8_t n, uint8_t size);
	void InitSlave(SPI_t * spi, PORT_t * port, uint8_t n);

	void Stop();

	void SetMode(uint8_t mod);
	void SetDataOrder(uint8_t dataorder);
	void SetSlave(PORT_t * port, uint8_t pin);
	void UnsetSlave();
	void SetDivider(xlib_core_spi_prescaler div);

	void IrqRequest();

	void StartTransmittion(PORT_t * port, uint8_t pin);
	uint8_t SendRaw(uint8_t data);

	void Write(uint8_t data);
	uint8_t Read();
	void Clear();
	void Wait();
};

#endif /* SPI_H_ */
