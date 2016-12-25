//
//  sd.h
//
//  SDcard control module mini / Like a "unix system call" APIs
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
//	Description
//		Minimum read/write function for Secure Digital memory card.
//		Include FAT16 file system, but very slow.
//		The API is like unix system call.
//
//	Spec
//		Physical I/O	: SPI(3wire,SCLK=100kHz~400kHz) and CS pin
//		Card type		: Standard size SDcard, miniSDcard, microSDcard, Under 2GB, not MMC
//		Protocol		: SDcard V2.00, SPI mode, CRC check
//		Volume format	: Non partition(floppy disk) and MBR Partition(only #0 entry)
//		File system		: FAT12 and FAT16, not FAT32
//						  Root directory and short name(8.3) only
//
//	Setting
//		PSoC
//			Device	: CY8C29466
//			SPI		: SPIM user module(Name=SD_SPI,Intrrupt=TXRegEmpty,InvertMISO=normal,Clock=SCLK*2)
//			CS		: LED user module(Name=SD_CS,Drive=Active low)
//			IDE		: PSoC Designer 5 Service Pack 6(Build 1127), ImageCraft C compiler 7.02.004(free version)
//			Code Size	: 8.6k ~ 13.0kbyte
//			Data Size	: 668byte
//		PIC18
//			Device	: PIC18F2550
//			SPI		: MSSP SCK(RB1),SDI(RB0),SDO(RC7)
//			CS		: GPIO CS(RC6)
//			IDE		: MPLAB IDE v8.36, C18 C Compiler v3.33(academic version)
//			Code Size	: R/O:11.2k R/W:15.4k debug:17.4k Full:18.7kbyte
//			Data Size	: 668byte
//		AVR
//			Device	: ATmega328P
//			SPI		: SPI master mode SCK(PB5),MISO(PB4),MOSI(PB3)
//			CS		: SPI SS(PB2)
//			IDE		: AVR Studio 4.18.700 + WinAVR-20100110(GCC 4.3.3)
//			Code Size	: R/O:4.6k R/W:6.5k Full:6.8kbyte
//			Data Size	: 688byte
//
//	Reference
//		SDcard	: SD Specifications, Part 1, Physical Layer, Simplified Specification, Version 2.00, September 25, 2006
//					<www.sdcard.org/developers/tech/sdcard/pls/Simplified_Physical_Layer_Spec.pdf>
//		FAT		: Microsoft Extensible Firmware Initiative, FAT32 File System Specification, FAT: General Overview of On-Disk Format Version 1.03, December 6, 2000
//					<www.microsoft.com/whdc/system/platform/firmware/fatgen.mspx>
//		MBR		: http://ja.wikipedia.org/wiki/%E3%83%9E%E3%82%B9%E3%82%BF%E3%83%BC%E3%83%96%E3%83%BC%E3%83%88%E3%83%AC%E3%82%B3%E3%83%BC%E3%83%89
//
//
//	Update history
//	---------- ----- -------------------------------------------
//	2009.12.18 v0.0  First cording
//	2010.01.11 v0.1  Support PIC18F2550
//	2010.01.16 v0.1a Split source
//	2010.02.19 v0.1b Bug fix, do not use printf()
//	2010.04.02 v0.1c Bug fix, open file not found
//	2010.07.07 v0.2  Support ATmega328P
//

#ifndef SD_H
#define SD_H

#include "SD/sd_config.h"
#include "SD/sd_common.h"
#include "SD/sd_L3.h"


#define SD_SET_DATE(year,month,day)			(sd_L3_NowDate = (((short)(year-1980)<<9) | ((short)month<<5) | ((short)day)))
#define SD_SET_TIME(hour,min,sec)			(sd_L3_NowTime = (((short)(hour)<<11) | ((short)min<<5) | ((short)sec>>1)))

#define SD_DIR								struct sd_L3_fd
#define sd_dirent							sd_L3_dirent
extern struct sd_L3_fd						*sd_L3_tempfp;
extern struct sd_L3_dirent 					sd_L3_tempdirent;
#define sd_mount(partno)					(sd_L3_mount(partno) ? 0 : -1)
#define sd_open(filename,oflags,attr)		((sd_L3_tempfp = sd_L3_open(&sd_L3_vol[0], (filename), (oflags), (attr))) != 0 ? sd_L3_tempfp->no : -1)
#define sd_read(fd,buf,bufsize)				sd_L3_read(&sd_L3_fdtbl[(fd)], (buf), (bufsize))
#define sd_write(fd,buf,bufsize)			sd_L3_write(&sd_L3_fdtbl[(fd)], (buf), (bufsize))
#define sd_lseek(fd,offset,whence)			sd_L3_lseek(&sd_L3_fdtbl[(fd)], (offset), (whence))
#define sd_sync(fd)							sd_L3_sync(&sd_L3_fdtbl[(fd)])
#define sd_close(fd)						sd_L3_close(&sd_L3_fdtbl[(fd)])
#define sd_unmount()						sd_L3_unmount(&sd_L3_vol[0])
#define sd_unlink(filename)					sd_L3_unlink(&sd_L3_vol[0], (filename))
#define sd_opendir(dirname)					sd_L3_open(&sd_L3_vol[0], (dirname), SD_O_RDONLY, 0)
#define sd_readdir(dirp)					((sd_L3_read((dirp), (char *)&(sd_L3_tempdirent), sizeof(struct sd_L3_dirent)) > 0) && (sd_L3_tempdirent.Name[0] != '\0') ? &sd_L3_tempdirent : 0)
#define sd_readdir_r(dirp, entry, result)	((sd_L3_read((dirp), (char *)(entry), sizeof(struct sd_L3_dirent)) > 0) && ((entry)->Name[0] != '\0') ? (*(result)=(entry), 0) : (*(result)=0, -1))
#define sd_telldir(dirp)					sd_L3_lseek((dirp), 0, SD_SEEK_CUR)
#define sd_seekdir(dirp, loc)				sd_L3_lseek((dirp), (loc), SD_SEEK_SET)
#define sd_rewinddir(dirp)					sd_L3_lseek((dirp), 0, SD_SEEK_SET)
#define sd_closedir(dirp)					sd_L3_close(dirp)
#define sd_dirfd(dirp)						(dirp->no)


//-------------------------- prototype --------------------------------------
// int		sd_mount(char partno);
// int		sd_open(char *filename, int oflags, int attr);
// int		sd_read(int fd, char *buf, int bufsize);
// int		sd_write(int fd, char *buf, int bufsize);
// int		sd_lseek(int fd, long offset, int whence);
// int		sd_sync(int fd);
// int		sd_close(int fd);
// int		sd_unmount(void);
// int		sd_unlink(char *filename);
// SD_DIR	*sd_opendir(char *dirname);
// struct sd_dirent *sd_readdir(SD_DIR *dirp);
// int		sd_readdir_r(SD_DIR *dirp, struct sd_dirent *entry, struct sd_dirent **result));
// long		sd_telldir(SD_DIR *dirp);
// void		sd_seekdir(SD_DIR *dirp, long loc);
// void		sd_rewinddir(SD_DIR *dirp);
// int		sd_closedir(SD_DIR *dirp);
// int		sd_dirfd(SD_DIR *dirp);

// oflags
//#define SD_O_RDONLY		0x00		// open for reading only
//#define SD_O_WRONLY		0x01		// open for writing only
//#define SD_O_RDWR			0x02		// open for reading and writing
//#define SD_O_NONBLOCK		0xXX		// do not block on open or for data to become available
//#define SD_O_APPEND		0x04		// append on each write
//#define SD_O_CREAT		0x08		// create file if it does not exist
//#define SD_O_TRUNC		0x10		// truncate size to 0
//#define SD_O_EXCL			0x20		// error if O_CREAT and the file exists
//#define SD_O_SHLOCK		0xXX		// atomically obtain a shared lock
//#define SD_O_EXLOCK		0xXX		// atomically obtain an exclusive lock
//#define SD_O_NOFOLLOW		0xXX		// do not follow symlinks
//#define SD_O_SYMLINK		0xXX		// allow open of symlinks
//#define SD_O_EVTONLY		0xXX		// descriptor requested for event notifications only

