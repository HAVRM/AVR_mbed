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
//  Device: CY8C29466/27443  IDE: PSoC Designer 5 Service Pack 6(Build 1127)
//  Device: MSP430F2013      IDE: IAR Embedded Workbench for MSP430 V4.11B
//  Device: PIC18F14K50      IDE: MPLAB IDE 8.36, C Compiler for PIC18 MCUs v3.33
//  Device: ATmega328        IDE: AVR Studio 4.18.700, WinAVR-20100110(GCC 4.3.3)
//
//    Update history
//    ---------- ----- -------------------------------------------
//    2009.12.22 v0.0  First cording
//    2010.07.07 v0.1  Support ATmega328P
//

#ifndef DELAY_H
#define DELAY_H

//
//    Configure
//        (Need modify real device)
//
//#define DELAY_M8C
//#define DELAY_MSP430	(50 * 8)		// MCLK=8MHz
//#define DELAY_PIC18	(60)			// FOSC=48MHz
#define DELAY_AVR		(1000000UL)		// clkCPU=1MHz


//
//    Macro
//
// delay_50u()
//-------------------------------------------------------
#ifdef DELAY_M8C
#include <m8c.h>
#include <GlobalParams.h>
#if (CPU_CLOCK == 0)
#define DELAY_WAITCOUNT 7
#endif
#if (CPU_CLOCK == 1)
#define DELAY_WAITCOUNT 14
#endif
#if (CPU_CLOCK == 2)
#define DELAY_WAITCOUNT 27
#endif
#if (CPU_CLOCK == 3)
#define DELAY_WAITCOUNT 54
#endif
#if (CPU_CLOCK == 4)
#define DELAY_WAITCOUNT 3
#endif
#if (CPU_CLOCK == 5)
#define DELAY_WAITCOUNT 2
#endif
#if (CPU_CLOCK == 6)
#define DELAY_WAITCOUNT 1
#endif
#if (CPU_CLOCK == 7)
#define DELAY_WAITCOUNT 1
#endif
#define delay_50u()    {char i; for(i = 0; i < DELAY_WAITCOUNT; i++);}
#endif  // DELAY_M8C
//-------------------------------------------------------
#ifdef DELAY_MSP430
#include "msp430.h"
#define delay_50u()		__delay_cycles(DELAY_MSP430)
#endif  // DELAY_MSP430
//-------------------------------------------------------
#ifdef DELAY_PIC18
#include <delays.h>
#define const  const rom
#define delay_50u()		Delay10TCYx(DELAY_PIC18)
#endif  // DELAY_PIC18
//-------------------------------------------------------
#ifdef DELAY_AVR
#define F_CPU 	DELAY_AVR
#include <util/delay.h>
#define delay_50u()		_delay_us(50)
#endif
//-------------------------------------------------------


//
//    A P I
//
extern void delay_50uTimes(unsigned char bTimes);
extern void delay_1mTimes(unsigned char bTimes);

#endif // DELAY_H
