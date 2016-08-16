#include "i2c.h"

#include "../../drivers/uart.h"

extern uint32_t freq_cpu;

I2c * i2cs[] = {NULL, NULL};


I2c::I2c()
{
	//reset events
	uint8_t i;

	for (i=0;i<xlib_core_i2c_events_count;i++)
		this->events[i] = NULL;
}

void I2c::InitMaster(TWI_t * twi, uint8_t n, uint32_t baud, RingBufferSmall * rx_buffer, RingBufferSmall * tx_buffer)
{
	mode = I2c_MASTERMODE;
	this->i2c = twi;

	this->i2c->MASTER.BAUD = (uint32_t)((uint32_t)freq_cpu/(2 * (uint32_t)baud) - 5);


	this->i2c->MASTER.CTRLA = TWI_MASTER_INTLVL_HI_gc | TWI_MASTER_RIEN_bm | TWI_MASTER_WIEN_bm | TWI_MASTER_ENABLE_bm;
	this->i2c->MASTER.CTRLC =  TWI_MASTER_CMD_STOP_gc | TWI_MASTER_ACKACT_bm;
	this->i2c->MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;

	this->rx_buffer = rx_buffer;
	this->tx_buffer = tx_buffer;
	this->error = 0;

	this->isbusy = false;


	i2cs[n] = this;
}

void I2c::Scan()
{
	uint8_t addr;

	DEBUG("scanning...\n");
	for (addr = 0; addr < 128; addr++)
	{
		this->Wait();
		this->StartTransmittion(addr, 0);
		this->Wait();
		if (!this->Error())
			DEBUG("%02X ", addr);
	}
	DEBUG("\ndone.\n");

}

void I2c::Write(uint8_t data)
{
	this->tx_buffer->Write(data);
}

uint8_t I2c::Read()
{
	return this->rx_buffer->Read();
}


void I2c::StartTransmittion(uint8_t address, uint8_t rx_length)
{
	this->rx_length = rx_length;
	this->address = address;

	this->i2c->MASTER.ADDR = (this->address << 1) | (WRITE & 0x01);
	this->isbusy = true;
	this->error = 0;

	this->rx_buffer->Clear();
}

void I2c::ReadData(uint8_t address, uint8_t rx_length)
{
	this->rx_length = rx_length;
	this->address = address;

	this->i2c->MASTER.ADDR = (this->address << 1) | (READ & 0x01);
	this->isbusy = true;

	this->rx_buffer->Clear();
}


void I2c::IrqRequest()
{
	uint8_t stat;

	stat = this->i2c->MASTER.STATUS;

	if (stat & TWI_MASTER_RIF_bm) //read complete
	{
		//if rx_length is 0xFF than read first byte from slave as rx_length
		if (this->rx_length == first_byte)
			this->rx_length = this->i2c->MASTER.DATA;
		else
		{
			this->rx_buffer->Write(this->i2c->MASTER.DATA); //store
			this->rx_length--;
		}

		if (this->rx_length > 0)
		{
			this->i2c->MASTER.CTRLC = TWI_MASTER_CMD_RECVTRANS_gc; //ACK
		}
		else
		{
			this->i2c->MASTER.CTRLC =  (1 << 2) | TWI_MASTER_CMD_STOP_gc; //STOP & NACK
			this->isbusy = false;
		}
	}

	if (stat & TWI_MASTER_WIF_bm) //write complete
	{
		if (!(stat & TWI_MASTER_RXACK_bm))//with ACK
		{
			if (this->tx_buffer->Length() > 0)
				this->i2c->MASTER.DATA = this->tx_buffer->Read();

			if (this->tx_buffer->Length() == 0)
			{
				this->tx_buffer->Clear();

				if (this->rx_length > 0)
				{
					this->i2c->MASTER.CTRLC = TWI_MASTER_CMD_REPSTART_gc; //RESTART
					this->i2c->MASTER.ADDR = (this->address << 1) | (READ & 0x01);
				}
				else
				{
					this->i2c->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc; //STOP
					this->isbusy = false;
				}
			}
		}
		else //no ACK
		{
			this->error = I2C_NACK;
			this->i2c->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc; //STOP
			this->tx_buffer->Clear();
			this->isbusy = false;
		}
	}

}

void I2c::Wait()
{
	while (this->Status() != i2c_idle);
}


uint8_t I2c::Error()
{
	return this->error;
}

uint8_t I2c::Status()
{
	if (this->isbusy)
		return i2c_busy;
	return (this->i2c->MASTER.STATUS & 0b00000011);
}


/**
 * Register event for this module
 *
 * \param event event to trigger callback
 * \param cb callback function
 *
 * \note clear callback using NULL cb parameter
 */
void I2c::RegisterEvent(xlib_core_i2c_events event,  i2c_event_cb_t cb)
{
	this->events[event] = cb;
}


ISR(TWIE_TWIM_vect) { i2cs[0]->IrqRequest(); }
ISR(TWIC_TWIM_vect) { i2cs[1]->IrqRequest(); }
