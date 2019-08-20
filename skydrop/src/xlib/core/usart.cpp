#include "usart.h"
#include "util/atomic.h"

#include "../../skydrop.h"

//pointers to handle IRQ
Usart * usarts[7];

CreateStdIn(usart_default_in, usarts[0]->Read);
CreateStdOut(usart_default_out, usarts[0]->Write);


//from clock.cc
extern uint32_t freq_cpu;

float xlib_core_usart_pow(int8_t exp)
{
    float res = 1;

    if (exp > 0)
        while (exp > 0)
        {
            exp--;
            res = res * 2;
        }
    if (exp < 0)
        while (exp < 0)
        {
            exp++;
            res = res / 2;
        }
    return res;
}

/**
 * Register event fore this module
 *
 * \param event event to trigger callback
 * \param cb callback function
 *
 * \note clear callback using NULL cb parameter
 */
void Usart::RegisterEvent(xlib_core_usart_events event, usart_event_cb_t cb)
{
	this->events[event] = cb;
}

Usart::Usart(uint16_t rx_size, uint8_t * rx_buffer, uint16_t tx_size, uint8_t * tx_buffer)
{
	//init buffers
	this->rx_buffer_size = rx_size;
	this->rx_buffer = rx_buffer;

	this->tx_buffer_size = tx_size;
	this->tx_buffer = tx_buffer;

	this->dma = false;
}


/**
 * Initialize usart (Quick init)
 *
 * init usart on usb port at baud 9600
 *
 */
void Usart::Init()
{
	this->Init(usart0, 9600ul);

	SetStdIO(usart_default_in, usart_default_out);
}

/**
 * Initialize usart (Quick init)
 *
 * \param baud Baudrate
 *
 */
void Usart::Init(uint32_t baud)
{
	this->Init(usart0, baud);

	SetStdIO(usart_default_in, usart_default_out);
}

/**
 * Initialize usart
 *
 * \param usart Usart structure
 * \param port Port structure
 * \param tx Number of tx pin
 * \param n Object index for ISR handling
 * \param baud Baudrate
 *
 * \note There are predefined usart modules like usart0, usart1 which contains usart and port structure, tx pin number and module index
 */
void Usart::Init(USART_t * usart, PORT_t * port, uint8_t tx, uint8_t n, uint32_t baud)
{
	//TODO:why is 1M not working?
	uint32_t new_baud;

	uint16_t bsel; //12 bit
	int8_t bscale; //-7 - 7

	uint16_t bsel_best = 0;
	int8_t bscale_best = 0;
	float error = 1;

	uint8_t mul = 16;

	uint8_t i;

	//reset events
	for (i=0;i<xlib_core_usart_events_count;i++)
		this->events[i] = NULL;

//	if (baud > freq_cpu/16)
//		mul = 8;
//
//	for (bscale = 7; bscale>=-7; bscale--)
//	{
//		if (bscale >= 0)
//		{
//			bsel = (uint16_t)((float)freq_cpu / (xlib_core_usart_pow(bscale)*mul*(float)baud) - 1);
//			new_baud = (uint32_t)((uint32_t)freq_cpu/(xlib_core_usart_pow(bscale)*mul*((uint16_t)bsel + 1)));
//		}
//		else
//		{
//			bsel = (uint16_t)((float)(1 / xlib_core_usart_pow(bscale)) * (float)(freq_cpu / (mul*(float)baud) - 1));
//			new_baud = (uint32_t)((uint32_t)freq_cpu/(mul*(xlib_core_usart_pow(bscale)*((uint16_t)bsel) + 1)));
//		}
//
//		if ((error > abs(1 - (float)((float)new_baud / (float)baud))) && (bsel < 1024))
//		{
//			bsel_best = bsel;
//			bscale_best = bscale;
//			error = abs(1 - (float)((float)new_baud / (float)baud));
//		}
//	}

	switch (baud)
	{
		case(9600ul):
			bsel_best = 3317;
			bscale_best = -4;
		break;

		case(19200ul):
			bsel_best = 3301;
			bscale_best = -5;
		break;

		case(38400ul):
			bsel_best = 3269;
			bscale_best = -6;
		break;

		case(57600ul):
			bsel_best = 2158;
			bscale_best = -6;
		break;

		case(115200ul):
			bsel_best = 2094;
			bscale_best = -7;
		break;

		case(921600ul):
			bsel_best = 150;
			bscale_best = -7;
		break;

	}

	GpioSetDirection(port, tx, OUTPUT);
	GpioWrite(port, tx, HIGH);

	this->dbg = false;

	this->spi_mode = false;
	this->sck_port = port;
	this->sck_pin = tx - 2;

	this->usart = usart;

	//set baudrate
	this->usart->BAUDCTRLA = bsel_best;
	this->usart->BAUDCTRLB = (bscale_best << 4) | (0x0F & (bsel_best >> 8));

	//reset CTRL
	this->usart->CTRLB = 0;
	this->usart->CTRLA = 0;

	//set doublespeed operation
//	if (mul == 8)
//		this->usart->CTRLB |= USART_CLK2X_bm;

	//reset counters
	this->write_index = 0;
	this->read_index = 0;
	this->rx_index = 0;
	this->rx_len = 0;
	this->tx_index = 0;
	this->tx_len = 0;

	//set format
	this->usart->CTRLC = USART_CHSIZE_8BIT_gc;

	//clear interrupts
	this->usart->STATUS = 0b11000000;

	//enable RX & int
	if (this->rx_buffer_size)
	{
		this->usart->CTRLB |= USART_RXEN_bm;
	}
	//enable RX & int
	if (this->tx_buffer_size)
	{
		this->usart->CTRLB |= USART_TXEN_bm;
	}

	//this has to be before IRQ settings so the hander can find the objects
	usarts[n] = this;

	this->irq_priority = LOW;
	this->SetInterruptPriority(LOW);

	this->rx_ovf = false;
}

