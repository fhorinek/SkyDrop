#ifndef SPI_H_
#define SPI_H_

#include "../common.h"
#include "../ring_small.h"
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


class Spi
{
public:

	SPI_t * spi;
	PORT_t * port;

	PORT_t * ss_port;
	uint8_t ss_pin;

//public:
	void InitMaster(SPI_t * spi, PORT_t * port, uint8_t n);

	void Stop();

	void SetMode(uint8_t mod);
	void SetDataOrder(uint8_t dataorder);
	void SetSlave(PORT_t * port, uint8_t pin);
	void UnsetSlave();
	void SetDivider(xlib_core_spi_prescaler div);

	uint8_t SendRaw(uint8_t data);
};

#endif /* SPI_H_ */
