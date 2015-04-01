#ifndef USART_H_
#define USART_H_

#include <avr/io.h>
#include "common.h"


//for baud 115200@16M
//#define BSEL	983
//#define BSCALE	-7

//for baud 115200@32M
//#define BSEL	2094
//#define BSCALE	-7

//for baud 921600@32M
#define BSEL	150
#define BSCALE	-7

void usart_init();
void usart_deinit();
void usart_putchar(char data);
void usart_putstr(char * str);

#endif /* USART_H_ */
