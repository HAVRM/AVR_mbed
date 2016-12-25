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
#include "sd_config.h"					// Private
#include "sd_common.h"					// Private
#include "sd_L2.h"						// Private
#include "sd_L3.h"						// Private
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

