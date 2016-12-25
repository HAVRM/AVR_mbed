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
	UBRR0H = (unsigned char)(baud>>8); /* �ްڰĐݒ�(����޲�) */
	UBRR0L = (unsigned char)baud; /* �ްڰĐݒ�(�����޲�) */
	UCSR0A = 1<<U2X0;
	UCSR0C = 3<<UCSZ00; /* �ڰь`���ݒ�(8�ޯ�,1��~�ޯ�) */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0); /* ����M���� */
}

unsigned char USART_getc(void)
{
	while ( !(UCSR0A & (1<<RXC0)) ); /* ��M�����ҋ@ */
	return UDR0; /* ��M�ް��擾 */
}

#endif /* SERIAL_H_ */