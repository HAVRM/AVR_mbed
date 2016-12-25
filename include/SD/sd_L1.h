//
//  sd_L1.h
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
//
//	Update history
//	---------- ----- -------------------------------------------
//	2009.12.18 v0.0  First cording
//	2010.01.11 v0.1  Support PIC18F2550
//	2010.01.16 v0.1a Split source
//	2010.07.07 v0.2  Support ATmega328P
//

#ifndef SD_L1_H
#define SD_L1_H

#include "sd_config.h"


//
//	Constant
//
// SDcard-Machine spec condition
#define SD_WAIT_POWERUP 		10		// 0xff send count
#define SD_WAIT_INIT_INTERVAL	10		// ACMD41(initialyze complete check) [ms]
#define SD_WAIT_INIT_COUNT		100		// Total wait = SD_WAIT_INIT_INTERVAL * SD_WAIT_INIT_COUNT
#define SD_WAIT_UNIT 			0		// x50[us]
#define SD_WAIT_RESPONSE		10		// 1count = SD_WAIT_UNIT + <1byte TX time>
#define SD_WAIT_READ			10000	// 1count = SD_WAIT_UNIT + <1byte TX time>
#define SD_WAIT_WRITE			10000	// 1count = SD_WAIT_UNIT + <1byte TX time>
#define SD_WAIT_INFO			10000	// 1count = SD_WAIT_UNIT + <1byte TX time>
#define SD_RETRY_CMD 			3		// Error retry / Command & Response
#define SD_RETRY_READ			3		// Error retry / 512byte data read
#define SD_RETRY_WRITE			3		// Error retry / 512byte data write
#define SD_RETRY_INFO			3		// Error retry / 16byte data read

// Data buffer size
#define SD_BUFSZ	512

// SDcard response code
#define SD_R1_IDLE_STATE		0x01
#define SD_R1_ERASE_RESET		0x02
#define SD_R1_ILLEGAL_CMD		0x04
#define SD_R1_CRC_ERR			0x08
#define SD_R1_ERASE_ERR			0x10
#define SD_R1_ADDR_ERR			0x20
#define SD_R1_PRM_ERR			0x40
#define SD_R1_TIMEOUT			0xff

// getinfo type
#define SD_L1_INFOTYPE_MEDIASIZE	0	// long [byte]

//
//	Data structure and area
//

//
//	Prototype
//
#ifdef SD_OPT_BUF256

	extern unsigned char	sd_L1_open(void);						// Response code, 0:normal
	extern void				sd_L1_close(void);
	extern unsigned char	sd_L1_read(long sect, char *buf1, char *buf2);	// Response code, 0:normal
	#ifdef SD_OPT_WRITE
	extern unsigned char	sd_L1_write(long sect, char *buf1, char *buf2);	// Response code, 0:normal
	#else
	#define					sd_L1_write(sect,buf1,buf2)		(0)		// dummy
	#endif // SD_OPT_WRITE
	extern unsigned char	sd_L1_getinfo(void *info, char type);	// Response code, 0:normal

#else

	extern unsigned char	sd_L1_open(void);						// Response code, 0:normal
	extern void				sd_L1_close(void);
	extern unsigned char	sd_L1_read(long sect, char *buf);		// Response code, 0:normal
	#ifdef SD_OPT_WRITE
	extern unsigned char	sd_L1_write(long sect, char *buf);		// Response code, 0:normal
	#else
	#define					sd_L1_write(sect,buf)	(0)				// dummy
	#endif // SD_OPT_WRITE
	extern unsigned char	sd_L1_getinfo(void *info, char type);	// Response code, 0:normal

#endif // SD_OPT_BUF256

#endif // SD_L1_H
