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

#include "sd_config.h"					// Private
#include "sd_common.h"					// Private
#include "sd_L1.h"						// Private
#include "sd_L2.h"						// Private

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


