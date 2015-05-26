#include "pan1322.h"
#include "../../tasks/tasks.h"
#include <string.h>

extern pan1322 bt_pan1322;
CreateStdOut(bt_pan1322_out, bt_pan1322.StreamWrite);

void pan1322::TxResume()
{
	this->usart.TxComplete();
}

void pan1322::Init()
{
	this->usart.InitBuffers(BUFFER_SIZE, BUFFER_SIZE);
	this->usart.Init(BT_UART, 115200);
	//this->usart.dbg = true;
	//this->usart.SetCtsPin(BT_CTS, LOW);
	this->usart.SetInterruptPriority(MEDIUM);

	GpioSetDirection(BT_EN, OUTPUT);
	GpioSetDirection(BT_RESET, OUTPUT);
	GpioSetDirection(BT_RTS, OUTPUT);
	GpioSetDirection(BT_CTS, INPUT);

	GpioSetInterrupt(BT_CTS, gpio_interrupt1, gpio_rising);



	GpioWrite(BT_RTS, LOW);
	GpioWrite(BT_EN, LOW);
	GpioWrite(BT_RESET, LOW);


	this->connected = false;
}

bool pan1322::SetBaudrate(uint32_t baud)
{
	fprintf_P(bt_pan1322_out, PSTR("AT+JSBR=%07lu\r\n"), baud);

	if (!this->WaitForOK())
	{
		DEBUG1("SetBaudrate FAILED");
		return false;
	}

	this->usart.Stop();

	while(GpioRead(BT_CTS) == HIGH);

	_delay_ms(150);
	this->usart.Init(BT_UART, baud);
	this->usart.dbg = true;
	this->usart.SetCtsPin(BT_RTS, LOW);
	this->usart.SetInterruptPriority(MEDIUM);

	return true;
}

void pan1322::Restart()
{
	DEBUG("reset_start\n");
	this->connected = false;
	this->p_state = BT_STATE_START;


	GpioWrite(BT_EN, LOW);
	GpioWrite(BT_RESET, LOW);

	_delay_ms(200);
	GpioWrite(BT_EN, HIGH);
	_delay_ms(200);
	GpioWrite(BT_RESET, HIGH);
	_delay_ms(300);

	if (!this->WaitForOK())
		DEBUG1("Reset FAILED");
}

void pan1322::StreamWrite(uint8_t data)
{
//	DEBUG("%02X .. %c\n", data);
	this->usart.Write(data);
}

bool pan1322::WaitForOK(uint16_t timeout_ms)
{
	this->ok_ans = false;
	this->error = false;

	while (timeout_ms > 0)
	{
		if (!this->usart.isRxBufferEmpty())
		{
			this->Step();
			if (this->ok_ans == true)
			{
				return true;
			}
			if (this->error)
			{
				this->error = false;
				return false;
			}
		}
		else
		{
			_delay_ms(1);
			timeout_ms--;
		}
	}

	DEBUG("TIMEOUT %d\n", this->usart.tx_len);
	return false;
}

bool pan1322::SetName(const char * name)
{
	uint8_t nameLength = strlen_P(name);
	if (nameLength > 18)
		return false;

	fprintf_P(bt_pan1322_out, PSTR("AT+JSLN=%02i,%S\r\n"), nameLength, name);

	return this->WaitForOK();
}

bool pan1322::SetDiscoverable(bool discoverable)
{
	fprintf_P(bt_pan1322_out, PSTR("AT+JDIS=%i\r\n"), (discoverable) ? 3 : 0);

	return this->WaitForOK();
}

bool pan1322::CreateService(const char * uuid, const char * name, uint8_t channel, const char * deviceClass)
{
	uint8_t nameLength = strlen_P(name);
	if (nameLength > 16)
		return false;

	uint8_t uuidLength = strlen_P(uuid);
	if (uuidLength > 32 || uuidLength < 4)
		return false;

	fprintf_P(bt_pan1322_out, PSTR("AT+JRLS=%02d,%02d,%S,%S,%02d,%S\r\n"), uuidLength, nameLength, uuid, name, channel, deviceClass);

	return this->WaitForOK();
}

void pan1322::Step()
{
	while (!this->usart.isRxBufferEmpty())
	{
//		DEBUG("%c", this->usart.Peek());
		this->Parse(this->usart.Read());
	}
}

bool pan1322::AcceptConnection()
{
	fprintf_P(bt_pan1322_out, PSTR("AT+JACR=1\r\n"));

	return this->WaitForOK();
}

void pan1322::FindRN()
{
//	return;
	uint8_t c;

	while (1)
	{
		c = this->usart.Read();
		//DEBUG("%c", c);
		if (c == '\n')
			return;
	}
}

void pan1322::StreamHead(uint16_t len)
{
	bool first = true;
	while (!this->isIdle())
	{
		if (first)
		{
			DEBUG("---tx lock\n");
			first = false;
		}

		DEBUG("SH %d,%d,%d,%d\n", this->p_state, this->p_len, this->usart.rx_len, this->data_len);

		this->Step();
	}
	if (!first)
	{
		DEBUG("---tx released\n");
	}

	fprintf_P(bt_pan1322_out, PSTR("AT+JSDA=%03d,"), len);
}

