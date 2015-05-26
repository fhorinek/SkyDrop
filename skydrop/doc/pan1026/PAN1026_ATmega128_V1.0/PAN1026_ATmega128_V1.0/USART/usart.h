/*
 * usart.h
 *
 * Author: Panasonic
 */ 


#ifndef USART_H_
#define USART_H_

void USART0_Init(void);
void USART1_Init(void);
void USART0_SendByte( unsigned char data );
void USART1_SendByte( unsigned char data );
void USART0_SendData( unsigned char data[], unsigned char length );
void USART1_SendData( unsigned char data[], unsigned char length );
void USARTS_SendData( unsigned char data[], unsigned char length );

#endif /* USART_H_ */