// attr
//#define SD_ATTR_READ_ONLY	0x01
//#define SD_ATTR_HIDDEN	0x02
//#define SD_ATTR_SYSTEM	0x04
//#define SD_ATTR_VOLUME_ID	0x08
//#define SD_ATTR_DIRECTORY	0x10
//#define SD_ATTR_ARCHIVE	0x20
//#define SD_ATTR_LONG_NAME	ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID

// whence
//#define SD_SEEK_SET		0			// the offset is set to offset bytes.
//#define SD_SEEK_CUR		1			// the offset is set to its current location plus offset bytes.
//#define SD_SEEK_END		2			// the offset is set to the size of the file plus offset bytes.

//
//  delay.h
//
//  Software delay
//  Copyright (C) 2009 Toyohiko Togashi tog001@nifty.com
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
//	Description
//		Constant wait.
//		Independence from CPU clock.  
//
//	Uasge
//		Edit source file "delay.h" in configure
//
//  Device: CY8C29466/27443  IDE: PSoC Designer 5 Service Pack 6(Build 1127)
//  Device: MSP430F2013      IDE: IAR Embedded Workbench for MSP430 V4.11B
//  Device: PIC18F14K50      IDE: MPLAB IDE 8.36, C Compiler for PIC18 MCUs v3.33
//  Device: ATmega328P       IDE: AVR Studio 4.18.700, WinAVR-20100110(GCC 4.3.3)
//
//    Update history
//    ---------- ----- -------------------------------------------
//    2009.12.22 v0.0  First cording
//    2010.07.07 v0.2  Support ATmega328P
//

#include "SD/delay.h"

void delay_50uTimes(unsigned char bTimes){
    for(; bTimes > 0; --bTimes) {
        delay_50u();
    }
    return;
}

void delay_1mTimes(unsigned char bTimes){
    for(; bTimes > 0; --bTimes) {
        delay_50uTimes(20);
    }
    return;
}


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

#include "SD/sd_config.h"					// Private
#include "SD/sd_L1.h"						// Private
#include "SD/delay.h"						// Private

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
#include "SD/PSoCAPI.h"					// PSoC API definitions for all User Modules
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
#include "SD/p18cxxx.h"					// C18 library
#include "SD/spi.h"						// C18 library

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
//
//  sd_L2.c
//
//  SDcard control module mini - Partition manage and data caching
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

#include "SD/sd_config.h"					// Private
#include "SD/sd_common.h"					// Private
#include "SD/sd_L1.h"						// Private
#include "SD/sd_L2.h"						// Private

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
//	C o m m o n   a r e a
//
//*********************************************************************

// Data buffer (Share all layer)
#ifdef __18CXX
#pragma udata sd_buf					// see linker script
#endif //__18CXX

#ifdef SD_OPT_BUF256
char	sd_buf1[SD_BUFSZ/2];			// ImageCraft compiler limit is 256
char	sd_buf2[SD_BUFSZ/2];
#else
char	sd_buf[SD_BUFSZ];
#endif //SD_OPT_BUF256

#ifdef __18CXX
#pragma udata
#endif //__18CXX


//*********************************************************************
//
// Layer 2	Partition manage and data caching
//
//*********************************************************************

//
//	Constant and macro
//
#define SD_L2_PASSCODE	0x55
#ifdef SD_OPT_BUF256
#define SD_L1_READ(a)	sd_L1_read(a, sd_buf1, sd_buf2)
#define SD_L1_WRITE(a)	sd_L1_write(a, sd_buf1, sd_buf2)
#else
#define SD_L1_READ(a)	sd_L1_read(a, sd_buf)
#define SD_L1_WRITE(a)	sd_L1_write(a, sd_buf)
#endif //SD_OPT_BUF256

//
//	Data area
//
struct sd_L2_part	sd_L2_part[SD_L2_MAXPD] = SD_L2_INIPD;

//
//	Open
//
struct sd_L2_part *sd_L2_open(char partno) {	// discriptor  0:error(detail is sd_errno)
#define SD_L2_TBLSZ 4
	struct parttbl {
		unsigned char	flag;
		unsigned char	start_chs[3];
		unsigned char	type;
		unsigned char	end_chs[3];
		unsigned char	start_lba[4];
		unsigned char	sect[4];
	};
	struct mbr {
		unsigned char	bootcode[446];
		struct parttbl	parttbl[SD_L2_TBLSZ];
		unsigned char	bootsig[2];
	};
	struct sd_L2_part	*pp;
	signed char			rc;

	rc = -1;
	
	// Dummy loop
	for(;;){
		struct parttbl	*pt;
		struct mbr		*m;
		unsigned char	r;
		char			pd;

#ifdef SD_OPT_ARGCHECK
		if ((partno < 0) || (partno >= SD_L2_TBLSZ)) {
			sd_errno = SD_EINVAL;
			break;
		}
#endif //SD_OPT_ARGCHECK

		// Allocate table
		pp = &sd_L2_part[0];
		for(pd = 0; pd < SD_L2_MAXPD; pd++) {
			if (pp->no < 0) {
				break;
			}
			pp++;
		}
		if (pd >= SD_L2_MAXPD) {
			sd_errno = SD_EMFILE;
			break;
		}
		
		// Device open
		if ((r = sd_L1_open()) != 0) {
			sd_errno = SD_EIO;
			if (r == 0xff) {			// Not insert
				sd_errno = SD_EPERM;
			}
			break;
		}

		// Read boot sector
		if (SD_L1_READ(0) != 0) {		// Data->L1_buf[]
			sd_errno = SD_EIO;
			break;
		}

		// signature
#ifdef SD_OPT_BUF256
		m = (struct mbr *)(sd_buf2 - 256);
#else
		m = (struct mbr *)sd_buf;
#endif //SD_OPT_BUF256
		if ((m->bootsig[0] != 0x55) || (m->bootsig[1] != 0xAA)) {
			sd_errno = SD_ENODEV;
			break;
		}

		// No partition
		if (m->bootcode[0] == 0xeb) {
			pp->parttype  = SD_L2_NOPARTID;
			pp->startsect = 0;
			pp->totalsect = 4194304L;	// 2GB

		// Partition
		} else {
			// No entry?
			pt = &(m->parttbl[partno]);
			if (pt->type == 0) {
				sd_errno = SD_ENOENT;
				break;
			}
			pp->parttype  = pt->type;
			pp->startsect = sd_boconv4(pt->start_lba);
			pp->totalsect = sd_boconv4(pt->sect);
		}

		pp->bufsect   = -1;
		pp->delayw    = 0;
#ifdef SD_OPT_ARGCHECK
		pp->sig       = SD_L2_PASSCODE;
#endif //SD_OPT_ARGCHECK
		
		printf("L2 pp->parttype=0x%02x\n", pp->parttype);
		printf("L2 pp->startsect=%ld\n", pp->startsect);
		printf("L2 pp->totalsect=%ld\n", pp->totalsect);

		// Commit
		pp->no = pd;
		rc = 0;
		break;
	}
	
	if (rc != 0) {
		pp = 0;
	}

	printf("L2open(partno=%d)=0x%08lx\n", partno, (long)pp);
	i2c_lcd_position(0,0);	i2c_lcd_prCString("L2open="); i2c_lcd_prHexInt((int)pp);
	return(pp);
}

//
//	Close
//
signed char sd_L2_close(struct sd_L2_part *pp) {	// 0:normal -1:error
	signed char rc;
	
	rc = -1;
	
	// dummy loop
	for(;;) {
		
#ifdef SD_OPT_ARGCHECK
		if ((pp == 0) || (pp->no < 0) || (pp->no >= SD_L2_MAXPD) || (pp->sig != SD_L2_PASSCODE)) {
			sd_errno = SD_EBADF;
			break;
		}
#endif //SD_OPT_ARGCHECK

		// Delayed write
		if (pp->delayw) {
			if (SD_L1_WRITE(pp->bufsect + pp->startsect) != 0) {
				sd_errno = SD_EIO;
#ifndef SD_OPT_FORCECLOSE
				break;
#endif //SD_OPT_FORCECLOSE
			}
		}
		
		// Device close
		sd_L1_close();
		pp->no = -1;
		rc = 0;
		break;
	}
	
	printf("L2close(pp=0x%08lx)=%d\n", (long)pp, rc);
	i2c_lcd_position(0,0);i2c_lcd_prCString("L2close=");i2c_lcd_prHexInt((int)pp);i2c_lcd_prHexByte(rc);
	return(rc);
}

