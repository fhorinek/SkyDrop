#include "i2c.h"

#include "../stdio.h"

extern uint32_t freq_cpu;

I2c * i2cs[] = {NULL, NULL};


I2c::I2c()
{
	//reset events
	uint8_t i;

	for (i=0;i<xlib_core_i2c_events_count;i++)
		this->events[i] = NULL;
}

void I2c::InitMaster()
{
	this->InitMaster(i2c0, 100000ul, BUFFER_SIZE, BUFFER_SIZE);
}

void I2c::InitMaster(TWI_t * twi, uint8_t n, uint32_t baud)
{
	this->InitMaster(twi, n, baud, BUFFER_SIZE, BUFFER_SIZE);
}

void I2c::InitMaster(TWI_t * twi, uint8_t n, uint32_t baud, uint8_t size)
{
	this->InitMaster(twi, n, baud, size, size);
}

void I2c::InitMaster(TWI_t * twi, uint8_t n, uint32_t baud, uint8_t rx_buffer, uint8_t tx_buffer)
{
	mode = I2c_MASTERMODE;
	this->i2c = twi;

	this->i2c->MASTER.BAUD = (uint32_t)((uint32_t)freq_cpu/(2 * (uint32_t)baud) - 5);


	this->i2c->MASTER.CTRLA = TWI_MASTER_INTLVL_HI_gc | TWI_MASTER_RIEN_bm | TWI_MASTER_WIEN_bm | TWI_MASTER_ENABLE_bm;
	this->i2c->MASTER.CTRLC =  TWI_MASTER_CMD_STOP_gc | TWI_MASTER_ACKACT_bm;
	this->i2c->MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;

	this->rx_buffer = new RingBufferSmall(rx_buffer);
	this->tx_buffer = new RingBufferSmall(tx_buffer);
	this->error = 0;

	this->isbusy = false;


	i2cs[n] = this;
}

