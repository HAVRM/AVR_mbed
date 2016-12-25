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

#include "sd_config.h"					// Private


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