//
//	Read
//
signed char sd_L2_read(struct sd_L2_part *pp, long sect) {	// 0:normal -1:error(detail is sd_errno)
	signed char rc;
	
	rc = -1;
	
	// dummy loop
	for(;;){
		
#ifdef SD_OPT_ARGCHECK
		if ((pp->no < 0) || (pp->no >= SD_L2_MAXPD) || (pp->sig != SD_L2_PASSCODE)) {
			sd_errno = SD_EBADF;
			break;
		}
#endif //SD_OPT_ARGCHECK
		
#ifdef SD_OPT_PARTLIMCHK
		if ((sect < 0) || (sect >= pp->totalsect)) {
			sd_errno = SD_EFBIG;
			break;
		}
#endif //SD_OPT_PARTLIMCHK

		// Read Cache
		if (pp->bufsect == sect) {
			rc = 0;
			break;
		}
		
		// Delayed write
		if (pp->delayw) {
			if (SD_L1_WRITE(pp->bufsect + pp->startsect) != 0) {
				sd_errno = SD_EIO;
				break;
			}
			pp->delayw = 0;
		}

		// Read sector
		if (SD_L1_READ(sect + pp->startsect) != 0) {
			sd_errno = SD_EIO;
			break;
		}

		pp->bufsect = sect;
		rc = 0;
		break;
	}

	printf("L2read(pp=0x%08lx,sect=%ld)=%d\n", (long)pp, sect, rc);
	i2c_lcd_position(0,0);i2c_lcd_prCString("L2read=");i2c_lcd_prHexInt((int)pp);i2c_lcd_prHexInt(sect);i2c_lcd_prHexByte(rc);
	return(rc);
}

//
//	Write 512bytes
//
#ifdef SD_OPT_WRITE
signed char sd_L2_write(struct sd_L2_part *pp, long sect) {	// 0:normal -1:error(detail is sd_errno)
	signed char rc;
	
	rc = -1;
	
	// dummy loop
	for(;;){
		
#ifdef SD_OPT_ARGCHECK
		if ((pp == 0) || (pp->no < 0) || (pp->no >= SD_L2_MAXPD) || (pp->sig != SD_L2_PASSCODE)) {
			sd_errno = SD_EBADF;
			break;
		}
#endif //SD_OPT_ARGCHECK

#ifdef SD_OPT_PARTLIMCHK
		if ((sect < 0) || (sect >= pp->totalsect)) {
			sd_errno = SD_EFBIG;
			break;
		}
#endif //SD_OPT_PARTLIMCHK
		
		// Write Cache
		if (pp->bufsect == sect) {
			pp->delayw = 1;
			rc = 0;
			break;
		}
		
		// Delayed write
		if (pp->delayw) {
			if (SD_L1_WRITE(pp->bufsect + pp->startsect) != 0) {
				sd_errno = SD_EIO;
				break;
			}
			pp->delayw = 0;
		}

		// Marking
		pp->bufsect = sect;
		pp->delayw = 1;

		rc = 0;
		break;
	}

	printf("L2write(pp=0x%08lx,sect=%ld)=%d\n", (long)pp, sect, rc);
	i2c_lcd_position(0,0);i2c_lcd_prCString("L2write=");i2c_lcd_prHexInt((int)pp);i2c_lcd_prHexInt(sect);i2c_lcd_prHexByte(rc);
	return(rc);
}
#endif // SD_OPT_WRITE

//
//  sd_L3.c
//
//  SDcard control module mini - FAT file system
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

#include <ctype.h>
#include "SD/sd_config.h"					// Private
#include "SD/sd_common.h"					// Private
#include "SD/sd_L2.h"						// Private
#include "SD/sd_L3.h"						// Private
#include "sd.h"							// Private

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
// Layer 3	FAT file system		Support 12/16bit
//
//*********************************************************************

// External date/time interface
unsigned char	sd_L3_NowTimeTenth = 0;
unsigned short	sd_L3_NowTime = 0b0000000000000000;		// 00:00:00
//                                hhhhhmmmmmmsssss
unsigned short	sd_L3_NowDate = 0b0011110000100001;		// 2010.1.1
//                                yyyyyyymmmmddddd

//---------------------------------------------------------------------
//	Volume information table
//---------------------------------------------------------------------
// Enable partition ID
#define SD_L3_PARTID_1			0x01		// FAT12
#define SD_L3_PARTID_2			0x04		// FAT16 <32KB
#define SD_L3_PARTID_3			0x06		// FAT16 >32KB
#define SD_L3_PARTID_4			0x0b		// FAT32
#define SD_L3_PARTID_5			0x0e		// FAT16 LBA
#define SD_L3_PARTID_6		SD_L2_NOPARTID	// No partition disk
// Fixed value
#define SD_L3_FATEntSz			2			// 2^1
#define SD_L3_FATEntSzPow		1			// 2^SD_L3_FATEntSzPow == SD_L3_FATEntSz
#define SD_L3_FATEntSzMask		0x01		//
#define SD_L3_RootEntSz			32			// 2^5
#define SD_L3_RootEntSzPow		5			// 2^SD_L3_RootEntSzPow == SD_L3_RootEntSz
#define SD_L3_RootEntSzMask		0x1f		//
#define SD_L3_BytePerSec		512			// 2^9
#define SD_L3_BytePerSecPow		9			// 2^SD_L3_BytePerSecPow == SD_L3_BytePerSec
#define SD_L3_BytePerSecMask	0x01ff		//
#define SD_L3_FAT12				12
#define SD_L3_FAT16				16
#define SD_L3_FAT32				32

struct sd_L3_vol	sd_L3_vol[SD_L3_MAXVD] = SD_L3_INIVD;

//---------------------------------------------------------------------
//	File access control table
//---------------------------------------------------------------------
struct sd_L3_fd		sd_L3_fdtbl[SD_L3_MAXFD] = SD_L3_INIFD;
struct sd_L3_fd		*sd_L3_tempfp;			// define macro temp

//---------------------------------------------------------------------
//	Cluster manage (FAT Region)
//---------------------------------------------------------------------
#define SD_L3_FREECL		0x0000		// Free cluster
#define SD_L3_SPECIALCL		0xfff0		// Functionary
#define SD_L3_BADCL			0xfff7		// Bad cluster
#define SD_L3_EOC			0xffff		// End of cluster
#define SD_L3_READONLYCL	0xfffe		// Pseudo: No update option
#define SD_L3_ROOTDIRCL		0xfffd		// Pseudo: Directory region access

