//
//  sd_L1.c
//
//  SDcard control module mini - Layer 1 Physical block I/O (SDcard SPI protocol)
//  Copyright (C) 2009-2010 Toyohiko Togashi tog001@nifty.com
//
//  This program is free software; you can redistribute it and/or modify it under the terms of the
//  GNU General Public License as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
//  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along with this program.
//  If not, see <http://www.gnu.org/licenses/>
//
//
//	Update history
//	---------- ----- -------------------------------------------
//	2009.12.18 v0.0  First cording
//	2010.01.11 v0.1  Support PIC18F2550
//	2010.01.16 v0.1a Split source
//	2010.02.19 v0.1b Bug fix, do not use printf()
//	2010.07.07 v0.2  Support ATmega328P
//

#include "sd_config.h"					// Private
#include "sd_L1.h"						// Private
#include "delay.h"						// Private

// Disable DEBUG code					// ADD 2010.02.19
#ifndef UDEBUG							// ADD 2010.02.19
#define printf(a,...) 					// ADD 2010.02.19
#define i2c_lcd_position(a,b)			// ADD 2010.02.19
#define i2c_lcd_prCString(a)			// ADD 2010.02.19
#define i2c_lcd_prHexByte(a)			// ADD 2010.02.19
#define i2c_lcd_prHexInt(a)				// ADD 2010.02.19
#endif									// ADD 2010.02.19



//*********************************************************************
//
// Layer 0	Hardware interface (HAL)
//
//*********************************************************************

//---------------------------------------------------------------------
// Cypress PSoC1
//---------------------------------------------------------------------
#ifdef _M8C
#include <m8c.h>						// part specific constants and macros
#include "PSoCAPI.h"					// PSoC API definitions for all User Modules
// Use PSoC Designer SPIM user module
//		void SD_SPI_Start(mode);
//		void SD_SPI_Stop();
//		void SD_SPI_SendTxData(c);
//		char SD_SPI_bReadRxData();
//		char SD_SPI_bReadStatus();
// Use PSoC Designer LED user module
//		void SD_CS_Start();
//		void SD_CS_Stop();
//		void SD_CS_On();
//		void SD_CS_Off();

#define sd_L0_init()	(SD_CS_Start(), SD_CS_Off(), SD_SPI_Start(SD_SPI_SPIM_MODE_0 | SD_SPI_SPIM_MSB_FIRST))
#define sd_L0_term()	(SD_SPI_Stop(),SD_CS_Stop())
#define sd_L0_cson()	SD_CS_On()
#define sd_L0_csoff()	SD_CS_Off()

void sd_L0_send(unsigned char sdata) {
	SD_SPI_SendTxData(sdata);
	while(!(SD_SPI_bReadStatus() & SD_SPI_SPIM_SPI_COMPLETE));
	return;
}

unsigned char sd_L0_recv(int retry) {	// Recieve data, 0xff=TimeOut
	unsigned char t;
	unsigned char r;

	if (retry > 0) {
		t = 0xff;
	} else {
		t = 0x00;
		retry = -retry;
	}
	for(; retry > 0; --retry) {
		SD_SPI_SendTxData(0xff);
		while(!(SD_SPI_bReadStatus() & SD_SPI_SPIM_SPI_COMPLETE));
		r = SD_SPI_bReadRxData();
		if (r != t) {
			break;
		}
#if SD_WAIT_UNIT > 0
		delay_50uTimes(SD_WAIT_UNIT);
#endif
	}
	return(r);
}
#endif // _M8C


//---------------------------------------------------------------------
// Microchip PIC18F
//---------------------------------------------------------------------
#ifdef __18CXX
#include "p18cxxx.h"					// C18 library
#include "spi.h"						// C18 library

//#define sd_L0_init()	(OpenSPI(SPI_FOSC_16, MODE_00, SMPMID),TRISCbits.TRISC6 = 0)
#define sd_L0_init()	(OpenSPI(SD_L1_SPICLKDIV, MODE_00, SMPMID),TRISCbits.TRISC6 = 0)
#define sd_L0_term()	CloseSPI()
#define sd_L0_cson()	(PORTCbits.RC6 = 0)
#define sd_L0_csoff()	(PORTCbits.RC6 = 1)
#define sd_L0_send(c) 	WriteSPI(c)

