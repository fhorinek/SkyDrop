#include "timer.h"

extern uint32_t freq_cpu;

void Timer::Init(TC0_t * tc0, PORT_t * port, xlib_core_timer_prescaler pres)
{
	this->tc0 = tc0;
	this->tc1 = 0;

	this->Init(port, pres);
}

void Timer::Init(TC1_t * tc1, PORT_t * port, xlib_core_timer_prescaler pres)
{
	this->tc0 = 0;
	this->tc1 = tc1;

	this->Init(port, pres);
}


void Timer::Init(PORT_t * port, xlib_core_timer_prescaler pres)
{
	this->port = port;
	this->prescaler = pres;

	switch (pres)
	{
		case(timer_div1):
				this->pres = 1;
		break;
		case(timer_div2):
				this->pres = 2;
		break;
		case(timer_div4):
				this->pres = 4;
		break;
		case(timer_div8):
				this->pres = 8;
		break;
		case(timer_div64):
				this->pres = 64;
		break;
		case(timer_div256):
				this->pres = 256;
		break;
		case(timer_div1024):
				this->pres = 1024;
		break;
		default:
				this->pres = 0;
	}

	this->SetInterruptPriority(LOW);
}

void Timer::SetPrescaler(xlib_core_timer_prescaler pres)
{
	this->prescaler = pres;

	if (this->tc0)
	{
		if (this->tc0->CTRLA & 0b00001111) //if running
			this->tc0->CTRLA = pres;
	}
	else
	{
		if (this->tc1->CTRLA & 0b00001111) //if running
			this->tc1->CTRLA = pres;
	}
}

void Timer::Start()
{
	if (this->tc0)
		this->tc0->CTRLA = 0b00001111 & this->prescaler;
	else
		this->tc1->CTRLA = 0b00001111 & this->prescaler;
}

void Timer::Stop()
{
	if (this->tc0)
		this->tc0->CTRLA = 0;
	else
		this->tc1->CTRLA = 0;
}

void Timer::SetMode(xlib_core_timer_mode mode)
{
	if (this->tc0)
		this->tc0->CTRLB = (this->tc0->CTRLB & 0b11111000) | mode;
	else
		this->tc1->CTRLB = (this->tc1->CTRLB & 0b11111000) | mode;
}

void Timer::EnableOutputs(uint8_t flags)
{
	if (this->tc0)
	{
		this->tc0->CTRLB |= (0b11110000 & flags);
		if (flags & timer_A) GpioSetDirection(this->port, 0, OUTPUT);
		if (flags & timer_B) GpioSetDirection(this->port, 1, OUTPUT);
		if (flags & timer_C) GpioSetDirection(this->port, 2, OUTPUT);
		if (flags & timer_D) GpioSetDirection(this->port, 3, OUTPUT);
	}
	else
	{
		this->tc1->CTRLB |= (0b11110000 & flags);
		if (flags & timer_A) GpioSetDirection(this->port, 4, OUTPUT);
		if (flags & timer_B) GpioSetDirection(this->port, 5, OUTPUT);
	}
}

void Timer::DisableOutputs(uint8_t flags)
{
	if (this->tc0)
	{
		this->tc0->CTRLB &= ~flags | 0b00001111;
		if (flags & timer_A) GpioSetDirection(this->port, 0, INPUT);
		if (flags & timer_B) GpioSetDirection(this->port, 1, INPUT);
		if (flags & timer_C) GpioSetDirection(this->port, 2, INPUT);
		if (flags & timer_D) GpioSetDirection(this->port, 3, INPUT);
	}
	else
	{
		this->tc1->CTRLB &= ~flags | 0b00001111;
		if (flags & timer_A) GpioSetDirection(this->port, 4, INPUT);
		if (flags & timer_B) GpioSetDirection(this->port, 5, INPUT);
	}
}

void Timer::SetTop(uint16_t top)
{
	if (this->tc0)
		this->tc0->PER = top;
	else
		this->tc1->PER = top;
}

void Timer::SetCompare(uint8_t flags, uint16_t compare)
{
	if (this->tc0)
	{
		if (flags & timer_A)
			this->tc0->CCABUF = compare;
		if (flags & timer_B)
			this->tc0->CCBBUF = compare;
		if (flags & timer_C)
			this->tc0->CCCBUF = compare;
		if (flags & timer_D)
			this->tc0->CCDBUF = compare;
	}
	else
	{
		if (flags & timer_A)
			this->tc1->CCABUF = compare;
		if (flags & timer_B)
			this->tc1->CCBBUF = compare;
	}

}

void Timer::EnableInterrupts(uint8_t inter)
{
	register8_t * regA;
	register8_t * regB;

	if (this->tc0)
	{
		regA = &this->tc0->INTCTRLA;
		regB = &this->tc0->INTCTRLB;
	}
	else
	{
		regA = &this->tc1->INTCTRLA;
		regB = &this->tc1->INTCTRLB;
	}

	if (inter & timer_compareA)
		(*regB) |= this->irq_priority << 0;
	if (inter & timer_compareB)
		(*regB) |= this->irq_priority << 2;
	if (inter & timer_compareC)
		(*regB) |= this->irq_priority << 4;
	if (inter & timer_compareD)
		(*regB) |= this->irq_priority << 6;

	if (inter & timer_overflow)
		(*regA) |= this->irq_priority << 0;
	if (inter & timer_error)
		(*regA) |= this->irq_priority << 2;
}

void Timer::DisableInterrupts(uint8_t inter)
{
	register8_t * regA;
	register8_t * regB;

	if (this->tc0)
	{
		regA = &this->tc0->INTCTRLA;
		regB = &this->tc0->INTCTRLB;
	}
	else
	{
		regA = &this->tc1->INTCTRLA;
		regB = &this->tc1->INTCTRLB;
	}

	if (inter & timer_compareA)
		(*regB) &= 0b11111100;
	if (inter & timer_compareB)
		(*regB) &= 0b11110011;
	if (inter & timer_compareC)
		(*regB) &= 0b11001111;
	if (inter & timer_compareD)
		(*regB) &= 0b00111111;

	if (inter & timer_overflow)
		(*regA) &= 0b11111100;
	if (inter & timer_error)
		(*regA) &= 0b11110011;
}

uint16_t Timer::GetValue()
{
	if (this->tc0)
		return tc0->CNT;
	else
		return tc1->CNT;
}

void Timer::SetValue(uint16_t value)
{
	if (this->tc0)
		tc0->CNT = value;
	else
		tc1->CNT = value;
}

void Timer::SetInterruptPriority(uint8_t p)
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
	}
}


bool Timer::IrqPending(uint8_t inter)
{
	if (this->tc0)
		return tc0->INTFLAGS & inter;
	else
		return tc1->INTFLAGS & inter;
}

void Timer::ClearIrqFlag(uint8_t inter)
{
	if (this->tc0)
		tc0->INTFLAGS = inter;
	else
		tc1->INTFLAGS = inter;
}