//
// read and update FAT entry
//
unsigned short sd_L3_fatentryio(struct sd_L3_vol *vp, unsigned short cl, unsigned short updcl) {	// read data(SD_L3_BADCL==error)
	unsigned short	d;					// readed data
	
	printf("L3fatentryio(vp=0x%08lx,cl=0x%04x,updcl=0x%04x) start\n", (long)vp, cl, updcl);
	d = SD_L3_BADCL;

	for(;;){
		int		r;						// relative sector location
		int		i;						// buffer index
		long	s;						// absolute sector location
		char	*p;						// buffer pointer

		if (cl >= SD_L3_SPECIALCL) {
			d = cl;
			break;
		}

		if (vp->FATType == SD_L3_FAT12) {
			int	w;

			w = cl + (cl >> 1);			// *1.5
//			r = w / SD_L3_BytePerSec;
//			i = w % SD_L3_BytePerSec;
			r = w >> SD_L3_BytePerSecPow;		// self optimize
			i = w & SD_L3_BytePerSecMask;		// self optimize
			if (i == (SD_L3_BytePerSec - 1)) {	// sector boundery
				i = 0;
				r++;
			}
		} else {						// FAT16
//			r = (cl * SD_L3_FATEntSz) / SD_L3_BytePerSec;
//			i = (cl * SD_L3_FATEntSz) % SD_L3_BytePerSec;
			r = cl >> (SD_L3_BytePerSecPow - SD_L3_FATEntSzPow);		// self optimize
			i = (cl << SD_L3_FATEntSzPow) & SD_L3_BytePerSecMask;		// self optimize
		}

		s = vp->FirstFATSector + r;
		if (sd_L2_read(vp->pp, s) != 0) {
			break;
		}
#ifdef SD_OPT_BUF256
		if (i & 0x100) {
			p = &sd_buf2[i & 0xff];
		} else {
			p = &sd_buf1[i];
		}
#else
		p = &sd_buf[i];
#endif
		d = sd_boconv2(p);
		if (vp->FATType == SD_L3_FAT12) {
			if (cl & 1) {
				d >>= 4;
			} else {
				d &= 0x0fff;
			}
			if (d >= (SD_L3_BADCL & 0x0fff)) {
				d |= 0xf000;
			}
		}

		// Update write
#ifdef SD_OPT_WRITE
		if (updcl != SD_L3_READONLYCL) {
			char n;

			if (vp->FATType == SD_L3_FAT12) {
				if (cl & 1) {
					updcl = (updcl << 4) | (*p & 0x0f);
				} else {
					updcl = (updcl & 0x0fff) | ((*(p + 1) & 0xf0) << 8);
				}
			}
			sd_bounconv2(p, updcl);

			// all copy
			for(n = vp->NumFATs; n > 0; --n) {
				if (sd_L2_write(vp->pp, s) != 0) {
					d = SD_L3_BADCL;
					break;
				}
				s += vp->FATSz;
			}
		}
#endif //SD_OPT_WRITE

		break;
	}
	
	printf("L3fatentryio()=0x%04x\n", d);
	return(d);
}

//
// allocate new FAT entry
//
#ifdef SD_OPT_WRITE
unsigned short sd_L3_alloccl(struct sd_L3_vol *vp, unsigned short firstcl) {	// new cluster number(SD_L3_BADCL==error)
	unsigned short cl;

	printf("L3alloccl(vp=0x%08lx,firstcl=0x%04x) start\n", (long)vp, firstcl);

	// dummy loop
	for(;;) {
		char found;

		// Find free cluster (sequential search)
		found = 0;
		for(cl = 0; cl < vp->FATEntCnt; cl++) {
			if (sd_L3_fatentryio(vp, cl, SD_L3_READONLYCL) == SD_L3_FREECL) {
				found = 1;
				break;
			}
		}
		if (!found) {
			sd_errno = SD_ENOSPC;
			cl = SD_L3_BADCL;
			break;
		}

		// Allocated mark
		if (sd_L3_fatentryio(vp, cl, SD_L3_EOC) == SD_L3_BADCL) {
			cl = SD_L3_BADCL;
			break;
		}

		// Link chain
		if (firstcl > 1) {	// not root
			unsigned short lastcl;
			unsigned short c;

			for(c = firstcl; (c >= 2) && (c < SD_L3_SPECIALCL); c = sd_L3_fatentryio(vp, c, SD_L3_READONLYCL)) {
				lastcl = c;
			}

			if (sd_L3_fatentryio(vp, lastcl, cl) == SD_L3_BADCL) {
				cl = SD_L3_BADCL;
				break;
			}
		}

		break;
	}

	printf("L3alloccl()=0x%04x\n", cl);
	return(cl);
}
#endif

//
// free FAT entry
//
#ifdef SD_OPT_UNLINK
signed char sd_L3_freecl(struct sd_L3_vol *vp, unsigned short firstcl) {	// 0:normal -1:error
	unsigned short	cl;
	signed char		rc;

	printf("L3freecl(vp=0x%08lx,freecl=0x%04x) start\n", (long)vp, firstcl);
	
	// dummy loop
	for(cl = firstcl; cl < SD_L3_SPECIALCL; ) {
		unsigned short	n;

		if ((n = sd_L3_fatentryio(vp, cl, SD_L3_READONLYCL)) == SD_L3_BADCL) {
			break;
		}

		if (sd_L3_fatentryio(vp, cl, SD_L3_FREECL) == SD_L3_BADCL) {
			break;
		}

		cl = n;
	}
	if (cl != SD_L3_BADCL) {
		rc = 0;
	} else {
		rc = -1;
	}
	
	printf("L3freecl()=%d\n", rc);
	return(rc);
}
#endif

//---------------------------------------------------------------------
//	Directory manage (Root Directory Region)
//---------------------------------------------------------------------
struct sd_L3_dirent	sd_L3_tempdirent;		// use reduction stack

//
// formating file name
//
void sd_L3_convdirname(char *out, char *in) {
	char	i;
	char	j;
	static const char ill[] = {
		0x22, 0x2A, 0x2B, 0x2C, 0x2E, 0x2F, 0x3A, 0x3B,
		0x3C, 0x3D, 0x3E, 0x3F, 0x5B, 0x5C, 0x5D, 0x7C,
		0x00 };
	
	for(i = SD_L3_DIRENT_NAMESZ; i > 0; --i) {
		*out = ' ';
		if (*in > ' ') {
			if (*in == '.') {
				if (i > 3) {
					out++;
					continue;
				} else {
					in++;
				}
			}
			*out = toupper(*in++);

			for(j = 0; ill[j] != 0x00; j++) {
				if (*out == ill[j]) {
					*out = '@';
					break;
				}
			}
		}
		out++;
	}
	return;
}


//---------------------------------------------------------------------
//	Data area manage (Data Region)
//---------------------------------------------------------------------
//
// convert seek address to sector number
//
long sd_L3_seek2sect(struct sd_L3_vol *vp, long seek, unsigned short firstcl) {		// sector number  -1:error
	long	s;							// Answer

	printf("L3seek2sect(vp=0x%08lx,seek=%ld,firstcl=0x%04x) start\n", (long)vp, seek, firstcl);
	s = -1;

	if (firstcl == SD_L3_ROOTDIRCL) {
//		s = vp->RootDirSector + (seek / SD_L3_BytePerSec);
		s = vp->RootDirSector + (seek >> SD_L3_BytePerSecPow);
		
	} else {
		unsigned short	n;				// Target cluster position
		unsigned short	cl;				// Current cluster number
		unsigned short	i;

		// cluster position : n
//		n = seek / vp->BytePerClus;
		n = seek >> vp->BytePerClusPow;
		
		// follow cluster chaine : cl
		if ((firstcl == vp->cl_1st) && (n == vp->cl_n)) {	// cache hit
			cl = vp->cl_cl;
			
		} else {
			i = n;
			for(cl = firstcl; (cl >= 2) && (cl < SD_L3_SPECIALCL); cl = sd_L3_fatentryio(vp, cl, SD_L3_READONLYCL)) {
				if (i <= 0) {
					break;
				}
				--i;
			}
		}
		
		// Calculate
		if ((cl >= 2) && (cl < SD_L3_SPECIALCL)) {
//			s = vp->FirstDataSector
//			  + ((cl-2) * vp->SecPerClus)
//		  	  + (seek / SD_L3_BytePerSec % vp->SecPerClus);
			s = vp->FirstDataSector
			  + ((cl-2) << vp->SecPerClusPow)
			  + ((seek >> SD_L3_BytePerSecPow) & vp->SecPerClusMask);
			
			vp->cl_1st = firstcl;		// caching
			vp->cl_n   = n;
			vp->cl_cl  = cl;
		} else {
			if (cl != SD_L3_BADCL) {
				sd_errno = SD_E2BIG;
			}
		}
	}
	

	printf("L3seek2sect()=%ld\n", s);
	return(s);
}

	
//---------------------------------------------------------------------
//	APIs
//---------------------------------------------------------------------

