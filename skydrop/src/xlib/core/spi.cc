#include "spi.h"

//pointers to handle IRQ
Spi * spis[3];

void Spi::InitMaster()
{
	this->InitMaster(spi0);
}

void Spi::InitMaster(SPI_t * spi, PORT_t * port, uint8_t n)
{
	this->InitMaster(spi, port, n, BUFFER_SIZE, BUFFER_SIZE);
}

void Spi::InitMaster(SPI_t * spi, PORT_t * port, uint8_t n, uint8_t size)
{
	this->InitMaster(spi, port, n, size, size);
}

void Spi::InitMaster(SPI_t * spi, PORT_t * port, uint8_t n, uint8_t rx_size, uint8_t tx_size)
{
	this->spi = spi;
	this->port = port;

	if (rx_size > 0)
		this->rx_buffer = new RingBufferSmall(rx_size);
	if (tx_size > 0)
		this->tx_buffer = new RingBufferSmall(tx_size);

	//ss set
//	GpioSetDirection(spi_ss_usb, OUTPUT);
//	GpioSetDirection(spi_ss_sdcard, OUTPUT);
//	GpioSetDirection(spi_ss_user0, OUTPUT);
//	GpioSetDirection(spi_ss_user1, OUTPUT);
//	GpioSetDirection(spi_ss_user2, OUTPUT);
//	GpioSetDirection(spi_ss_user3, OUTPUT);
//
//	GpioWrite(spi_ss_usb, HIGH);
//	GpioWrite(spi_ss_sdcard, HIGH);
//	GpioWrite(spi_ss_user0, HIGH);
//	GpioWrite(spi_ss_user1, HIGH);
//	GpioWrite(spi_ss_user2, HIGH);
//	GpioWrite(spi_ss_user3, HIGH);

	GpioSetDirection(port, 4, OUTPUT); //ss
	GpioSetDirection(port, 5, OUTPUT); //mosi
	GpioSetDirection(port, 6, INPUT);  //miso
	GpioSetDirection(port, 7, OUTPUT); //sck

	//enable master
	this->spi->CTRL = SPI_ENABLE_bm | SPI_MASTER_bm;

	spis[n] = this;
	this->isbusy = false;
}

void Spi::Stop()
{
	this->spi->CTRL = 0x00;

	GpioSetDirection(this->port, 4, INPUT); //ss
	GpioSetDirection(this->port, 5, INPUT); //mosi
	GpioSetDirection(this->port, 6, INPUT);  //miso
	GpioSetDirection(this->port, 7, INPUT); //sck
}

void Spi::InitSlave(SPI_t * spi, PORT_t * port, uint8_t n)
{
	this->InitSlave(spi, port, n, BUFFER_SIZE, BUFFER_SIZE);
}

void Spi::InitSlave(SPI_t * spi, PORT_t * port, uint8_t n, uint8_t size)
{
	this->InitSlave(spi, port, n, size, size);
}

void Spi::InitSlave(SPI_t * spi, PORT_t * port, uint8_t n, uint8_t rx_size, uint8_t tx_size)
{

	this->spi = spi;

	this->rx_buffer = new RingBufferSmall(rx_size);
	this->tx_buffer = new RingBufferSmall(tx_size);

	GpioSetDirection(port, 4, INPUT);   //ss
	GpioSetDirection(port, 5, INPUT);   //mosi
	GpioSetDirection(port, 6, OUTPUT);  //miso
	GpioSetDirection(port, 7, INPUT);   //sck

	//enable slave
	this->spi->CTRL  = SPI_ENABLE_bm & ~SPI_MASTER_bm;

	spis[n] = this;
	this->isbusy = false;
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

void Spi::IrqRequest()
{
	//read
	this->rx_buffer->Write(this->spi->DATA);

	//send next
	if (this->tx_buffer->Length() > 0) {
		this->spi->DATA = this->tx_buffer->Read();
	}
	else
	{
		this->UnsetSlave();
		this->isbusy = false;
		//disable irq
		this->spi->INTCTRL &= ~SPI_INTLVL_LO_gc;
	}

}

void Spi::StartTransmittion(PORT_t * port, uint8_t pin)
{
	if (!this->tx_buffer->Length())
		return;

	//set slave
	this->SetSlave(port, pin);
	//start transmittion

	this->spi->DATA = this->tx_buffer->Read();
	//enable irq
	this->spi->INTCTRL |= SPI_INTLVL_LO_gc;
	this->isbusy = true;
}

uint8_t Spi::SendRaw(uint8_t data)
{
	this->spi->DATA = data;
	while (!(this->spi->STATUS & SPI_IF_bm));
	return this->spi->DATA;
}

void Spi::Clear()
{
	this->tx_buffer->Clear();
}

void Spi::Write(uint8_t data)
{
	this->tx_buffer->Write(data);
}

uint8_t Spi::Read()
{
	return this->rx_buffer->Read();
}

void Spi::Wait()
{
	while (this->isbusy);
}

ISR(SPID_INT_vect) {spis[0]->IrqRequest();}
ISR(SPIC_INT_vect) {spis[1]->IrqRequest();}
