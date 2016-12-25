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

#include "delay.h"

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