//
// read input
//
int sd_L3_read(struct sd_L3_fd *fp, char *buf, int bufsz) {	// Readed byte length  0>=:EOF or error
	int		rd;
	long 	sek;
	long 	unr;
	

	printf("L3read(fp=0x%08lx,*,bufsz=%d) start\n", (long)fp, bufsz);
	rd  = 0;
	sek = fp->seekpos;
	unr = fp->filesz - sek;
	
	while((bufsz > 0) && (unr > 0)) {
		long	s;
		int		pos;
		int		rem;
		int		i;
		char	*p;
		
#ifdef SD_OPT_ARGCHECK
		if ((fp == 0) || (fp->no < 0) || (fp->no >= SD_L3_MAXFD)) {
			sd_errno = SD_EBADF;
			break;
		}
#endif
		
		// Get target sect#
		s = sd_L3_seek2sect(fp->vp, sek, fp->firstcl);
		if (s < 0) {
			rd = -1;
			break;
		}
		
		// Read target sect
		if (sd_L2_read(fp->pp, s) != 0) {
			rd = -1;
			break;
		}
		
		// Transfer to user buffer
		//			pos = sek % SD_L3_BytePerSec;
		pos = sek & SD_L3_BytePerSecMask;
#ifdef SD_OPT_BUF256
		rem = (SD_L3_BytePerSec - pos) & 0xff;
		if (rem == 0) {
			rem = 256;
		}
		if (pos & 0x100) {
			p = &sd_buf2[pos & 0xff];
		} else {
			p = &sd_buf1[pos];
		}
#else
		rem = SD_L3_BytePerSec - pos;
		p = &sd_buf[pos];
#endif
		if (rem > unr) {
			rem = unr;
		}
		if (bufsz < rem) {
			rem = bufsz;
		}
		sd_memcpy(buf, p, rem);
		
		printf("L3read pos=%d ", pos);
		printf("rem=%d ", rem);
		printf("bufsz=%d ", bufsz);
		printf("sek=%ld ", sek);
		printf("unr=%ld ", unr);
		printf("rd=%d\n", rd);
		
		// seek pos update
		sek   += rem;
		rd    += rem;
		unr   -= rem;
		bufsz -= rem;
		buf   += rem;
	}
	
	if (rd > 0) {
		fp->seekpos += rd;
	}
	
	printf("L3read(*,buf='%-8.8s...',*)=%d\n", (buf-rd), rd);
	i2c_lcd_position(0,0);i2c_lcd_prCString("L3read=");i2c_lcd_prHexInt((int)fp);i2c_lcd_prHexInt(rd);
	return(rd);
}

//
// write output
//
#ifdef SD_OPT_WRITE
int sd_L3_write(struct sd_L3_fd *fp, char *buf, int bufsz) {	// Writed byte length  0>=:error
	int wd;
	
	printf("L3write(fp=0x%08lx,buf='%-8.8s...',bufsz=%d) start\n", (long)fp, buf, bufsz);
	wd = 0;
	
	if (fp->wflg & SD_L3_WFLG_RO) {
		sd_errno = SD_EACCES;
		
#ifdef SD_OPT_ARGCHECK
	} else if ((fp == 0) || (fp->no < 0) || (fp->no >= SD_L3_MAXFD)) {
		sd_errno = SD_EBADF;
#endif

	} else {
		long sek;
		
		sek = fp->seekpos;
		while(bufsz > 0) {
			long	s;
			int		pos;
			int		rem;
			char	*p;
			
			// Get target sect#
			while((s = sd_L3_seek2sect(fp->vp, sek, fp->firstcl)) < 0) {
				unsigned short cl;
				
				if (sd_errno != SD_E2BIG) {
					break;
				}
				
				// growth storage
				cl = sd_L3_alloccl(fp->vp, fp->firstcl);
				
				// 1st grow, chaining root
				if (fp->firstcl == 0) {
					fp->firstcl = cl;
					fp->wflg   |= SD_L3_WFLG_WD;

					if (sd_L3_sync(fp) < 0) {
						break;
					}
				}
			}
			if (s < 0) {
				break;
			}
			
			// Read target sect
			if (bufsz < SD_L3_BytePerSec) {
				if (sd_L2_read(fp->pp, s) != 0) {
					break;
				}
			}
			
			// Transfer to user buffer
//			pos = sek % SD_L3_BytePerSec;
			pos = sek & SD_L3_BytePerSecMask;
#ifdef SD_OPT_BUF256
			if (pos & 0x100) {
				p = &sd_buf2[pos & 0xff];
			} else {
				p = &sd_buf1[pos];
			}
			rem = (SD_L3_BytePerSec - pos) & 0xff;
			if (rem == 0) {
				rem = 256;
			}
#else
			p = &sd_buf[pos];
			rem = SD_L3_BytePerSec - pos;
#endif
			if (bufsz < rem) {
				rem = bufsz;
			}
			sd_memcpy(p, buf, rem);
			
			// Rewrite
			if (sd_L2_write(fp->pp, s) != 0) {
				break;
			}
			fp->wflg |= SD_L3_WFLG_WD;
			
			// seek pos update
			sek   += rem;
			wd    += rem;
			bufsz -= rem;
			buf   += rem;
			
		}
	}
	if (wd > 0) {
		fp->seekpos += wd;
		if (fp->filesz < fp->seekpos) {
			fp->filesz = fp->seekpos;
		}
	}
	
	printf("L3write()=%d\n", wd);
	i2c_lcd_position(0,0);i2c_lcd_prCString("L3write=");i2c_lcd_prHexInt((int)fp);i2c_lcd_prHexInt(wd);
	return(wd);
}
#endif //SD_OPT_WRITE

//
// reposition read/write file offset
//
long sd_L3_lseek(struct sd_L3_fd *fp, long offset, char whence){ // Absolute seek posotion
	long	s;

	printf("L3lseek(fp=0x%08lx,offset=%ld,whence=%d) start\n", (long)fp, offset, whence);
	s = -1;
	
	for(;;){

#ifdef SD_OPT_ARGCHECK
		if ((fp == 0) || (fp->no < 0) || (fp->no >= SD_L3_MAXFD)) {
			sd_errno = SD_EBADF;
			break;
		}
#endif

		switch(whence) {
			case SD_SEEK_SET:
				s = offset;
				break;
			case SD_SEEK_CUR:
				s = fp->seekpos + offset;	// enable size < seek
				break;
			case SD_SEEK_END:
				s = fp->filesz;
				break;
			default:
				sd_errno = SD_EINVAL;
				break;
		}
		if (s >= 0) {
			fp->seekpos = s;
		}
		
		break;
	}
	
	printf("L3lseek()=%ld\n", s);
	i2c_lcd_position(0,0);i2c_lcd_prCString("L3lseek=");i2c_lcd_prHexInt(s);
	return(s);
}

//
// find directory entry (internal)
//
signed char sd_L3_finddir(struct sd_L3_vol *vp, struct sd_L3_dirent *de, char *dname, char len){	// 1:found 0:not found -1:error
	signed char	rc;
	
	printf("L3finddir(vp=0x%08lx,de=0x%08lx,dname=%-11.11s,len=%d) start\n", (long)vp, (long)de, dname, len);
	rc = -1;

	if (sd_L3_lseek(vp->dirfp, 0, SD_SEEK_SET) >= 0) {	// Rewind
		while(rc < 0) {
			if (sd_L3_read(vp->dirfp, (char *)de, sizeof(struct sd_L3_dirent)) != sizeof(sd_L3_tempdirent)) {
				break;
			}
			if (de->Name[0] == 0x00) {
				sd_errno = SD_ENOENT;
				rc = 0;
			}
			if (!sd_strncmp(de->Name, dname, len)) {
				rc = 1;
			}
		}
	}
	
	printf("L3finddir()=%d\n", rc);
	return(rc);
}

//
// open or create a file for reading or writing
//
struct sd_L3_fd *sd_L3_open(struct sd_L3_vol *vp, char *fname, unsigned char oflag, unsigned char mode) {	// File discriptor  -1:error
	signed char		fd;
	struct sd_L3_fd	*fp;
	
	printf("L3open(vp=0x%08lx,file=%s,oflag=0x%02x,mode=0x%02x) start\n", (long)vp, fname, oflag, mode);
	fd = -1;