void Usart::SetupRxDMA(DMA_CH_t * ch, uint8_t trig)
{
	byte2 tmp_adr;

	//disable rx IRQ
	this->usart->CTRLA &= 11001111;

	//assign DMA channel
	this->dma_rx_ch = ch;

	//source address is usart data
	tmp_adr.uint16 = (uint16_t)(&this->usart->DATA);

	this->dma_rx_ch->SRCADDR0 = tmp_adr.uint8[0];
	this->dma_rx_ch->SRCADDR1 = tmp_adr.uint8[1];
	this->dma_rx_ch->SRCADDR2 = 0;

	//destination address is begining of the rx buffer
	tmp_adr.uint16 = (uint16_t)this->rx_buffer;

	this->dma_rx_ch->DESTADDR0 = tmp_adr.uint8[0];
	this->dma_rx_ch->DESTADDR1 = tmp_adr.uint8[1];
	this->dma_rx_ch->DESTADDR2 = 0;

	//source address is fixed
	this->dma_rx_ch->ADDRCTRL = DMA_CH_SRCRELOAD_NONE_gc | DMA_CH_SRCDIR_FIXED_gc |
	//destination address is incremental and reload after whole block
			DMA_CH_DESTRELOAD_BLOCK_gc | DMA_CH_DESTDIR_INC_gc;

	//infinite repeat
	this->dma_rx_ch->REPCNT = 0;

	//block size is size of rx buffer
	this->dma_rx_ch->TRFCNT = this->rx_buffer_size;

	//trigger should be the rx complete on uart peripheral
	this->dma_rx_ch->TRIGSRC = trig;

	//dma channel enable
	this->dma_rx_ch->CTRLA |= DMA_CH_ENABLE_bm | DMA_CH_REPEAT_bm | DMA_CH_SINGLE_bm;

	this->dma = true;
}

uint16_t Usart::GetRxLen()
{
	if (this->dma)
	{
		uint16_t pos = this->rx_buffer_size - this->dma_rx_ch->TRFCNT;
		if (pos >= this->read_index)
			return pos - this->read_index;
		else
			return this->rx_buffer_size - (this->read_index - pos);
	}
	else
	{
		return this->rx_len;
	}
}

void Usart::DumpDMA()
{
	DEBUG("CTRLA:    %02X\n", this->dma_rx_ch->CTRLA);
	DEBUG("CTRLB:    %02X\n", this->dma_rx_ch->CTRLB);
	DEBUG("REPCNT:   %02X\n", this->dma_rx_ch->REPCNT);
	DEBUG("TRFCNT:   %04X\n", this->dma_rx_ch->TRFCNT);
	DEBUG("ADDRCTRL: %02X\n", this->dma_rx_ch->ADDRCTRL);
	DEBUG("rx_len: %u\n", this->GetRxLen());
	DEBUG("DumpBuffer: \n");
	for (uint16_t i = 0; i < this->rx_buffer_size; i++)
	{
		DEBUG("%c", this->rx_buffer[i]);
	}
	DEBUG("\n");
}