void pan1322::StreamTail()
{
	fprintf_P(bt_pan1322_out, "\r\n");
	this->usart.FlushTxBuffer();

	this->WaitForOK();
}

bool pan1322::isIdle()
{
	return this->p_state == BT_STATE_START;
}

bool pan1322::isConnected()
{
	return this->connected;
}

void pan1322::Parse(uint8_t c)
{
	uint8_t param;
	uint8_t tmp;
	uint8_t n;

//	if (c == 10)
//		DEBUG("P \\r,%d,%d,%d,%d\n", this->p_state, this->p_len, this->usart.rx_len, this->data_len);
//	else if (c == 13)
//		DEBUG("P \\n,%d,%d,%d,%d\n", this->p_state, this->p_len, this->usart.rx_len, this->data_len);
//	else
//		DEBUG("P %c,%d,%d,%d,%d\n", c, this->p_state, this->p_len, this->usart.rx_len, this->data_len);

	uint8_t state = this->p_state;
	switch (state)
	{
	case(BT_STATE_START):
		if (c == '+')
		{
			this->p_state = BT_STATE_RESP;
			this->p_len = 4;
			this->p_index = 0;
		}
		if (c == 'E')
		{
			this->p_state = BT_STATE_ERR;
			this->p_len = 4;
			this->p_index = 0;
		}
		if (c == 'O')
		{
			this->p_state = BT_STATE_OK;
		}
	break;

	case(BT_STATE_OK):
		if (c == 'K')
		{
			this->ok_ans = true;
		}
		this->p_state = BT_STATE_START;
		this->FindRN();
	break;

	case(BT_STATE_ERR):
		if (this->p_len > this->p_index)
		{
			this->p_buff[this->p_index] = c;
			this->p_index++;
		}
		else
		{
			if (cmpn(this->p_buff, "RR=-", 4))
			{
				param = c - '0';
				this->error = true;
				this->p_state = BT_STATE_START;
				this->FindRN();
//				task_irqh(TASK_IRQ_BT, param);
				bt_irgh(BT_IRQ_ERROR, &param);
			}
		}
	break;

	case(BT_STATE_RESP):
		if (this->p_len > this->p_index)
		{
			this->p_buff[this->p_index] = c;
			this->p_index++;
		}
		else
		{
			if (cmpn(this->p_buff, "RSLE", 4))
			{
				this->FindRN();

//				param[0] = BT_EVENT_PAIR;
				this->p_state = BT_STATE_START;
//				task_irqh(TASK_IRQ_BT, param);
//				this->AcceptConnection();
				break;
			}
			if (cmpn(this->p_buff, "RCOI", 4))
			{
				//TODO: decode btaddress
				// this is hack
				this->FindRN();

//				param[0] = BT_EVENT_INCOMING;
				this->p_state = BT_STATE_START;
				this->AcceptConnection();
				break;
			}
			if (cmpn(this->p_buff, "RCCR", 4))
			{
				this->FindRN();

//				param[0] = BT_EVENT_CONNECTED;
				this->connected = true;
				this->p_state = BT_STATE_START;
//				task_irqh(TASK_IRQ_BT, param);
				bt_irgh(BT_IRQ_CONNECTED, 0);
				break;
			}
			if (cmpn(this->p_buff, "RDII", 4))
			{
				this->FindRN();

//				param[0] = BT_EVENT_DISCONNECTED;
				this->connected = false;
				this->p_state = BT_STATE_START;
				bt_irgh(BT_IRQ_DISCONNECTED, 0);
				break;
			}
			if (cmpn(this->p_buff, "RDAI", 4))
			{
				this->p_state = BT_STATE_DATA;
				this->p_len = 0;
				break;
			}

			this->p_state = BT_STATE_START;
			this->FindRN();
		}


	break;

	case(BT_STATE_DATA):
		tmp = this->p_len;

		switch(tmp)
		{
		case(0): // first digit
			this->p_len++;

			n = c - '0';
			if (n > 9)
			{
				DEBUG1("?? 2 %c = %02X\n", c, c);
				this->p_state = BT_STATE_START;
				break;
			}
			this->data_len += 100 * n;

			break;
		case(1): // second digit
			this->p_len++;

			n = c - '0';
			if (n > 9)
			{
				DEBUG1("?? 3 %c = %02X\n", c, c);
				this->p_state = BT_STATE_START;
				break;
			}
			this->data_len += 10 * n;

			break;
		case(2): // third digit
			this->p_len++;

			n = c - '0';
			if (n > 9)
			{
				DEBUG1("?? 4 %c = %02X\n", c, c);
				this->p_state = BT_STATE_START;
				break;
			}
			this->data_len += 1 * n;

			break;
		case(3): // semicolon
			this->p_len++;

			if (c != ',')
			{
				DEBUG1("?? 5 %c = %02X\n", c, c);
				this->p_state = BT_STATE_START;
				break;
			}
			break;
		case(4): // data
			this->data_len -= 1;

			if (this->data_len == 0)
			{
				this->FindRN();
				this->p_state = BT_STATE_START;
			}

			//HANDLE INCOMING DATA HERE
			//XXX: hack sa to bude stale nahodne zasekavat tu bude chyba
//			if (rgui.ParserStep(c))
//				task_irqh(TASK_IRQ_RGUI, NULL);
//			break;

		}
		break;
	}
}