	// dummy loop
	for(;;){
		
#ifdef SD_OPT_ARGCHECK
		if ((vp == 0) || (vp->no < 0) || (vp->no >= SD_L3_MAXVD)) {
			sd_errno = SD_EBADF;
			break;
		}
#endif

		//
		// attach new fd
		//
		fp = &sd_L3_fdtbl[SD_L3_MAXFD];
		for(fd = SD_L3_MAXFD-1; fd >= 0; --fd) {
			--fp;
			if (fp->no < 0) {
				break;
			}
		}
		if (fd < 0) {
			sd_errno = SD_EMFILE;
			break;
		}
		fp->vp      = vp;
		fp->pp      = vp->pp;
		fp->seekpos = 0;
		fp->wflg    = 0;

		//
		// Root directory file open
		//
		if ((fname[0] == '/') && (fname[1] == '\0')) {
			fp->firstcl = SD_L3_ROOTDIRCL;
			fp->filesz  = vp->RootEntCnt << SD_L3_RootEntSzPow;
			fp->dirloc  = -1;			// Root directory is no entry
			
		//
		// Normal file open
		//
		} else {
			char		m[SD_L3_DIRENT_NAMESZ+1];
			long		dl;				// directory entry location
			signed char	found;
			
			//
			// Format file name
			//
			sd_L3_convdirname(m, fname);
			m[SD_L3_DIRENT_NAMESZ] = '\0';
			if (m[0] == ' ') {
				sd_errno = SD_EINVAL;
				break;
			}
			
			//
			// Directory entry
			//
			found = 0;
			{
				// Search directory entry
				if ((found = sd_L3_finddir(vp, &sd_L3_tempdirent, m, SD_L3_DIRENT_NAMESZ)) < 0) {
					break;
				}
				
				// Create option
#ifdef SD_OPT_WRITE
				if ((!found) && (oflag & SD_O_CREAT)) {
					char f[1];
					
					// Search free entry
					f[0] = 0xe5;
					if (sd_L3_finddir(vp, &sd_L3_tempdirent, f, 1) < 0) {
						break;
					}
					found = 1;
					
					sd_memcpy(sd_L3_tempdirent.Name, m, sizeof(sd_L3_tempdirent.Name));
					sd_L3_tempdirent.Attr			= mode;
					sd_L3_tempdirent.NTRes			= 0;
					sd_L3_tempdirent.CrtTimeTenth	= sd_L3_NowTimeTenth;
					sd_bounconv2(sd_L3_tempdirent.CrtTime, sd_L3_NowTime);
					sd_L3_tempdirent.WrtTime[0]		= sd_L3_tempdirent.CrtTime[0];
					sd_L3_tempdirent.WrtTime[1]		= sd_L3_tempdirent.CrtTime[1];
					sd_bounconv2(sd_L3_tempdirent.CrtDate, sd_L3_NowDate);
					sd_L3_tempdirent.LstAccDate[0]	= sd_L3_tempdirent.CrtDate[0];
					sd_L3_tempdirent.WrtDate[0]		= sd_L3_tempdirent.CrtDate[0];
					sd_L3_tempdirent.LstAccDate[1]	= sd_L3_tempdirent.CrtDate[1];
					sd_L3_tempdirent.WrtDate[1]		= sd_L3_tempdirent.CrtDate[1];
					sd_L3_tempdirent.FstClusLO[0]	= 0;
					sd_L3_tempdirent.FstClusLO[1]	= 0;
					sd_L3_tempdirent.FileSize[0]	= 0;
					sd_L3_tempdirent.FileSize[1]	= 0;
					sd_L3_tempdirent.FileSize[2]	= 0;
					sd_L3_tempdirent.FileSize[3]	= 0;

					// Rewrite
					if (sd_L3_lseek(vp->dirfp, -(long)sizeof(sd_L3_tempdirent), SD_SEEK_CUR) < 0) {
						break;
					}
					if (sd_L3_write(vp->dirfp, (char *)&sd_L3_tempdirent, sizeof(sd_L3_tempdirent)) != sizeof(sd_L3_tempdirent)) {
						break;
					}
				}
#endif // SD_OPT_WRITE
				dl = sd_L3_lseek(vp->dirfp, 0, SD_SEEK_CUR) - sizeof(sd_L3_tempdirent);
				
				// Exists
				if ((found) && (oflag & SD_O_EXCL)) {
					sd_errno = SD_EEXIST;
					break;
				}
				if (!found) {
					sd_errno = SD_ENOENT;
					break;
				}
			}
			
			//
			// Init fd table
			//
			
			// setup fd table
			fp->firstcl = sd_boconv2(sd_L3_tempdirent.FstClusLO);
			fp->filesz  = sd_boconv4(sd_L3_tempdirent.FileSize);
			fp->dirloc  = dl;
			
			// check file attribute
			if ((sd_L3_tempdirent.Attr & 0x01) || (oflag == SD_O_RDONLY)) {
				fp->wflg |= SD_L3_WFLG_RO;
			}
			
			// truncate
			if (oflag & SD_O_TRUNC) {
				fp->filesz = 0;
			}
			
			// append
			if (oflag & SD_O_APPEND) {
				fp->seekpos = fp->filesz;
			}
		}
		
		
		printf("fd[%d].no=%d\n", fd, fp->no);
		printf("fd[%d].wflg=%d\n", fd, fp->wflg);
		printf("fd[%d].firstcl=%d\n", fd, fp->firstcl);
		printf("fd[%d].filesz=%ld\n", fd, fp->filesz);
		printf("fd[%d].seekpos=%ld\n", fd, fp->seekpos);
		printf("fd[%d].dirloc=%ld\n", fd, fp->dirloc);
		printf("fd[%d].vp=0x%08lx\n", fd, (long)fp->vp);
		printf("fd[%d].pp=0x%08lx\n", fd, (long)fp->pp);
		
		//
		// commit
		//
		fp->no = fd;
		break;
	}

	if ((fd < 0) || (fp->no < 0)) {
		fp = 0;
	}
	
	printf("L3open()=0x%08lx errno=%d\n", (long)fp, sd_errno);
	i2c_lcd_position(0,0);i2c_lcd_prCString("L3open=");i2c_lcd_prHexInt((int)fp);i2c_lcd_prHexByte(sd_errno);
	return(fp);
}

//
// sync buffer - device
//
#ifdef SD_OPT_WRITE
signed char sd_L3_sync(struct sd_L3_fd *fp) {	// 0:normal -1:error
	signed char			rc;
	struct sd_L3_vol	*vp;

	printf("L3sync(fp=0x%08lx) start\n", (long)fp);
	rc = -1;
	vp = fp->vp;
	
	// dummy loop
	for(;;) {
		
#ifdef SD_OPT_ARGCHECK
		if ((fp == 0) || (fp->no < 0) || (fp->no >= SD_L3_MAXFD)) {
			sd_errno = SD_EBADF;
			break;
		}
#endif

		// Directory update
		if ((fp->wflg & SD_L3_WFLG_WD) && (fp->dirloc >= 0)) {
			if (sd_L3_lseek(vp->dirfp, fp->dirloc, SD_SEEK_SET) <= 0) {
				break;
			}
			if (sd_L3_read(vp->dirfp, (char *)&sd_L3_tempdirent, sizeof(sd_L3_tempdirent)) != sizeof(sd_L3_tempdirent)) {
				break;
			}
			sd_bounconv2(sd_L3_tempdirent.FstClusLO, fp->firstcl);
			sd_bounconv4(sd_L3_tempdirent.FileSize,  fp->filesz);
			sd_bounconv2(sd_L3_tempdirent.CrtTime, sd_L3_NowTime);
			sd_bounconv2(sd_L3_tempdirent.CrtDate, sd_L3_NowDate);
			if (sd_L3_lseek(vp->dirfp, fp->dirloc, SD_SEEK_SET) <= 0) {
				break;
			}
			if (sd_L3_write(vp->dirfp, (char *)&sd_L3_tempdirent, sizeof(sd_L3_tempdirent)) != sizeof(sd_L3_tempdirent)) {
				break;
			}
			fp->wflg &= ~SD_L3_WFLG_WD;
		}

		// Delayed write trigger
		if (sd_L2_read(vp->pp, 0) != 0) {
			break;
		}
		
		rc = 0;
		break;
	}
	
	printf("L3sync()=%d\n", rc);
	i2c_lcd_position(0,0);i2c_lcd_prCString("L3sync=");i2c_lcd_prHexInt((int)fp);i2c_lcd_prHexByte(rc);
	return(rc);
}
#endif //SD_OPT_WRITE