unsigned char sd_L0_recv(int retry) {	// Recieve data, 0xff=TimeOut
	unsigned char t;
	unsigned char r;

	if (retry > 0) {
		t = 0xff;
	} else {
		t = 0x00;
		retry = -retry;
	}

	for(; retry > 0; --retry) {
//................................................................
		unsigned char TempVar;			// Copy from c18/src/spi.c

		TempVar = SSPBUF;				// Clear BF
		PIR1bits.SSPIF = 0;				// Clear interrupt flag
		SSPBUF = 0xff;					// initiate bus cycle  ==> Special 0x00->0xff
		while(!PIR1bits.SSPIF);			// wait until cycle complete
		r = SSPBUF;
//................................................................
		if (r != t) {
			break;
		}
#if (SD_WAIT_UNIT > 0)
		delay_50uTimes(SD_WAIT_UNIT);
#endif
	}

	return(r);
}
#endif // __18CXX


//---------------------------------------------------------------------
// ATMEL AVR ATmega SPI unit			ADD 2010.07.07
//---------------------------------------------------------------------
#ifdef __AVR__
#include "avr/io.h"						// WinAVR library

#define sd_L0_cson()	(PORTB &= ~_BV(PB2))
#define sd_L0_csoff()	(PORTB |=  _BV(PB2))

void sd_L0_init(void) {
	// SPI master mode
	DDRB  |=  _BV(DDB5);				// MOSI output
	DDRB  &= ~_BV(DDB4);				// MISO input
	DDRB  |=  _BV(DDB3);				// SCK output
	DDRB  |=  _BV(DDB2);				// SS output
	PORTB |=  _BV(PB2);					// SS high level
	SPCR =								// SPI Control Register
		//	_BV(SPIE) |					// SPI Interrupt Enable
			_BV(SPE)  |					// SPI Enable
		//	_BV(DORD) |					// Data Order   1:LSB 0:MSB
			_BV(MSTR) |					// Master/Slave Select  1:Master 0:Slave
		//	_BV(CPOL) |					// Clock Polarity         0  0  1  1
		//	_BV(CPHA) |					// Clock Phase            0  1  0  1
										//                        +--|--|--|-- SPI Mode 0 <==
										//                           +--|--|-- SPI Mode 1
										//                              +--|-- SPI Mode 2
										//                                 +-- SPI Mode 3
		//	_BV(SPI2X) <- SPSR			// Double SPI Speed Bit   0  0  0  0  1  1  1  1
		//	_BV(SPR1) |					// SPI Clock Rate Select  0  0  1  1  0  0  1  1
		//	_BV(SPR0) |					//                        0  1  0  1  0  1  0  1
										//                        +--|--|--|--|--|--|--|-- 1/4
										//                           +--|--|--|--|--|--|-- 1/16
										//                              +--|--|--|--|--|-- 1/64
										//                                 +--|--|--|--|-- 1/128
										//                                    +--|--|--|-- 1/2
										//                                       +--|--|-- 1/8
										//                                          +--|-- 1/32
										//                                             +-- 1/64
			SD_L1_SPICLKDIV |			// SPR0,1 <= sd_config.h
			0;
	SPSR =								// SPI Status Register
		//	_BV(SPIF)  |				// SPI Interrupt Flag
		//	_BV(WCOL)  |				// Write COLlision Flag
			_BV(SPI2X) |				// Double SPI Speed Bit -> See SPCR
			0;


	return;
}

void sd_L0_term() {
	SPCR = 0;
}

void sd_L0_send(unsigned char c) {
	SPDR = c;
	while(!(SPSR & _BV(SPIF)));

	return;
}

unsigned char sd_L0_recv(int retry) {	// Recieve data, 0xff=TimeOut
	unsigned char t;

	if (retry > 0) {
		t = 0xff;
	} else {
		t = 0x00;
		retry = -retry;
	}

	for(; retry > 0; --retry) {
		SPDR = 0xff;
		while(!(SPSR & _BV(SPIF)));
		if (SPDR != t) {
			break;
		}
#if (SD_WAIT_UNIT > 0)
		delay_50uTimes(SD_WAIT_UNIT);
#endif
	}

	return(SPDR);
}
#endif // __AVR__
// ADD END 2010.07.07


//*********************************************************************
//
// Layer 1	Physical block I/O		(512byte/IO, SPI interface)
//
//*********************************************************************


//---------------------------------------------------------------------
//	SDcard SPI mode control
//---------------------------------------------------------------------

