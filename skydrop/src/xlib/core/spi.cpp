#include "spi.h"

void Spi::InitMaster(SPI_t * spi, PORT_t * port, uint8_t n)
{
	this->spi = spi;
	this->port = port;

	GpioSetDirection(port, 4, OUTPUT); //ss
	GpioSetDirection(port, 5, OUTPUT); //mosi
	GpioSetDirection(port, 6, INPUT);  //miso
	GpioSetDirection(port, 7, OUTPUT); //sck

	//enable master
	this->spi->CTRL = SPI_ENABLE_bm | SPI_MASTER_bm;
}

void Spi::Stop()
{
	this->spi->CTRL = 0x00;

	GpioSetDirection(this->port, 4, INPUT); //ss
	GpioSetDirection(this->port, 5, INPUT); //mosi
	GpioSetDirection(this->port, 6, INPUT);  //miso
	GpioSetDirection(this->port, 7, INPUT); //sck
}

void Spi::SetMode(uint8_t mode)
{
	if (mode > 3) return;

	this->spi->CTRL = (this->spi->CTRL & 0b11110011) | (mode << 2);
}

void Spi::SetDataOrder(uint8_t dataorder)
{
	if (dataorder == MSB)
		this->spi->CTRL &= ~SPI_DORD_bm;
	else
		this->spi->CTRL |= SPI_DORD_bm;
}

void Spi::SetSlave(PORT_t * port, uint8_t pin)
{
	this->ss_pin = pin;
	this->ss_port = port;

	GpioWrite(port, pin, LOW);
}

void Spi::UnsetSlave()
{
	GpioWrite(this->ss_port, this->ss_pin, HIGH);
}

void Spi::SetDivider(xlib_core_spi_prescaler div)
{
	this->spi->CTRL = (this->spi->CTRL & 0b01111100) | div;
}


uint8_t Spi::SendRaw(uint8_t data)
{
	this->spi->DATA = data;
	while (!(this->spi->STATUS & SPI_IF_bm));
	return this->spi->DATA;
}