//
// delete a descriptor
//
signed char sd_L3_close(struct sd_L3_fd *fp) {
										// -1:error 0:normal
	signed char	rc;						// return code

	printf("L3close(fp=0x%08lx) start\n", (long)fp);
	rc = -1;
	
	// dummy loop
	for(;;) {

#ifdef SD_OPT_ARGCHECK
		if ((fp == 0) || (fp->no < 0) || (fp->no >= SD_L3_MAXFD)) {
			sd_errno = SD_EBADF;
			break;
		}
#endif //SD_OPT_ARGCHECK

#ifdef SD_OPT_WRITE
		if (sd_L3_sync(fp) != 0) {
#ifndef SD_OPT_FORCECLOSE
			break;
#endif //SD_OPT_FORCECLOSE
		}
#endif //SD_OPT_WRITE

		// fd free
		fp->no = -1;

		rc = 0;
		break;
	}
	
	printf("L3close()=%d\n", rc);
	i2c_lcd_position(0,0);i2c_lcd_prCString("L3close=");i2c_lcd_prHexInt((int)fp);i2c_lcd_prHexByte(rc);
	return(rc);
}

//
// remove directory entry
//
#ifdef SD_OPT_UNLINK
signed char sd_L3_unlink(struct sd_L3_vol *vp, char *fname){
	signed char rc;
		
	printf("L3unlink(vp=0x%08lx,fname=%s) start\n", (long)vp, fname);
	rc = -1;
	
	// dummy loop
	for(;;) {
		char			m[SD_L3_DIRENT_NAMESZ+1];
		unsigned short	cl;

#ifdef SD_OPT_ARGCHECK
		if ((vp == 0) || (vp->no < 0) || (vp->no >= SD_L3_MAXVD)) {
			sd_errno = SD_EBADF;
			break;
		}
#endif //SD_OPT_ARGCHECK

		// Format name
		sd_L3_convdirname(m, fname);
		m[SD_L3_DIRENT_NAMESZ] = '\0';

		// Erase flag on
		if (sd_L3_finddir(vp, &sd_L3_tempdirent, m, SD_L3_DIRENT_NAMESZ) <= 0) {
			break;
		}
		cl = sd_boconv2(sd_L3_tempdirent.FstClusLO);
		sd_L3_tempdirent.Name[0] = 0xe5;
		if (sd_L3_lseek(vp->dirfp, -(long)sizeof(sd_L3_tempdirent), SD_SEEK_CUR) < 0) {
			break;
		}
		if (sd_L3_write(vp->dirfp, (char *)&sd_L3_tempdirent, sizeof(sd_L3_tempdirent)) < 0) {
			break;
		}

		// Free cluster chain
		if (sd_L3_freecl(vp, cl) < 0) {
			break;
		}

		rc = 0;
		break;
	}
			
	printf("L3unlink()=%d\n", rc);
	i2c_lcd_position(0,0);i2c_lcd_prCString("L3unlink=");i2c_lcd_prHexInt((int)vp);i2c_lcd_prHexByte(rc);
	return(rc);
}
#endif //SD_OPT_UNLINK

//
// mount a filesystem
//
struct sd_L3_vol *sd_L3_mount(char partno) {
	signed char			rc;
	signed char			vd;
	struct sd_L3_vol	*vp;
	struct sd_L2_part	*pp;
	
	printf("L3mount(partno=%d) start\n", partno);
	rc = -1;

	// dummy loop
	for(;;) {
		struct bpb1 {
			unsigned char BPB_jmpBoot[3];
			unsigned char BPB_OEMName[8];
			unsigned char BPB_BytePerSec[2];
			unsigned char BPB_SecPerClus;
			unsigned char BPB_RsvdSecCnt[2];
			unsigned char BPB_NumFATs;
			unsigned char BPB_RootEntCnt[2];
			unsigned char BPB_TotSec16[2];
			unsigned char BPB_Media;
			unsigned char BPB_FATSz16[2];
			unsigned char BPB_SecPerTrk[2];
			unsigned char BPB_NumHeads[2];
			unsigned char BPB_HiddSec[4];
			unsigned char BPB_TotSec32[4];
			unsigned char res[220];
		} *bpb1;
		struct bpb2 {
			unsigned char res[254];
			unsigned char sig[2];
		} *bpb2;
		unsigned short	BytePerSec;
//		unsigned char	SecPerClus;
//		unsigned short	RsvdSecCnt;
//		unsigned char	NumFATs;
//		unsigned short	RootEntCnt;
//		unsigned short	TotSec16;
//		unsigned char	Media;
//		unsigned short	FATSz16;
//		unsigned short	SecPerTrk;
//		unsigned short	NumHeads;
//		unsigned long	HiddSec;
//		unsigned long	TotSec32;
		long			TotSec;
		
		unsigned short	RsvdSectors;
		unsigned short	FATSectors;
		unsigned short	RootDirSectors;
		long			DataSectors;
		
		unsigned char	n;
		char			rootdirname[] = "/";
		

		//
		// attach new vd
		//
		vp = &sd_L3_vol[0];
		for(vd = 0; vd < SD_L3_MAXVD; vd++) {
			if (vp->no < 0) {
				break;
			}
			vp++;
		}
		if (vd >= SD_L3_MAXVD) {
			sd_errno = SD_EMFILE;
			break;
		}
		vp->cl_1st = SD_L3_BADCL;		// cluster manage
		vp->cl_n   = 0xffff;			//
		vp->cl_cl  = 0;					//

		// device open
		if ((pp = sd_L2_open(partno)) == 0) {
			break;
		}
		vp->pp = pp;

		// Check target partition
		if ((pp->parttype != SD_L3_PARTID_1) &&
			(pp->parttype != SD_L3_PARTID_2) &&
			(pp->parttype != SD_L3_PARTID_3) &&
			(pp->parttype != SD_L3_PARTID_4) &&
			(pp->parttype != SD_L3_PARTID_5) &&
			(pp->parttype != SD_L3_PARTID_6)) {
			sd_errno = SD_ENOTBLK;
			break;
		}
		
		// get boot sector (reserved region)
		if (sd_L2_read(pp, 0) != 0) {
			break;
		}
#ifdef SD_OPT_BUF256
		bpb1 = (struct bpb1 *)sd_buf1;
		bpb2 = (struct bpb2 *)sd_buf2;
#else
		bpb1 = (struct bpb1 *)&(sd_buf[0]);
		bpb2 = (struct bpb2 *)&(sd_buf[256]);
#endif
		if ((bpb2->sig[0] != 0x55) ||
			(bpb2->sig[1] != 0xaa)) {
			sd_errno = SD_ENXIO;
			break;
		}
		
		// set value
		BytePerSec			= sd_boconv2(bpb1->BPB_BytePerSec);
		vp->SecPerClus		= bpb1->BPB_SecPerClus;
		RsvdSectors			= sd_boconv2(bpb1->BPB_RsvdSecCnt);
		vp->NumFATs			= bpb1->BPB_NumFATs;
		vp->RootEntCnt		= sd_boconv2(bpb1->BPB_RootEntCnt);
		TotSec				= sd_boconv2(bpb1->BPB_TotSec16);
		if (TotSec == 0) {
			TotSec			= sd_boconv4(bpb1->BPB_TotSec32);
		}
		vp->FATSz			= sd_boconv2(bpb1->BPB_FATSz16);
		
		// convert 512byte sector
		n = sd_log2(BytePerSec) - SD_L3_BytePerSecPow;
		vp->FATSz			= vp->FATSz << n;
		vp->SecPerClus		= vp->SecPerClus << n;
		RsvdSectors			= RsvdSectors << n;
		TotSec				= TotSec << n;
		
		// calcurate region size and top location
		FATSectors			= (vp->NumFATs * vp->FATSz);
//		RootDirSectors		= ((vp->RootEntCnt * SD_L3_RootEntSz) + (SD_L3_BytePerSec - 1)) / SD_L3_BytePerSec;
		RootDirSectors		= ((vp->RootEntCnt * SD_L3_RootEntSz) + (SD_L3_BytePerSec - 1)) >> SD_L3_BytePerSecPow;
		DataSectors			= TotSec - (RsvdSectors + FATSectors + RootDirSectors);
		
		vp->FirstFATSector	= RsvdSectors;
		vp->RootDirSector	= RsvdSectors + FATSectors;
		vp->FirstDataSector	= RsvdSectors + FATSectors + RootDirSectors;
		
// calcurate other parameter
		vp->SecPerClusPow	= sd_log2(vp->SecPerClus);
		vp->SecPerClusMask	= ~(0xffff << vp->SecPerClusPow);
//		vp->BytePerClus		= (int)vp->SecPerClus * SD_L3_BytePerSec;
		vp->BytePerClus		= (int)vp->SecPerClus << SD_L3_BytePerSecPow;
		vp->BytePerClusPow	= vp->SecPerClusPow + SD_L3_BytePerSecPow;
//		vp->FATEntCnt		= DataSectors / vp->SecPerClus;		// CountofClusters
		vp->FATEntCnt		= DataSectors >> vp->SecPerClusPow;	// CountofClusters
		if (vp->FATEntCnt < 4085) {
			vp->FATType = SD_L3_FAT12;
		} else if(vp->FATEntCnt < 65525) {
			vp->FATType = SD_L3_FAT16;
		} else {
			vp->FATType = SD_L3_FAT32;
		}
		
		printf("vp->NumFATs=%d\n", vp->NumFATs);
		printf("vp->FATSz=%d\n", vp->FATSz);
		printf("vp->RootEntCnt=%d\n", vp->RootEntCnt);
		printf("vp->SecPerClus=%d\n", vp->SecPerClus);
		printf("RsvdSectors=%d\n", RsvdSectors);
		printf("FATSectors=%d\n", FATSectors);
		printf("RootDirSectors=%d\n", RootDirSectors);
		printf("DataSectors=%ld\n", DataSectors);
		printf("vp->FirstFATSector=%ld\n", vp->FirstFATSector);
		printf("vp->RootDirSector=%ld\n", vp->RootDirSector);
		printf("vp->FirstDataSector=%ld\n", vp->FirstDataSector);
		printf("vp->FATEntCnt=%d\n", vp->FATEntCnt);
		printf("vp->SecPerClusPow=%d\n", vp->SecPerClusPow);
		printf("vp->SecPerClusMask=%d\n", vp->SecPerClusMask);
		printf("vp->BytePerClus=%ld\n", vp->BytePerClus);
		printf("vp->BytePerClusPow=%d\n", vp->BytePerClusPow);
		printf("vp->FATType=%d\n", vp->FATType);
		printf("vp->pp=0x%08lx\n", (long)vp->pp);
		
		if ((vp->RootEntCnt == 0) ||	// FAT32 volume?
			(vp->FATSz      == 0) ||
			(vp->FATType    == SD_L3_FAT32) ||
			((vp->FATType   == SD_L3_FAT12) && (n != 0))) {
			sd_errno = SD_ENODEV;
			break;
		}

		// Commit
		vp->no = vd;

		// Open root directory file
		if ((vp->dirfp = sd_L3_open(vp, rootdirname, SD_O_RDWR, SD_ATTR_DIRECTORY)) == 0) {
			break;
		}

		rc = 0;
		
		break;
	}
	
