#include "common.h"
#include "drivers/uart.h"
#include <string.h>

extern float avg_task_loop;
extern float avg_system_loop;
extern float avg_sleep;
extern float avg_irq;
extern float avg_loop;

struct app_info ee_fw_info __attribute__ ((section(".fw_info")));
struct app_info fw_info;

void print_fw_info()
{
	eeprom_busy_wait();
	eeprom_read_block(&fw_info, &ee_fw_info, sizeof(fw_info));

	DEBUG("App name: ");
	for (uint8_t i = 0; i < 32; i++)
	{
		uint8_t c = fw_info.app_name[i];
		if (c < 32 || c > 126)
			c = '_';
		DEBUG("%c", c);
	}
	DEBUG("\n");
}

void print_cpu_usage()
{
	float fcnt = avg_loop;

	//DEBUG("%0.1f %0.1f %0.1f %0.1f\n", avg_task_loop, avg_system_loop, avg_sleep, avg_loop);

	avg_irq = avg_loop - (avg_task_loop + avg_system_loop + avg_sleep);

	//time in application task
	float t1 = (avg_task_loop / fcnt) * 100;
	//time in system task
	float t2 = (avg_system_loop / fcnt) * 100;
	//time in irq
	float t3 = (avg_irq / fcnt) * 100;
	//time in sleep
	float t4 = (avg_sleep / fcnt) * 100;

	DEBUG("CPU %3.1f%% %3.1f%% %3.1f%% %3.1f%%\n", t1, t2, t3, t4);
}

void test_memory()
{
	DEBUG("Free RAM now ... %d\n", freeRam());
}

//----------------------------------------------------------

void turnoff_subsystems()
{
	//PORTA
	PR.PRPA = 0b00000111;
	//PORTB
	PR.PRPB = 0b00000111;
	//PORTC
	PR.PRPC = 0b01111111;
	//PORTD
	PR.PRPD = 0b01111111;
	//PORTE
	PR.PRPE = 0b01111111;
	//PORTF
	PR.PRPF = 0b01111111;
	//PRGEN - RTC must stay on
	PR.PRGEN = 0b01011011;
}

//----------------------------------------------------------

DataBuffer::DataBuffer(uint16_t size)
{
	this->size = size;
	this->length = 0;
	this->write_index = 0;
	this->read_index = 0;

	this->data = new uint8_t[size];
	if (this->data == NULL)
	{
		this->size = 0;
	}
}

DataBuffer::~DataBuffer()
{
	test_memory();
	if (this->size)
	{
		DEBUG("Doing nothing!\n");
		delete[] this->data;
	}
	test_memory();
}

uint16_t DataBuffer::Read(uint16_t len, uint8_t * * data)
{
	(*data) = this->data + this->read_index;

	if (len > this->length)
		len = this->length;

	if (this->read_index + len > this->size)
		len = this->size - this->read_index;

	this->read_index = (this->read_index + len) % this->size;
	this->length -= len;

	return len;
}

bool DataBuffer::Write(uint16_t len, uint8_t * data)
{
//	for (uint16_t i=0;i<len;i++)
//		DEBUG(" %02X", data[i]);
//	DEBUG(" << \n");

	if (this->size - this->length < len)
		return false;

	if (this->write_index + len > this->size)
	{
		uint16_t first_len = this->size - this->write_index;
		memcpy(this->data + this->write_index, data, first_len);
		memcpy(this->data, data + first_len, len - first_len);
	}
	else
	{
		memcpy(this->data + this->write_index, data, len);
	}

	this->write_index = (this->write_index + len) % this->size;
	this->length += len;

//	DEBUG("wi: %d ", this->write_index);
//	DEBUG("ri: %d ", this->read_index);
//	DEBUG("ln: %d\n", this->length);
//
//	DEBUG("S");
//	for (uint16_t i=this->read_index;i < this->read_index + this->length;i++)
//		DEBUG(" %02X", this->data[i]);
//	DEBUG("\n");
//	DEBUG("\n");

	return true;
}

uint16_t DataBuffer::Length()
{
	return this->length;
}

void DataBuffer::Clear()
{
	this->length = 0;
	this->write_index = 0;
	this->read_index = 0;
}

volatile uint8_t bat_en_mask = 0;

void bat_en_high(uint8_t mask)
{
	bat_en_mask |= mask;
	GpioWrite(BAT_EN, HIGH);
}

void bat_en_low(uint8_t mask)
{
	bat_en_mask &= ~mask;

	if (bat_en_mask == 0)
		GpioWrite(BAT_EN, LOW);
}


bool cmpn(char * s1, const char * s2, uint8_t n)
{
	for (uint8_t i = 0; i < n; i++)
	{
		if (s1[i] != s2[i])
			return false;
	}
	return true;
}

bool cmpn_p(char * s1, const char * s2, uint8_t n)
{
	for (uint8_t i = 0; i < n; i++)
	{
		if (s1[i] != pgm_read_byte(&s2[i]))
			return false;
	}
	return true;
}

int freeRam()
{
	extern int __heap_start, *__brkval;
	int v;
	return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
