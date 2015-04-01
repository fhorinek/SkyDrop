#include "usart.h"

void usart_deinit()
{
	USART_USART.BAUDCTRLA = 0;
	USART_USART.BAUDCTRLB = 0;
	USART_USART.CTRLC = 0b00000110;
	USART_USART.CTRLB = 0;
	USART_USART.CTRLA = 0;

	USART_PORT.OUT = 0;
	USART_PORT.DIR = 0;
}

void usart_init()
{
	//set port dir
	USART_PORT.OUT |= (1 << UART_TX_PIN);
	USART_PORT.DIR |= (1 << UART_TX_PIN);

	//set baudrate
	USART_USART.BAUDCTRLA = BSEL;
	USART_USART.BAUDCTRLB = (BSCALE << 4) | (0x0F & (BSEL >> 8));


	//set format
	USART_USART.CTRLC = USART_CHSIZE_8BIT_gc;

	//enable RX TX
	USART_USART.CTRLB = USART_TXEN_bm;

}

void usart_putchar(char data)
{
	USART_USART.DATA = data;
	while ((USART_USART.STATUS & USART_TXCIF_bm) == 0);
	USART_USART.STATUS = USART_TXCIF_bm;
}

void  usart_putstr(char * str)
{
	while (*str != '\0')
	{
		usart_putchar(*str);
		str++;
	}
}