void I2c::Scan()
{
	uint8_t addr;

	printf("scanning...\n");
	for (addr = 0; addr < 128; addr++)
	{
		this->Wait();
		this->StartTransmittion(addr, 0);
		this->Wait();
		if (!this->Error())
			printf("%02X ", addr);
	}
	printf("\ndone.\n");

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




void I2c::InitSlave()
{
	this->InitSlave(i2c0, i2c_defaultSlave, BUFFER_SIZE, BUFFER_SIZE);
}

void I2c::InitSlave(uint8_t slaveAddress)
{
	this->InitSlave(i2c0, slaveAddress, BUFFER_SIZE, BUFFER_SIZE);
}

void I2c::InitSlave(TWI_t * twi, uint8_t n, uint8_t slaveAddress, uint8_t rx_buffer, uint8_t tx_buffer)
{
	mode = I2c_SLAVEMODE;

	this->i2c = twi;

	this->rx_buffer = new RingBufferSmall(rx_buffer);
	this->tx_buffer = new RingBufferSmall(tx_buffer);
	this->error = 0;

	this->i2c->SLAVE.CTRLA = 	TWI_SLAVE_INTLVL_LO_gc |
            					TWI_SLAVE_DIEN_bm |
            					TWI_SLAVE_APIEN_bm |
            					TWI_SLAVE_ENABLE_bm |
            					TWI_SLAVE_PIEN_bm;
	this->i2c->SLAVE.ADDR = slaveAddress << 1;

	//PMIC.CTRL |= PMIC_LOLVLEN_bm;

	this->isbusy = false;
	i2cs[n] = this;
}

void I2c::IrqSlaveRequest()
{
	uint8_t currentStatus = this->i2c->SLAVE.STATUS;

	/* If bus error. */
	if (currentStatus & TWI_SLAVE_BUSERR_bm) {
		this->rx_buffer->Clear();
		this->tx_buffer->Clear();
		this->error = true; // twi->result = TWIS_RESULT_BUS_ERROR;
		this->SlaveTransactionFinished(i2c_slave_error);
	}

	/* If transmit collision. */
	else if (currentStatus & TWI_SLAVE_COLL_bm) {
		this->rx_buffer->Clear();
		this->tx_buffer->Clear();
		this->error = true; // twi->result = TWIS_RESULT_TRANSMIT_COLLISION;
		this->SlaveTransactionFinished(i2c_slave_collision);
	}

	/* If address match. */
	else if ((currentStatus & TWI_SLAVE_APIF_bm) &&
	        (currentStatus & TWI_SLAVE_AP_bm)) {
		this->SlaveAddressMatchHandler();
	}

	/* If stop (only enabled through slave read transaction). */
	else if (currentStatus & TWI_SLAVE_APIF_bm) {
		this->SlaveStopHandler();
	}

	/* If data interrupt. */
	else if (currentStatus & TWI_SLAVE_DIF_bm) {
		//printf("D");
		this->SlaveDataHandler();
	}

	/* If unexpected state. */
	else {
		this->SlaveTransactionFinished(i2c_slave_unknown); // TWIS_RESULT_FAIL
	}
}

void I2c::SlaveAddressMatchHandler()
{

	this->isbusy = true;

	/* Disable stop interrupt. */
//	this->i2c->SLAVE.CTRLA;
//	uint8_t currentCtrlA = this->i2c->SLAVE.CTRLA;
//	this->i2c->SLAVE.CTRLA = currentCtrlA & ~TWI_SLAVE_PIEN_bm;

	//this->rx_buffer->Clear();
	//this->tx_buffer->Clear();

	if (this->i2c->SLAVE.STATUS & TWI_SLAVE_DIR_bm)
	{
		if (this->events[i2c_event_rxcomplete] != NULL)
			this->events[i2c_event_rxcomplete](this);
	}
	this->i2c->SLAVE.CTRLB = TWI_SLAVE_CMD_RESPONSE_gc;
}

void I2c::SlaveStopHandler()
{
	/* Disable stop interrupt. */
//	uint8_t currentCtrlA = this->i2c->SLAVE.CTRLA;
//	this->i2c->SLAVE.CTRLA = currentCtrlA & ~TWI_SLAVE_PIEN_bm;

	uint8_t currentStatus = this->i2c->SLAVE.STATUS;
	this->i2c->SLAVE.STATUS = currentStatus | TWI_SLAVE_APIF_bm;

	this->SlaveTransactionFinished(i2c_slave_ok);
}

void I2c::SlaveDataHandler()
{
	this->slaveDIR = this->i2c->SLAVE.STATUS & TWI_SLAVE_DIR_bm;
	if (this->slaveDIR)
		this->SlaveWriteHandler();
	else
		this->SlaveReadHandler();
}

void I2c::SlaveReadHandler()
{
	/* Enable stop interrupt. */
//	uint8_t currentCtrlA = this->i2c->SLAVE.CTRLA;
//	this->i2c->SLAVE.CTRLA = currentCtrlA | TWI_SLAVE_PIEN_bm;

	if (this->rx_length < this->rx_buffer->size) {
		uint8_t data = this->i2c->SLAVE.DATA;
		this->rx_buffer->Write(data);
		this->i2c->SLAVE.CTRLB = TWI_SLAVE_CMD_RESPONSE_gc;
	}
	else {
		this->i2c->SLAVE.CTRLB = TWI_SLAVE_ACKACT_bm |
		                         TWI_SLAVE_CMD_COMPTRANS_gc;
		this->SlaveTransactionFinished(i2c_slave_rxoverflow); // TWIS_RESULT_BUFFER_OVERFLOW
	}
}

void I2c::SlaveWriteHandler()
{
	if (!(this->i2c->SLAVE.STATUS & TWI_SLAVE_RXACK_bm))
	{
		if (this->tx_buffer->Length() > 0)
			this->i2c->SLAVE.DATA = this->tx_buffer->Read();
		else
			this->i2c->SLAVE.DATA = 0xFF;
		this->i2c->SLAVE.CTRLB = TWI_SLAVE_CMD_RESPONSE_gc;
	}
	else
	{
		this->i2c->SLAVE.CTRLB = TWI_SLAVE_CMD_COMPTRANS_gc;
		this->SlaveTransactionFinished(i2c_slave_ok);
	}
}

void I2c::SlaveTransactionFinished(xlib_core_i2c_slave_status event)
{

	if (event == i2c_slave_ok)
	{
		if (!this->slaveDIR && this->events[i2c_event_rxcomplete] != NULL)
		{
			this->events[i2c_event_rxcomplete](this);
			this->rx_buffer->Clear();
		}
		else if (this->slaveDIR && this->events[i2c_event_txcomplete] != NULL)
		{
			this->events[i2c_event_txcomplete](this);
			this->tx_buffer->Clear();
		}
	}
	else printf("?%i\n", event);

	this->isbusy = false;
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
ISR(TWIE_TWIS_vect) { i2cs[0]->IrqSlaveRequest(); }
ISR(TWIC_TWIS_vect) { i2cs[1]->IrqSlaveRequest(); }