/**
 * Send all data remaning in tx buffer, stop usart module, disable Rx/Tx, free buffers
 */
void Usart::Stop()
{
	if (this->dma)
	{
		this->dma_rx_ch->CTRLA = 0;
		this->dma = false;
	}

	//wait to send all data
	this->FlushTxBuffer();

	//clear int and RX TX(data->rx_index)++
	this->usart->CTRLB = 0;
	this->usart->CTRLA = 0;
}

/**
 * Standard RX complete interrupt handler
 */
void Usart::RxComplete()
{
	if ((this->usart->STATUS & (USART_FERR_bm | USART_BUFOVF_bm | USART_PERR_bm )) == 0)
	{
		this->rx_buffer[(this->rx_index)++] = this->usart->DATA;
		if (this->rx_index == this->rx_buffer_size)
			this->rx_index = 0;
		if (++(this->rx_len) == this->rx_buffer_size)
		{
			//this is overflow!
			this->rx_len = 0;
			this->rx_ovf = true;
		}
	}
	else
	{
		//dummy read
		uint8_t a = this->usart->DATA;
		a++;
	}

	if (this->events[usart_event_rxcomplete] != NULL)
		this->events[usart_event_rxcomplete](this);
}

/**
 * Standard TX complete interrupt handler
 */
void Usart::TxComplete()
{
	if (this->tx_len)
	{
		(this->tx_len)--;
		this->usart->DATA = this->tx_buffer[(this->tx_index)++];
		if (this->tx_index == this->tx_buffer_size)
			this->tx_index = 0;
	}

	if (this->events[usart_event_txcomplete] != NULL)
		this->events[usart_event_txcomplete](this);

	//if no data to tx disable DRE irq
	if (this->tx_len == 0)
		this->usart->CTRLA &= 0b11111100;
}

/**
 * Test if TX buffer is empty (all data in TX buffer transmitted)
 *
 * \return true if TX buffer is empty
 */
bool Usart::isTxBufferEmpty()
{
	return (this->tx_len == 0);
}

/**
 * Test if RX buffer is empty (all received data read from RX buffer)
 *
 * \return true if RX buffer is empty
 */
bool Usart::isRxBufferEmpty()
{
	return (this->GetRxLen() == 0);
}

/**
 * Block until all data in TX buffer is transmitted
 */
void Usart::FlushTxBuffer()
{
	//wait to send all data
	while (this->tx_len);// || !this->usart->STATUS & USART_TXCIF_bm);
}

void Usart::ClearRxBuffer()
{
	//drop all data
	this->rx_len = 0;
	this->rx_index = this->GetRxLen();
}


/**
 * Set interrupt priority for this usart module
 *
 * \param p interrupt priority
 */
void Usart::SetInterruptPriority(uint8_t p)
{
	switch (p)
	{
	case(HIGH):
		this->irq_priority = 0b11;
	break;
	case(MEDIUM):
		this->irq_priority = 0b10;
	break;
	case(LOW):
		this->irq_priority = 0b01;
	break;
	case(NONE):
		this->irq_priority = 0b00;
	break;
	}

	this->usart->CTRLA &= 0b11001100;

	if (this->rx_buffer_size)
		this->usart->CTRLA |= this->irq_priority << 4;

	if (this->tx_buffer_size)
		this->usart->CTRLA |= this->irq_priority << 0;
}

/**
 * Write character to TX buffer
 *
 * \param c character to write
 */
void Usart::Write(uint8_t c)
{
	while (this->tx_len == this->tx_buffer_size);

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if (this->tx_len || !(this->usart->STATUS & USART_DREIF_bm))
		{
			cli();
			this->tx_len++;
			this->tx_buffer[(this->write_index)++] = c;
			sei();
			if (this->write_index == this->tx_buffer_size)
				this->write_index = 0;

			//enable DRE interupts
			this->usart->CTRLA |= this->irq_priority << 0;
		}
		else
		{
			this->usart->DATA = c;
		}
	}
}

