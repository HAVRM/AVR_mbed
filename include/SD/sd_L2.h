//
//  sd_L2.h
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
//
//	Update history
//	---------- ----- -------------------------------------------
//	2009.12.18 v0.0  First cording
//	2010.01.11 v0.1  Support PIC18F2550
//	2010.01.16 v0.1a Split source
//	2010.07.07 v0.2  Support ATmega328P
//

#ifndef SD_L2_H
#define SD_L2_H

#include "sd_config.h"					// Private


//
// Constant
//
#define SD_BUFSZ		512
#define SD_L2_MAXPD 	1				// Maximum open
#define SD_L2_INIPD		{{-1}}			// Initial value
#define SD_L2_NOPARTID	0xff

//
//	Data structure and area
//
// Data buffer
#ifdef SD_OPT_BUF256
extern char	sd_buf1[SD_BUFSZ/2];
extern char	sd_buf2[SD_BUFSZ/2];
#else
extern char	sd_buf[SD_BUFSZ];
#endif

// Partition information table
struct sd_L2_part {
	signed char		no;					// Entry# (-1:free)
	char			delayw;				// Write cache flag
	char			sig;				// Fail safe
	
	unsigned char	parttype;			// ID
	long			startsect;			// Partition top
	long			totalsect;			// Partition size
	long			bufsect;			// Memory buffer in sector
};
extern struct sd_L2_part	sd_L2_part[SD_L2_MAXPD];


//
//	Prototype
//
extern struct sd_L2_part	*sd_L2_open(char partno);
extern signed char			sd_L2_close(struct sd_L2_part *pp);
extern signed char			sd_L2_read(struct sd_L2_part *pp, long sect);
#ifdef SD_OPT_WRITE
extern signed char			sd_L2_write(struct sd_L2_part *pp, long sect);
#else
#define sd_L2_write(pp,sect)	(0)
#endif


#endif // SD_L2_H
