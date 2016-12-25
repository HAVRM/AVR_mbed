//=========================================================================
//
//	Test program
//
//=========================================================================
//http://www.suzume-syako.jp/personal/Tips/peji/Easy_SDcard_FAT_Library_for_AVR.html#1

#include "avr/io.h"
#include "sd.h"
#include "serial.h"

int main(void) {
	int i;
	int fd;
	char fname[] = "test.txt";
	char buf[16] = "0123456789\r\n";
	
	USART_Init(9600);
	
	PORTC =  _BV(PC0);
	DDRC  |=  _BV(DDC0) | _BV(DDC1) | _BV(DDC2) | _BV(DDC3) | _BV(DDC4);
	
	PORTD |=  _BV(PD2);
	DDRD  |=  _BV(DDD2);

	PORTB |=  _BV(PB2);					// SS high level
	DDRB  |=  _BV(DDB2);				// SS output

	PORTD &= ~_BV(PD3);					// ON
	DDRD  |=  _BV(DDD3);				// LED port output

	// Mount
	if (sd_mount(0) < 0) {
		for(;;) {
			PORTD |= _BV(PD3);				// OFF
			delay_1mTimes(50);
			PORTD &= ~_BV(PD3);				// ON
			delay_1mTimes(50);
		}
	}

	PORTC |= _BV(PC1);
	SD_SET_DATE(2010,7,7);
	SD_SET_TIME(12,34,56);

	// Open
	if ((fd = sd_open(fname, SD_O_CREAT|SD_O_TRUNC, 0)) < 0) {
		for(;;) {
			PORTD |= _BV(PD3);				// OFF
			delay_1mTimes(50);
			PORTD &= ~_BV(PD3);				// ON
			delay_1mTimes(100);
		}
	}

	PORTC |= _BV(PC2);
	// Write
	for(i = 0; i < 2000; i++) {
		if(i/2)PORTD |= _BV(PD2);
		else   PORTD &= ~_BV(PD2);
		buf[0]=USART_getc();
		if (sd_write(fd, buf, 1) <= 0) {
			for(;;) {
				PORTD |= _BV(PD3);			// OFF
				delay_1mTimes(100);
				PORTD &= ~_BV(PD3);			// ON
				delay_1mTimes(50);
			}
		}
	}

	PORTC |= _BV(PC3);
	// Close
	if (sd_close(fd) < 0) {
		for(;;) {
			PORTD |= _BV(PD3);				// OFF
			delay_1mTimes(100);
			PORTD &= ~_BV(PD3);				// ON
			delay_1mTimes(100);
		}
	}

	PORTC |= _BV(PC4);
	// unmount
	if (sd_unmount() < 0) {
		for(;;) {
			PORTD |= _BV(PD3);				// OFF
			delay_1mTimes(200);
			PORTD &= ~_BV(PD3);				// ON
			delay_1mTimes(200);
		}
	}

	PORTC = _BV(PC0);
	PORTD |= _BV(PD3);				// OFF
}