//
//	Generate cyclical redundancy check code
//
#ifdef SD_OPT_CRC
unsigned char sd_L1_crc7(char *buf, int len) {
	unsigned short	a;				// 0xAABB  A=XORbit B=NextData
	signed char		j;
	
	a = 0;
	for(len += 1; len > 0; --len) {
		if (len > 1) {
			a |= (unsigned char)*buf++;
		}
		for(j = 8; j > 0; --j) {
			if (a & 0x8000) {
				a ^= 0x8900;		// Polynom(high bit on)
			}
			a <<= 1;
		}
	}
	return(a >> 9);
}

#ifdef SD_OPT_BUF256
unsigned short sd_L1_crc16(char *buf1, char *buf2, int len) {	// len <= 512
	unsigned long	a;					// 0x00AAAABB  A=XORbit B=NextData
	int				i;
	int				j;
	signed char		k;
	
	a = 0;
	j = len + 2;
	for(i = 0; i < j; i++) {
		if (i < len) {
			if (i & 0x100) {
				a |= (unsigned char)*buf2++;
			} else {
				a |= (unsigned char)*buf1++;
			}
		}
		for (k = 8; k > 0; --k) {
			a <<= 1;
			if (a & 0x01000000) {
				a ^= 0x01102100;		// Polynom(high bit on)
			}
		}
	}
	return(a >> 8);
}
#else
unsigned short sd_L1_crc16(char *buf, int len) {	// len <= 512
	unsigned long	a;					// 0x00AAAABB  A=XORbit B=NextData
	int				i;
	int				j;
	signed char		k;
	
	a = 0;
	j = len + 2;
	for(i = 0; i < j; i++) {
		if (i < len) {
			a |= (unsigned char)*buf++;
		}
		for (k = 8; k > 0; --k) {
			a <<= 1;
			if (a & 0x01000000) {
				a ^= 0x01102100;		// Polynom(high bit on)
			}
		}
	}
	return(a >> 8);
}
#endif // SD_OPT_BUF256

#endif // SD_OPT_CRC


//
//	Send SD command and Recieve response
//
unsigned char sd_L1_cmd_r1(char cmd, unsigned long prm) {	// Response code,  0:normal
	char			buf[6];	// In stack
//	char			i;					// DEL 2010.07.07
	unsigned char	i;					// ADD 2010.07.07
	char			j;
	unsigned char	r;
	
	buf[0] = (cmd & 0x3f) | 0x40;
	buf[1] = prm >> 24;
	buf[2] = prm >> 16;
	buf[3] = prm >> 8;
	buf[4] = prm;
#ifdef SD_OPT_CRC
	buf[5] = (sd_L1_crc7(buf, sizeof(buf) - 1) << 1) | 0x01;
#else
	buf[5] = 0x95;						// CMD0(GO_IDLE_STATE)CRC
#endif

	// Try loop
	for(j = SD_RETRY_CMD; j > 0; --j) {

		// Command
		sd_L0_send(0xff);				// Stable (Kingston accept)
		for(i = 0; i < sizeof(buf); i++) {
			sd_L0_send(buf[i]);
		}
	
		// Response
		r = sd_L0_recv(SD_WAIT_RESPONSE);
		if ((r & ~SD_R1_IDLE_STATE) == 0x00) {
			break;
		}
	}

	return(r);
}


//
//	Open: Initiate SDcard device
//
unsigned char sd_L1_open(void){			// Response code, 0:normal
	int				i;
	unsigned char	r;
	
	// Attach User module
	sd_L0_init();
	
	// Power on, wait init sequence
	for(i = SD_WAIT_POWERUP; i > 0; --i) {
		sd_L0_send(0xff);
	}

	// Card reset
	sd_L0_cson();
	r = sd_L1_cmd_r1(0, 0);				// GO_IDLE_STATE
	sd_L0_send(0xff);					// stable (go to standby)
	sd_L0_csoff();

	// Wait idle process
	for(i = SD_WAIT_INIT_COUNT; i > 0; --i) {
		sd_L0_cson();
		r = sd_L1_cmd_r1(55, 0);		// APP_CMD
		r = sd_L1_cmd_r1(41, 0);		// SD_SEND_OP_COND
		sd_L0_send(0xff);				// stable (go to standby)
		sd_L0_csoff();
		if (!(r & SD_R1_IDLE_STATE)) {
			break;
		}
		delay_1mTimes(SD_WAIT_INIT_INTERVAL);
	}

	// CRC enable
#ifdef SD_OPT_CRC
	if (r == 0x00) {
		sd_L0_cson();
		r = sd_L1_cmd_r1(59, 1);		// CRC_ON_OFF
		sd_L0_send(0xff);				// stable (go to standby)
		sd_L0_csoff();
	}
#endif

	i2c_lcd_position(0,0);	i2c_lcd_prCString("L1open="); i2c_lcd_prHexByte(r);
	return(r);
}


