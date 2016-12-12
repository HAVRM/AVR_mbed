//  Author: hiroki.mine

#ifndef SDFATFILESYSTEM_H_20161202_
#define SDFATFILESYSTEM_H_20161202_

#include "FatFileSystem/FatFileSystem.h"
#include "mbed.h"

//diskio.c file is under here

/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

/* Definitions for MMC/SDC command */
#define CMD0	(0)			/* GO_IDLE_STATE */
#define CMD1	(1)			/* SEND_OP_COND (MMC) */
#define	ACMD41	(0x80+41)	/* SEND_OP_COND (SDC) */
#define CMD8	(8)			/* SEND_IF_COND */
#define CMD9	(9)			/* SEND_CSD */
#define CMD10	(10)		/* SEND_CID */
#define CMD12	(12)		/* STOP_TRANSMISSION */
#define ACMD13	(0x80+13)	/* SD_STATUS (SDC) */
#define CMD16	(16)		/* SET_BLOCKLEN */
#define CMD17	(17)		/* READ_SINGLE_BLOCK */
#define CMD18	(18)		/* READ_MULTIPLE_BLOCK */
#define CMD23	(23)		/* SET_BLOCK_COUNT (MMC) */
#define	ACMD23	(0x80+23)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24	(24)		/* WRITE_BLOCK */
#define CMD25	(25)		/* WRITE_MULTIPLE_BLOCK */
#define CMD32	(32)		/* ERASE_ER_BLK_START */
#define CMD33	(33)		/* ERASE_ER_BLK_END */
#define CMD38	(38)		/* ERASE */
#define	CMD48	(48)		/* READ_EXTR_SINGLE */
#define	CMD49	(49)		/* WRITE_EXTR_SINGLE */
#define CMD55	(55)		/* APP_CMD */
#define CMD58	(58)		/* READ_OCR */

#define R1_IDLE_STATE           (1 << 0)
#define R1_ERASE_RESET          (1 << 1)
#define R1_ILLEGAL_COMMAND      (1 << 2)
#define R1_COM_CRC_ERROR        (1 << 3)
#define R1_ERASE_SEQUENCE_ERROR (1 << 4)
#define R1_ADDRESS_ERROR        (1 << 5)
#define R1_PARAMETER_ERROR      (1 << 6)

#define SDCARD_FAIL 0
#define SDCARD_V1   1
#define SDCARD_V2   2
#define SDCARD_V2HC 3

static volatile
DSTATUS Stat[9] = {STA_NOINIT,STA_NOINIT,STA_NOINIT,STA_NOINIT,STA_NOINIT,STA_NOINIT,STA_NOINIT,STA_NOINIT,STA_NOINIT};

int _sd_num=0;
SPI _sd_spi[9];
DigitalOut _sd_cs[9];
BYTE CardType[9];

deselect()
select()

BYTE send_cmd(BYTE pdrv,BYTE cmd,DWORD arg){
  BYTE n,res;

  if(cmd & 0x80){
    cmd &= 0x7F;
    res = send_cmd(pdrv,CMD55,0);
    if(res>1)return res;
  }

  if(cmd != CMD12){
    deselect();
    if(!select())return 0xFF;
  }

  _sd_cs[pdrv]=0;
  _sd_spi[pdrv].write(0x40 | cmd);
  _sd_spi[pdrv].write((uint8_t)(arg >> 24));
  _sd_spi[pdrv].write((uint8_t)(arg >> 16));
  _sd_spi[pdrv].write((uint8_t)(arg >> 8));
  _sd_spi[pdrv].write((uint8_t)arg);
  n = 0x01;
  if(cmd == CMD0)n = 0x95;
  if(cmd == CMD8)n = 0x87;
  _sd_spi.write[pdrv](n);

  if(cmd == CMD12)_sd_spi[pdrv].write(0xFF);
  n=10;
  for(int i=0;i<n;i++){
    res = _sd_spi[pdrv].write(0xFF);
    if(!(res & 0x80)){
      _sd_cs[pdrv]=1;
      return res;
    }
  }
  _sd_cs[pdrv]=1;
  return res;
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	return Stat[pdrv];
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
  new(_sd_cs+pdrv) DigitalOut(PB_6);
  BYTE n, cmd, ty, ocr[4],tim;

  power_off();				/* Turn off the socket power to reset the card */
  for (int i=0;i<10;i++)wait_ms(100);		/* Wait for 100ms */
  if (Stat & STA_NODISK) return Stat[pdrv];	/* No card in the socket? */

  power_on();				/* Turn on the socket power */
  _sd_spi[pdrv].frequency(100000);
  for (int i=0;i<10;i++) _sd_spi[pdrv].write(0xFF);	/* 80 dummy clocks */

  ty = 0;
  if (send_cmd(pdrv,CMD0, 0) == 1) {		/* Put the card SPI mode */
    if (send_cmd(pdrv,CMD8, 0x1AA) == 1) {	/* Is the card SDv2? */
      for (n = 0; n < 4; n++) ocr[n] = _sd_spi[pdrv].write(0xFF);	/* Get trailing return value of R7 resp */
      if (ocr[2] == 0x01 && ocr[3] == 0xAA) {	/* The card can work at vdd range of 2.7-3.6V */
        tim=100;
        while (tim && send_cmd(pdrv,ACMD41, 1UL << 30))tim--;	/* Wait for leaving idle state (ACMD41 with HCS bit) */
        if (tim && send_cmd(pdrv,CMD58, 0) == 0) {		/* Check CCS bit in the OCR */
          for (n = 0; n < 4; n++) ocr[n] = _sd_spi[pdrv].write(0xFF);
          ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;	/* Check if the card is SDv2 */
        }
      }
    } else {							/* SDv1 or MMCv3 */
      if (send_cmd(pdrv,ACMD41, 0) <= 1) 	{
        ty = CT_SD1; cmd = ACMD41;	/* SDv1 */
      } else {
        ty = CT_MMC; cmd = CMD1;	/* MMCv3 */
      }
      tim=100;
      while (tim && send_cmd(pdrv,cmd, 0))tim--;			/* Wait for leaving idle state */
      if (!tim || send_cmd(pdrv,CMD16, 512) != 0)ty = 0;	/* Set R/W block length to 512 */
    }
  }
  CardType[pdrv] = ty;
  deselect();

  if (ty) {			/* Initialization succeded */
    Stat[pdrv] &= ~STA_NOINIT;		/* Clear STA_NOINIT */
    _sd_spi[pdrv].frequency(1000000);
  } else {			/* Initialization failed */
    power_off();
  }

  return Stat[pdrv];
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_RAM :
		// translate the arguments here

		result = RAM_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_MMC :
		// translate the arguments here

		result = MMC_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_USB :
		// translate the arguments here

		result = USB_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_RAM :
		// translate the arguments here

		result = RAM_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_MMC :
		// translate the arguments here

		result = MMC_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_USB :
		// translate the arguments here

		result = USB_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_RAM :

		// Process of the command for the RAM drive

		return res;

	case DEV_MMC :

		// Process of the command for the MMC/SD card

		return res;

	case DEV_USB :

		// Process of the command the USB drive

		return res;
	}

	return RES_PARERR;
}




#endif