/**
 * Read character from RX buffer
 *
 * \return character form RX buffer
 * \note this function will block if there is no character in RX buffer (use isRxBufferEmpty or Peek for non blocking behavior)
 */
uint8_t Usart::Read()
{
	uint8_t c;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		while (this->GetRxLen() == 0);

		c = this->rx_buffer[(this->read_index)++];
		if (this->read_index == this->rx_buffer_size)
			this->read_index = 0;

		if (!this->dma)
			this->rx_len--;
	}

	return c;
}

/**
 * Return first byte from RX buffer but do not increse read index
 *
 * \return first byte from RX buffer
 */
uint8_t Usart::Peek()
{
	while (this->GetRxLen() == 0);

	return this->rx_buffer[this->read_index];
}

void Usart::BecomeSPI(uint8_t mode, uint8_t dataorder, uint32_t speed)
{
	this->spi_mode = true;

	GpioSetDirection(this->sck_port, this->sck_pin, OUTPUT);

	uint8_t ctrlc = USART_CMODE_MSPI_gc;

	if (dataorder == LSB)
		ctrlc |= 0b00000100; //UDORD

	switch (mode)
	{
	case (0):
		GpioSetInvert(this->sck_port, this->sck_pin, OFF);
	break;
	case (1):
		GpioSetInvert(this->sck_port, this->sck_pin, OFF);
		ctrlc |= 0b00000010;
	break;
	case (2):
		GpioSetInvert(this->sck_port, this->sck_pin, ON);
	break;
	case (3):
		GpioSetInvert(this->sck_port, this->sck_pin, ON);
		ctrlc |= 0b00000010;
	break;

	}

	//enable RX
	this->usart->CTRLB |= USART_RXEN_bm;
	//enable TX
	this->usart->CTRLB |= USART_TXEN_bm;

	uint16_t bsel = freq_cpu / (2 * speed) - 1;

	this->usart->BAUDCTRLA = bsel;
	this->usart->BAUDCTRLB = 0x0F & (bsel >> 8);

	this->usart->CTRLC = ctrlc;
}


uint8_t Usart::SendRaw(uint8_t data)
{
	while ((this->usart->STATUS & USART_DREIF_bm) == 0);
	this->usart->DATA = data;
	while ((this->usart->STATUS & USART_TXCIF_bm) == 0);
	this->usart->STATUS = USART_TXCIF_bm;

	return this->usart->DATA;
}

void Usart::SendRaw1(uint8_t data)
{
	while ((this->usart->STATUS & USART_DREIF_bm) == 0);
	this->usart->DATA = data;
	while ((this->usart->STATUS & USART_TXCIF_bm) == 0);
	this->usart->STATUS = USART_TXCIF_bm;
}


//static IRQ handlers
void UsartRxInteruptHandler(uint8_t n)
{
	usarts[n]->RxComplete();
}

void UsartTxInteruptHandler(uint8_t n)
{
	usarts[n]->TxComplete();
}

#include "../../drivers/uart.h"

//IRQ mapping
//usart0
ISR(USARTE0_RXC_vect){UsartRxInteruptHandler(0);}
ISR(USARTE0_DRE_vect){UsartTxInteruptHandler(0);}
//usart1
ISR(USARTE1_RXC_vect){UsartRxInteruptHandler(1);}
ISR(USARTE1_DRE_vect){UsartTxInteruptHandler(1);}
//usart2
ISR(USARTF0_RXC_vect){UsartRxInteruptHandler(2);}
ISR(USARTF0_DRE_vect){UsartTxInteruptHandler(2);}
//usart3
ISR(USARTC0_RXC_vect){UsartRxInteruptHandler(3);}
ISR(USARTC0_DRE_vect){UsartTxInteruptHandler(3);}
//usart4
ISR(USARTC1_RXC_vect){UsartRxInteruptHandler(4);}
ISR(USARTC1_DRE_vect){UsartTxInteruptHandler(4);}
//usart5
ISR(USARTD1_RXC_vect){UsartRxInteruptHandler(5);}
ISR(USARTD1_DRE_vect){UsartTxInteruptHandler(5);}
//usart6
ISR(USARTD0_RXC_vect){UsartRxInteruptHandler(6);}
ISR(USARTD0_DRE_vect){UsartTxInteruptHandler(6);}
