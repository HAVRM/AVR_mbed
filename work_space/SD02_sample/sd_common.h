//
//  sd_common.h
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
//
//	Update history
//	---------- ----- -------------------------------------------
//	2009.12.18 v0.0  First cording
//	2010.01.11 v0.1  Support PIC18F2550
//	2010.01.16 v0.1a Split source
//	2010.07.07 v0.2  Support ATmega328P
//

#ifndef SD_COMMON_H
#define SD_COMMON_H

#include "sd_config.h"
#ifndef SD_OPT_STRING_H
#include <string.h>						// c library
#endif


//
//	Error codes
//
extern char sd_errno;
#define SD_EPERM		1				/* Operation not permitted */	// Not insert
#define SD_ENOENT		2				/* No such file or directory */
#define SD_ESRCH		3				/* No such process */
#define SD_EINTR		4				/* Interrupted system call */
#define SD_EIO			5				/* Input/output error */
#define SD_ENXIO		6				/* Device not configured */		// Unformated
#define SD_E2BIG		7				/* Argument list too long */
#define SD_ENOEXEC		8				/* Exec format error */
#define SD_EBADF		9				/* Bad file descriptor */
#define SD_ECHILD		10				/* No child processes */
#define SD_EDEADLK		11				/* Resource deadlock avoided */ /* 11 was EAGAIN */
#define SD_ENOMEM		12				/* Cannot allocate memory */
#define SD_EACCES		13				/* Permission denied */
#define SD_EFAULT		14				/* Bad address */
#define SD_ENOTBLK		15				/* Block device required */
#define SD_EBUSY		16				/* Device / Resource busy */
#define SD_EEXIST		17				/* File exists */
#define SD_EXDEV		18				/* Cross-device link */
#define SD_ENODEV		19				/* Operation not supported by device */	// Unmatch file system
#define SD_ENOTDIR		20				/* Not a directory */
#define SD_EISDIR		21				/* Is a directory */
#define SD_EINVAL		22				/* Invalid argument */
#define SD_ENFILE		23				/* Too many open files in system */
#define SD_EMFILE		24				/* Too many open files */
#define SD_ENOTTY		25				/* Inappropriate ioctl for device */
#define SD_ETXTBSY		26				/* Text file busy */
#define SD_EFBIG		27				/* File too large */
#define SD_ENOSPC		28				/* No space left on device */
#define SD_ESPIPE		29				/* Illegal seek */
#define SD_EROFS		30				/* Read-only file system */
#define SD_EMLINK		31				/* Too many links */
#define SD_EPIPE		32				/* Broken pipe */


//
//	Private string.h s
//
#ifdef SD_OPT_STRING_H
extern void			sd_memcpy(char *d, char *s, char n);
extern signed char	sd_strncmp(char *s1, char *s2, int n);
#else							// redefine standerd library
#define sd_memcpy(d,s,n)		memcpy((void *)(d),(void *)(s),(n))
#define sd_strncmp(s1,s2,n)		strncmp((char *)(s1),(char *)(s2),(n))
#endif

//
//	Convert byte order
//
//extern unsigned long	sd_boconv(unsigned char *buf, char len);						// DEL 2010.07.07
//extern void				sd_bounconv(unsigned char *buf, char len, unsigned long a);	// DEL 2010.07.07
//#ifdef __18CXX																		// DEL 2010.07.07
#if defined(__18CXX) || defined(__AVR__)												// ADD 2010.07.07
#define sd_boconv2(s)		(*((unsigned short *)(s)))
#define sd_boconv4(s)		(*((unsigned long *)(s)))
#define sd_bounconv2(s,a)	(*((unsigned short *)(s)) = (a))
#define sd_bounconv4(s,a)	(*((unsigned long *)(s)) = (a))
#else
extern unsigned long		sd_boconv(unsigned char *buf, char len);					// ADD 2010.07.07
extern void					sd_bounconv(unsigned char *buf, char len, unsigned long a);	// ADD 2010.07.07
#define sd_boconv2(s)		sd_boconv((unsigned char *)(s), sizeof(unsigned short))
#define sd_boconv4(s)		sd_boconv((unsigned char *)(s), sizeof(unsigned long))
#define sd_bounconv2(s,a)	sd_bounconv((unsigned char *)(s), sizeof(unsigned short), (unsigned long)(a))
#define sd_bounconv4(s,a)	sd_bounconv((unsigned char *)(s), sizeof(unsigned long), (unsigned long)(a))
#endif

//
//	Logarithms
//
extern signed char	sd_log2(unsigned short x);


#endif // SD_COMMON_H
