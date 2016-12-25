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

#include "sd_config.h"
#include "sd_common.h"
#include "sd_L3.h"


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


#endif // SD_H
