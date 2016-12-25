//
//  sd_L3.h
//
//  SDcard control module mini - Hardware interface for microchip PIC18
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

#ifndef SD_L3_H
#define SD_L3_H

#include "sd_config.h"					// Private


//
//	Constant
//
// oflags
#define SD_O_RDONLY			0x00		// open for reading only
#define SD_O_WRONLY			0x01		// open for writing only
#define SD_O_RDWR			0x02		// open for reading and writing
//#define SD_O_NONBLOCK		0xXX		// do not block on open or for data to become available
#define SD_O_APPEND			0x04		// append on each write
#define SD_O_CREAT			0x08		// create file if it does not exist
#define SD_O_TRUNC			0x10		// truncate size to 0
#define SD_O_EXCL			0x20		// error if O_CREAT and the file exists
//#define SD_O_SHLOCK		0xXX		// atomically obtain a shared lock
//#define SD_O_EXLOCK		0xXX		// atomically obtain an exclusive lock
//#define SD_O_NOFOLLOW		0xXX		// do not follow symlinks
//#define SD_O_SYMLINK		0xXX		// allow open of symlinks
//#define SD_O_EVTONLY		0xXX		// descriptor requested for event notifications only

// attr
#define SD_ATTR_READ_ONLY	0x01
#define SD_ATTR_HIDDEN		0x02
#define SD_ATTR_SYSTEM		0x04
#define SD_ATTR_VOLUME_ID	0x08
#define SD_ATTR_DIRECTORY	0x10
#define SD_ATTR_ARCHIVE		0x20
#define SD_ATTR_LONG_NAME	ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID

// whence
#define SD_SEEK_SET			0			// the offset is set to offset bytes.
#define SD_SEEK_CUR			1			// the offset is set to its current location plus offset bytes.
#define SD_SEEK_END			2			// the offset is set to the size of the file plus offset bytes.

// control table
#define SD_L3_MAXVD 1					// Maximum mount
#define SD_L3_INIVD	{{-1}}				// Initial value
#define SD_L3_MAXFD 3					// Maximum open + 1
#define SD_L3_INIFD	{{-1},{-1},{-1}}	// Initial value

// sd_L3_fdtbl[].wflg
#define SD_L3_WFLG_RO	0x01			// Read only request
#define SD_L3_WFLG_WD	0x02			// Writed

//
//	Data structure and area
//
// Volume information table
struct sd_L3_vol {
	signed char		no;					// Entry# (-1:free)

	// Region size and location (unit=512byte/sector)
	long			FirstFATSector;		//
	long			RootDirSector;		//
	long			FirstDataSector;	//
	
	// FAT region parameter
	unsigned short	FATEntCnt;			// CountofClusters
	unsigned short	FATSz;				// sect
	unsigned char	NumFATs;			//
	char			FATType;			//
	
	// Root directory region parameter
	unsigned short	RootEntCnt;			//
	
	// Data region parameter
	unsigned short	SecPerClus;
	char			SecPerClusPow;		// 2^SecPerClusPow == SecPerClus
	char			SecPerClusMask;		//
	long 			BytePerClus;		// SecPerClus * SD_L3_BytePerSec
	char			BytePerClusPow;		// 2^SecPerClusPow == SecPerClus

	// Other
	unsigned short	cl_1st;				// Cache memory
	unsigned short	cl_n;				//
	unsigned short	cl_cl;				//
	struct sd_L3_fd		*dirfp;			// directory file, not residence
	struct sd_L2_part	*pp;			// Target partition
};
extern struct sd_L3_vol	sd_L3_vol[SD_L3_MAXVD];

// File access control table
struct sd_L3_fd {
	signed char		no;					// Entry# (-1:free)
	unsigned char 	wflg;				// Write flag
	unsigned short	firstcl;			// Cluster number top
	long			filesz;				// Byte length
	long			seekpos;			// Byte address
	long			dirloc;				// Directory entry seek
	struct sd_L3_vol	*vp;			// Target volume
	struct sd_L2_part	*pp;			// Target partition
};
extern struct sd_L3_fd	sd_L3_fdtbl[SD_L3_MAXFD];

// Directory access
#define SD_L3_DIRENT_NAMESZ	11
struct sd_L3_dirent {					// FAT file system's directory structure
	char			Name[SD_L3_DIRENT_NAMESZ];
	unsigned char	Attr;
	unsigned char	NTRes;
	unsigned char	CrtTimeTenth;
	unsigned char	CrtTime[2];
	unsigned char	CrtDate[2];
	unsigned char	LstAccDate[2];
	unsigned char	FstClusHI[2];
	unsigned char	WrtTime[2];
	unsigned char	WrtDate[2];
	unsigned char	FstClusLO[2];
	unsigned char	FileSize[4];
};

// date/time interface
extern unsigned short	sd_L3_NowDate;		// bit: yyyyyyymmmmddddd
extern unsigned short	sd_L3_NowTime;		// bit: hhhhhmmmmmmsssss
extern unsigned char	sd_L3_NowTimeTenth;	// Need real time update

//
//	Prototype
//
extern struct sd_L3_vol	*sd_L3_mount(char partno);
extern signed char		sd_L3_unmount(struct sd_L3_vol *vp);
extern signed char		sd_L3_unlink(struct sd_L3_vol *vp, char *fname);
extern struct sd_L3_fd	*sd_L3_open(struct sd_L3_vol *vp, char *fname, unsigned char oflag, unsigned char attr);
extern signed char		sd_L3_close(struct sd_L3_fd *fp);
extern int				sd_L3_read(struct sd_L3_fd *fp, char *buf, int bufsz);
extern int				sd_L3_write(struct sd_L3_fd *fp, char *buf, int bufsz);
extern long				sd_L3_lseek(struct sd_L3_fd *fp, long offset, char whence);
extern signed char		sd_L3_sync(struct sd_L3_fd *fp);


#endif // SD_L3_H
