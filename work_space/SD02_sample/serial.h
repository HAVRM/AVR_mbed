/*
 * serial.h
 *
 * Created: 2016/12/18 2:02:25
 *  Author: hiroki.mine
 */ 


#ifndef SERIAL_H_
#define SERIAL_H_

#include "delay.h"

void USART_Init(unsigned int baud)
{
	baud=12;
	UBRR0H = (unsigned char)(baud>>8); /* ﾎﾞｰﾚｰﾄ設定(上位ﾊﾞｲﾄ) */
	UBRR0L = (unsigned char)baud; /* ﾎﾞｰﾚｰﾄ設定(下位ﾊﾞｲﾄ) */
	UCSR0A = 1<<U2X0;
	UCSR0C = 3<<UCSZ00; /* ﾌﾚｰﾑ形式設定(8ﾋﾞｯﾄ,1停止ﾋﾞｯﾄ) */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0); /* 送受信許可 */
}

unsigned char USART_getc(void)
{
	while ( !(UCSR0A & (1<<RXC0)) ); /* 受信完了待機 */
	return UDR0; /* 受信ﾃﾞｰﾀ取得 */
}

#endif /* SERIAL_H_ */