//
//	Close SDcard device
//
void sd_L1_close(void) {

	// dettach User module
    sd_L0_term();

	i2c_lcd_position(0,0);	i2c_lcd_prCString("L1close ");
	return;
}


//
//	Read 1 block
//
#ifdef SD_OPT_BUF256
unsigned char sd_L1_read(long sect, char *buf1, char *buf2){	// Response code, 0:normal
#else
unsigned char sd_L1_read(long sect, char *buf){	// Response code, 0:normal
#endif // SD_OPT_BUF256
	unsigned char	r;
	char			j;
	unsigned short	c;

	// Try loop
	for(j = SD_RETRY_READ; j > 0; --j) {

		// Transaction start
		sd_L0_cson();
	
		// Dummy loop
		for(;;) {
			char	*p;
			int		i;
		
			// Read cmd
			r = sd_L1_cmd_r1(17, sect << 9);	// READ_SINGLE_BLOCK
			if (r != 0x00) {
				break;
			}
		
			// Wait start data block talken
			r = sd_L0_recv(SD_WAIT_READ);
			if (r != 0xfe) {
				if (r == 0) {
					r = 0xff;
				}
				break;
			}

			// Recieve, data & CRC
#ifdef SD_OPT_BUF256
			p = buf1;
			for(i = SD_BUFSZ+2; i > 0; --i) {
				r = sd_L0_recv(1);		// no retry
				
				if (i == (SD_BUFSZ/2+2)) {
					p = buf2;
				}
				if (i > 2) {
					*p++ = r;
				} else if (i == 2) {
					c = (unsigned short)r << 8;
				} else {
					c |= r;
				}
			}
#else
			p = buf;
			for(i = SD_BUFSZ+2; i > 0; --i) {
				r = sd_L0_recv(1);		// no retry
				if (i > 2) {
					*p++ = r;
				} else if (i == 2) {
					c = (unsigned short)r << 8;
				} else {
					c |= r;
				}
			}
#endif // SD_OPT_BUF256
			
			r = 0x00;
			break;
		}
		
		// Transaction end
		sd_L0_send(0xff);				// stable (go to standby)
		sd_L0_csoff();

		// Normal
		if (r == 0x00) {
#ifdef SD_OPT_CRC
#ifdef SD_OPT_BUF256
			if (sd_L1_crc16(buf1, buf2, SD_BUFSZ) != c) {
				r = SD_R1_CRC_ERR;
				continue;
			}
#else
			if (sd_L1_crc16(buf, SD_BUFSZ) != c) {
				r = SD_R1_CRC_ERR;
				continue;
			}
#endif // SD_OPT_BUF256
#endif // SD_OPT_CRC

			break;
		}
	}

	i2c_lcd_position(0,0);	i2c_lcd_prCString("L1read="); i2c_lcd_prHexByte(r);
	return(r);
}


//
//	Write 1 block
//
#ifdef SD_OPT_WRITE
#ifdef SD_OPT_BUF256
unsigned char sd_L1_write(long sect, char *buf1, char *buf2){	// Response code, 0:normal
#else
unsigned char sd_L1_write(long sect, char *buf){	// Response code, 0:normal
#endif // SD_OPT_BUF256
	unsigned char	r;
	char			j;
	unsigned short	c;

	// Calc CRC
#ifdef SD_OPT_CRC

#ifdef SD_OPT_BUF256
	c = sd_L1_crc16(buf1, buf2, SD_BUFSZ);
#else
	c = sd_L1_crc16(buf, SD_BUFSZ);
#endif // SD_OPT_BUF256

#else
	c = 0;
#endif // SD_OPT_CRC

	// Try loop
	for(j = SD_RETRY_WRITE; j > 0; --j) {

		// Transaction start
		sd_L0_cson();

		// Dummy loop
		for(;;) {
			char	*p;
			int		i;
		
			// Write cmd
			r = sd_L1_cmd_r1(24, sect << 9);	// WRITE_BLOCK
			if(r != 0x00) {
				break;
			}
	
			// Start data block talken
			sd_L0_send(0xfe);
		
			// Send data & CRC
#ifdef SD_OPT_BUF256
			p = buf1;
			for(i = SD_BUFSZ+2; i > 0; --i) {
				unsigned char d;
			
				if (i == (SD_BUFSZ/2+2)) {
					p = buf2;
				}

				d = 0;
				if (i > 2) {
					d = *p++;
				} else if (i == 2) {
					d = c >> 8;
				} else {
					d = c & 0xff;
				}
				sd_L0_send(d);
			}
#else
			p = buf;
			for(i = SD_BUFSZ+2; i > 0; --i) {
				unsigned char d;
			
				d = 0;
				if (i > 2) {
					d = *p++;
				} else if (i == 2) {
					d = c >> 8;
				} else {
					d = c & 0xff;
				}
				sd_L0_send(d);
			}
#endif // SD_OPT_BUF256

			// wait response
			r = sd_L0_recv(SD_WAIT_RESPONSE);
			if ((r & 0x1f) != 0x05) {
				break;
			}

			// Busy wait
			r = sd_L0_recv(-SD_WAIT_WRITE);	// wait 0x00 to 0xff
			if (r != 0x00) {
				r = 0;
				break;
			}
		}
		
		// Transaction end
		sd_L0_send(0xff);				// stable (go to standby)
		sd_L0_csoff();

		// Normal
		if (r == 0x00) {
			break;
		}
	}

	i2c_lcd_position(0,0);	i2c_lcd_prCString("L1close="); i2c_lcd_prHexByte(r);
	return(r);
}
#endif // SD_OPT_WRITE


//
//	Get card information
//
#ifdef SD_OPT_GETINFO
unsigned char sd_L1_getinfo(void *info, char type){
	unsigned char	r;
	char			j;
	unsigned short	c;
	char			buf[16];

	// Try loop
	for(j = SD_RETRY_INFO; j > 0; --j) {

		// Transaction start
		sd_L0_cson();
	
		// Dummy loop
		for(;;) {
			char	*p;
			int		i;
		
			// Read cmd
			r = sd_L1_cmd_r1(9, 0);		// SEND_CSD
			if (r != 0x00) {
				break;
			}
		
			// Wait start data block talken
			r = sd_L0_recv(SD_WAIT_INFO);
			if (r != 0xfe) {
				if (r == 0) {
					r = 0xff;
				}
				break;
			}

			// Recieve, data & CRC
			p = buf;
			for(i = sizeof(buf)+2; i > 0; --i) {
				r = sd_L0_recv(1);		// no retry
				if (i > 2) {
					*p++ = r;
				} else if (i == 2) {
					c = (unsigned short)r << 8;
				} else {
					c |= r;
				}
			}
			
			r = 0x00;
			break;
		}
		
		// Transaction end
		sd_L0_send(0xff);				// stable (go to standby)
		sd_L0_csoff();

		// Error
		if (r != 0x00) {
			continue;
		}

#ifdef SD_OPT_CRC
#ifdef SD_OPT_BUF256
		if (sd_L1_crc16(buf, 0, sizeof(buf)) != c) {
			r = SD_R1_CRC_ERR;
			continue;
		}
#else
		if (sd_L1_crc16(buf, sizeof(buf)) != c) {
			r = SD_R1_CRC_ERR;
			continue;
		}
#endif // SD_OPT_BUF256
#endif // SD_OPT_CRC

		// Set info
		if (type == SD_L1_INFOTYPE_MEDIASIZE) {
			short	c_size;
			int		mult;

			c_size = ((short)((unsigned char)buf[6] & 0x3) << 10)
			       | ((short)((unsigned char)buf[7]) << 2)
			       | ((unsigned char)buf[8] >> 6);
			mult = ((((unsigned char)buf[9] & 0x3) << 1) | ((unsigned char)buf[10] >> 7))
			     + 2
			     + ((unsigned char)buf[5] & 0xf);
			*((long *)info) = (long)(c_size + 1) << mult;
		}
		// CSD structure version 1.0 <- SD1.01-1.10,2.00/Standard Capacity

		break;
	}

	i2c_lcd_position(0,0);	i2c_lcd_prCString("L1info="); i2c_lcd_prHexByte(r);
	return(r);
}

#endif // SD_OPT_INFO