	if (rc) {
		if (vp->dirfp != 0) {
			sd_L3_close(vp->dirfp);
		}
		if (pp != 0) {
			sd_L2_close(pp);
		}
		vp->no = -1;
		vp = 0;
	}

	printf("L3mount()=0x%08lx\n", (long)vp);
	i2c_lcd_position(0,0);i2c_lcd_prCString("L3mount=");i2c_lcd_prHexInt((int)vp);i2c_lcd_prHexByte(sd_errno);
	return(vp);
}

//
// dismount a filesystem
//
signed char sd_L3_unmount(struct sd_L3_vol *vp) {
	signed char rc;
	
	printf("L3unmount(0x%08lx) start\n", (long)vp);
	rc = -1;
	
	for (;;) {
		
#ifdef SD_OPT_ARGCHECK
		if ((vp == 0) || (vp->no < 0) || (vp->no >= SD_L3_MAXVD)) {
			sd_errno = SD_EBADF;
			break;
		}
#endif
		
		// Close directory file
		if (vp->dirfp != 0) {
			if (sd_L3_close(vp->dirfp) < 0) {
				break;
			}
			vp->dirfp = 0;
		}

		// Close device
		if (sd_L2_close(vp->pp) != 0) {
			break;
		}

		vp->no = -1;
		rc = 0;
		break;
	}
	
	printf("L3unmount()=%d\n", rc);
	i2c_lcd_position(0,0);i2c_lcd_prCString("L3unmount=");i2c_lcd_prHexInt((int)vp);i2c_lcd_prHexByte(rc);
	return(rc);
}
//
//  sd_common.c
//
//  SDcard control module mini - Common subroutine
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
//	2010.04.02 v0.1c Bug fix, file not found
//	2010.07.07 v0.2  Support ATmega328P
//

#include "SD/sd_config.h"					// Private


//
//	C o m m o n   a r e a
//
// detail error code
char sd_errno = 0;

//
//	Private string.h s
//
#ifdef SD_OPT_STRING_H
void sd_memcpy(char *d, char *s, char n) {
	for(; n > 0; --n) {
		*d++ = *s++;
	}
	return;
}
signed char	sd_strncmp(char *s1, char *s2, int n){
	--n;																// ADD 2010.04.02
	for(; n > 0; --n,s1++,s2++) {
		if (*s1 == 0 || *s1 != *s2) {
			break;
		}
	}
	return((unsigned char)*s1 - (unsigned char)*s2);
}
//#else							// redefine standerd library			// DEL 2010.04.02
//#define sd_memcpy(d,s,n)		memcpy((void *)(d),(void *)(s),(n))		// DEL 2010.04.02
//#define sd_strncmp(s1,s2,n)	strncmp((char *)(s1),(char *)(s2),(n))	// DEL 2010.04.02
#endif

//
//	Convert byte order
//
//#ifdef __18CXX																							// DEL 2010.07.07
//#define sd_boconv2(s)		(*((unsigned short *)(s)))														// DEL 2010.07.07
//#define sd_boconv4(s)		(*((unsigned long *)(s)))														// DEL 2010.07.07
//#define sd_bounconv2(s,a)	(*((unsigned short *)(s)) = (a))												// DEL 2010.07.07
//#define sd_bounconv4(s,a)	(*((unsigned long *)(s)) = (a))													// DEL 2010.07.07
//#else																										// DEL 2010.07.07
//#define sd_boconv2(s)		sd_boconv((unsigned char *)(s), sizeof(unsigned short))							// DEL 2010.07.07
//#define sd_boconv4(s)		sd_boconv((unsigned char *)(s), sizeof(unsigned long))							// DEL 2010.07.07
//#define sd_bounconv2(s,a)	sd_bounconv((unsigned char *)(s), sizeof(unsigned short), (unsigned long)(a))	// DEL 2010.07.07
//#define sd_bounconv4(s,a)	sd_bounconv((unsigned char *)(s), sizeof(unsigned long), (unsigned long)(a))	// DEL 2010.07.07
#if !defined(__18CXX) && !defined(__AVR__)																	// ADD 2010.07.07
unsigned long sd_boconv(unsigned char *buf, char len) {
	unsigned long a;
	unsigned char *p;
	
	a = 0;
	p = buf + len;
	for(; len > 0; --len) {
		a = a << 8 | *--p;
	}
	return(a);
}

void sd_bounconv(unsigned char *buf, char len, unsigned long a) {
	for(; len > 0; --len) {
		*buf++ = a;
		a = a >> 8;
	}
	return;
}
#endif

//
//	Logarithms
//
signed char sd_log2(unsigned short x) {
	char i;
	
	for(i = 15; i >= 0; --i) {
		if (x & 0x8000) {
			break;
		}
		x <<= 1;
	}
	return(i);
}


#endif // SD_H
