#ifndef I2C_H_
#define I2C_H_

#include "../common.h"
#include "../ring_small.h"

#define i2c0	&TWIE, 0
#define i2c1	&TWIC, 1

#define i2ce	&TWIE, 0
#define i2cc	&TWIC, 1


#define first_byte			0xFF
#define i2c_defaultSlave	0x02
#define I2c_MASTERMODE		0
#define I2c_SLAVEMODE		!I2c_MASTERMODE

typedef enum xlib_core_i2c_status_e
{
	i2c_unknown = 0,
	i2c_idle	= 1,
	i2c_owner	= 2,
	i2c_busy	= 3
} xlib_core_i2c_status;

typedef enum xlib_core_i2c_slave_status_e
{
	i2c_slave_unknown 		= 0,
	i2c_slave_idle			= 1,
	i2c_slave_error			= 2,
	i2c_slave_busy			= 3,
	i2c_slave_rxoverflow 	= 4,
	i2c_slave_txoverflow	= 5,
	i2c_slave_collision		= 6,
	i2c_slave_ok			= 7
} xlib_core_i2c_slave_status;

#define I2C_NACK 0x01

typedef enum xlib_core_i2c_events_e //! i2c events
{
	i2c_event_rxcomplete = 0,	//!< execute after RX complete routine
	i2c_event_txcomplete = 1	//!< execute after TX complete routine
} xlib_core_i2c_events;

#define xlib_core_i2c_events_count	2

class I2c;

typedef void (*i2c_event_cb_t)(I2c *);

class I2c
{
private:
	TWI_t * i2c;
	bool mode;

	volatile uint8_t address;
	volatile uint8_t rx_length;
	volatile uint8_t error;
	volatile uint8_t isbusy;

	i2c_event_cb_t events[xlib_core_i2c_events_count];

public:
	RingBufferSmall * rx_buffer;
	RingBufferSmall * tx_buffer;

	void InitMaster(TWI_t * twi, uint8_t n, uint32_t baud, RingBufferSmall * rx_buffer, RingBufferSmall * tx_buffer);

	void Write(uint8_t data);
	uint8_t Read();

	void StartTransmittion(uint8_t adress, uint8_t rx_length);
	void ReadData(uint8_t address, uint8_t rx_length);
	void Wait();
	uint8_t Status();

	uint8_t Error();

	void Scan();
	void IrqRequest();

	void RegisterEvent(xlib_core_i2c_events, i2c_event_cb_t cb);

	I2c();
};


#endif /* I2C_H_ */
