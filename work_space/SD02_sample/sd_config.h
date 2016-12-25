//
//  sd_config.h
//
//  SDcard control module mini - Configure file
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
//	2010.02.19 v0.1b Bug fix, do not use printf()
//	2010.07.07 v0.2  Support ATmega328P
//

#ifndef SD_CONFIG_H
#define SD_CONFIG_H


//
//	Option
//
//#define SD_OPT_BUF256					// ImageCraft compiler/PSoC limit is 256
//#define SD_OPT_STRING_H				// Use private string.h for PSoC-libc
#define SD_OPT_CRC						// Enable CRC
//#define SD_OPT_ARGCHECK				// Logical check
//#define SD_OPT_PARTLIMCHK				// Partition border check
#define SD_OPT_GETINFO					// support sd_L1_getinfo()
#define SD_OPT_WRITE					// support sd_write()
#define SD_OPT_UNLINK					// support sd_unlink()
#define SD_OPT_DIR						// support sd_dir*()
//#define SD_OPT_FORCECLOSE				// Close and unmount


//
//	Device config
//
// PIC18: SPI Clock divide
//#define SD_L1_SPICLKDIV	SPI_FOSC_4		// Fosc/4
//#define SD_L1_SPICLKDIV	SPI_FOSC_16		// Fosc/16
//#define SD_L1_SPICLKDIV	SPI_FOSC_64		// Fosc/64
//#define SD_L1_SPICLKDIV	SPI_FOSC_TMR2	// Timer2

// ATmega: SPI Clock divide
//#define SD_L1_SPICLKDIV		0					// clk(I/O)/2
  #define SD_L1_SPICLKDIV		_BV(SPR0)			// clk(I/O)/8
//#define SD_L1_SPICLKDIV		_BV(SPR1)			// clk(I/O)/32
//#define SD_L1_SPICLKDIV		_BV(SPR0)|_BV(SPR1)	// clk(I/O)/64



//
//	DEBUG
//
//#ifndef UDEBUG						// DEL 2010.02.19  Disable DEBUG
//#define printf(a,...) 				// DEL 2010.02.19
//#define i2c_lcd_position(a,b)			// DEL 2010.02.19
//#define i2c_lcd_prCString(a)			// DEL 2010.02.19
//#define i2c_lcd_prHexByte(a)			// DEL 2010.02.19
//#define i2c_lcd_prHexInt(a)			// DEL 2010.02.19
//#endif								// DEL 2010.02.19

#endif // SD_CONFIG_H
