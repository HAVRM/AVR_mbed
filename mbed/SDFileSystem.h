//  Author: hiroki.mine

#ifndef SDFATFILESYSTEM_H_20161202_
#define SDFATFILESYSTEM_H_20161202_

#include "FATFileSystem/diskio.h"
#include "FATFileSystem/ff.h"
#include "FATFileSystem/ffconf.h"
#include "FATFileSystem/integer.h"
#include "mbed.h"
#include <stdarg.h>
#include <stdlib.h>

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

#define CT_MMC		0x01		/* MMC ver 3 */
#define CT_SD1		0x02		/* SD ver 1 */
#define CT_SD2		0x04		/* SD ver 2 */
#define CT_SDC		(CT_SD1|CT_SD2)	/* SD */
#define CT_BLOCK	0x08		/* Block addressing */

#define CTRL_POWER_IDLE		6	/* Put the device idle state */
#define CTRL_POWER_OFF		7	/* Put the device off state */
#define CTRL_UNLOCK			9	/* Unlock media removal */


static volatile DSTATUS Stat[9] = {STA_NOINIT,STA_NOINIT,STA_NOINIT,STA_NOINIT,STA_NOINIT,STA_NOINIT,STA_NOINIT,STA_NOINIT,STA_NOINIT};

int _sd_num=0;
SPI _sd_spi[9];
DigitalOut _sd_cs(PB_6);
BYTE CardType[9];

static void power_on(void){;}
static void power_off(void){;}
static void deselect(BYTE pdrv);
static int select(BYTE pdrv);
static int wait_ready (BYTE pdrv,UINT wt);
static void rcvr_spi_multi(BYTE pdrv,BYTE *p,UINT cnt);
static int rcvr_datablock(BYTE pdrv,BYTE *buff,UINT btr);
static void xmit_spi_multi (BYTE pdrv,const BYTE *p,UINT cnt);
static int xmit_datablock (BYTE pdrv,const BYTE *buff,BYTE token);
BYTE send_cmd(BYTE pdrv,BYTE cmd,DWORD arg);

static void deselect(BYTE pdrv){
  _sd_cs=1;
  _sd_spi[pdrv].write(0xFF);
}

static int select(BYTE pdrv){
  _sd_cs=0;
  _sd_spi[pdrv].write(0xFF);
  if(wait_ready(pdrv,500))return 1;
  deselect(pdrv);
  return 0;
}

static
int wait_ready (BYTE pdrv,UINT wt){ //wt [ms]
  BYTE d;

  int n = wt / 10;
  do{
    d = _sd_spi[pdrv].write(0xFF);
    n--;
  }while (d != 0xFF && n>0);

  return (d == 0xFF) ? 1 : 0;
  /* 1:Ready, 0:Timeout */
}

static void rcvr_spi_multi(BYTE pdrv,BYTE *p,UINT cnt){
  do {
    *p++=_sd_spi[pdrv].write(0xFF);
    *p++=_sd_spi[pdrv].write(0xFF);
  } while (cnt -= 2);
}

static int rcvr_datablock(BYTE pdrv,BYTE *buff,UINT btr){
  BYTE token;

  int n = 20;
  do {
    token = _sd_spi[pdrv].write(0xFF);
  } while ((token == 0xFF) && n>0);
  if (token != 0xFE) return 0;	/* If not valid data token, retutn with error */

  rcvr_spi_multi(pdrv,buff, btr);	/* Receive the data block into buffer */
  _sd_spi[pdrv].write(0xFF);	/* Discard CRC */
  _sd_spi[pdrv].write(0xFF);
  return 1;	/* Return with success */
}

static void xmit_spi_multi (BYTE pdrv,const BYTE *p,UINT cnt){
  do {
    _sd_spi[pdrv].write(*p++);
    _sd_spi[pdrv].write(*p++);
  } while (cnt -= 2);
}

static int xmit_datablock (BYTE pdrv,const BYTE *buff,BYTE token){
  BYTE resp;

  if (!wait_ready(pdrv,500)) return 0;
  _sd_spi[pdrv].write(token);					/* Xmit data token */
  if (token != 0xFD) {	/* Is data token */
    xmit_spi_multi(pdrv,buff, 512);		/* Xmit the data block to the MMC */
    _sd_spi[pdrv].write(0xFF);					/* CRC (Dummy) */
    _sd_spi[pdrv].write(0xFF);
    resp = _sd_spi[pdrv].write(0xFF);			/* Reveive data response */
    if ((resp & 0x1F) != 0x05)		/* If not accepted, return with error */
      return 0;
  }

  return 1;
}

BYTE send_cmd(BYTE pdrv,BYTE cmd,DWORD arg){
  BYTE n,res;

  if(cmd & 0x80){
    cmd &= 0x7F;
    res = send_cmd(pdrv,CMD55,0);
    if(res>1)return res;
  }

  if(cmd != CMD12){
    deselect(pdrv);
    if(!select(pdrv))return 0xFF;
  }

  _sd_cs=0;
  _sd_spi[pdrv].write(0x40 | cmd);
  _sd_spi[pdrv].write((uint8_t)(arg >> 24));
  _sd_spi[pdrv].write((uint8_t)(arg >> 16));
  _sd_spi[pdrv].write((uint8_t)(arg >> 8));
  _sd_spi[pdrv].write((uint8_t)arg);
  n = 0x01;
  if(cmd == CMD0)n = 0x95;
  if(cmd == CMD8)n = 0x87;
  _sd_spi[pdrv].write(n);

  if(cmd == CMD12)_sd_spi[pdrv].write(0xFF);
  n=10;
  for(int i=0;i<n;i++){
    res = _sd_spi[pdrv].write(0xFF);
    if(!(res & 0x80)){
      _sd_cs=1;
      return res;
    }
  }
  _sd_cs=1;
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
  //new(_sd_cs+pdrv) DigitalOut(PB_6);
  BYTE n, cmd, ty, ocr[4],tim;

  power_off();				/* Turn off the socket power to reset the card */
  for (int i=0;i<10;i++)wait_ms(100);		/* Wait for 100ms */
  if (Stat[pdrv] & STA_NODISK) return Stat[pdrv];	/* No card in the socket? */

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
  deselect(pdrv);

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
  BYTE cmd;

  if(!count)return RES_PARERR;
  if(Stat[pdrv] & STA_NOINIT)return RES_NOTRDY;
  if(!(CardType[pdrv] & CT_BLOCK))sector*=512;	/* Convert to byte address if needed */
  cmd = count > 1 ? CMD18 : CMD17;	/*  READ_MULTIPLE_BLOCK : READ_SINGLE_BLOCK */
  if (send_cmd(pdrv,cmd, sector) == 0) {
    do {
      if (!rcvr_datablock(pdrv,buff, 512)) break;
        buff += 512;
    } while (--count);
    if (cmd == CMD18) send_cmd(pdrv,CMD12, 0);	/* STOP_TRANSMISSION */
  }
  deselect(pdrv);

  return count ? RES_ERROR : RES_OK;
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
  if (!count) return RES_PARERR;
  if (Stat[pdrv] & STA_NOINIT) return RES_NOTRDY;
  if (Stat[pdrv] & STA_PROTECT) return RES_WRPRT;

  if (!(CardType[pdrv] & CT_BLOCK)) sector *= 512;	/* Convert to byte address if needed */

  if (count == 1) {	/* Single block write */
    if ((send_cmd(pdrv,CMD24, sector) == 0) && xmit_datablock(pdrv,buff, 0xFE))	/* WRITE_BLOCK */
      count = 0;
  }
  else {				/* Multiple block write */
    if (CardType[pdrv] & CT_SDC) send_cmd(pdrv,ACMD23, count);
    if (send_cmd(pdrv,CMD25, sector) == 0) {	/* WRITE_MULTIPLE_BLOCK */
      do {
        if (!xmit_datablock(pdrv,buff, 0xFC)) break;
        buff += 512;
      } while (--count);
      if (!xmit_datablock(pdrv,0, 0xFD))	/* STOP_TRAN token */
        count = 1;
    }
  }
  deselect(pdrv);
  return count ? RES_ERROR : RES_OK;
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
  BYTE n, csd[16], *ptr = (BYTE*)buff;
  DWORD *dp, st, ed, csize;
#if _USE_ISDIO
  SDIO_CTRL *sdi;
  BYTE rc, *bp;
  UINT dc;
#endif

  if (Stat[pdrv] & STA_NOINIT) return RES_NOTRDY;

  res = RES_ERROR;
  switch (cmd) {
    case CTRL_SYNC :		/* Make sure that no pending write process. Do not remove this or written sector might not left updated. */
      if (select(pdrv)) res = RES_OK;
      deselect(pdrv);
      break;

    case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
      if ((send_cmd(pdrv,CMD9, 0) == 0) && rcvr_datablock(pdrv,csd, 16)) {
        if ((csd[0] >> 6) == 1) {	/* SDC ver 2.00 */
          csize = csd[9] + ((WORD)csd[8] << 8) + ((DWORD)(csd[7] & 63) << 16) + 1;
          *(DWORD*)buff = csize << 10;
        } else {					/* SDC ver 1.XX or MMC*/
          n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
          csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
          *(DWORD*)buff = csize << (n - 9);
        }
        res = RES_OK;
      }
      deselect(pdrv);
      break;

    case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
      if (CardType[pdrv] & CT_SD2) {	/* SDv2? */
        if (send_cmd(pdrv,ACMD13, 0) == 0) {	/* Read SD status */
          _sd_spi[pdrv].write(0xFF);
          if (rcvr_datablock(pdrv,csd, 16)) {				/* Read partial block */
            for (n = 64 - 16; n; n--) _sd_spi[pdrv].write(0xFF);	/* Purge trailing data */
            *(DWORD*)buff = 16UL << (csd[10] >> 4);
            res = RES_OK;
          }
        }
      } else {					/* SDv1 or MMCv3 */
        if ((send_cmd(pdrv,CMD9, 0) == 0) && rcvr_datablock(pdrv,csd, 16)) {	/* Read CSD */
          if (CardType[pdrv] & CT_SD1) {	/* SDv1 */
            *(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
          } else {					/* MMCv3 */
            *(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
          }
          res = RES_OK;
        }
      }
      deselect(pdrv);
      break;

    case CTRL_TRIM:		/* Erase a block of sectors (used when _USE_TRIM in ffconf.h is 1) */
      if (!(CardType[pdrv] & CT_SDC)) break;				/* Check if the card is SDC */
      if (disk_ioctl(pdrv,MMC_GET_CSD, csd)) break;	/* Get CSD */
      if (!(csd[0] >> 6) && !(csd[10] & 0x40)) break;	/* Check if sector erase can be applied to the card */
      dp = (DWORD*)buff; st = dp[0]; ed = dp[1];				/* Load sector block */
      if (!(CardType[pdrv] & CT_BLOCK)) {
        st *= 512; ed *= 512;
      }
      if (send_cmd(pdrv,CMD32, st) == 0 && send_cmd(pdrv,CMD33, ed) == 0 && send_cmd(pdrv,CMD38, 0) == 0 && wait_ready(pdrv,30000))	/* Erase sector block */
      res = RES_OK;	/* FatFs does not check result of this command */
      break;

	/* Following commands are never used by FatFs module */

      case MMC_GET_TYPE :		/* Get card type flags (1 byte) */
        *ptr = CardType[pdrv];
        res = RES_OK;
        break;

      case MMC_GET_CSD :		/* Receive CSD as a data block (16 bytes) */
        if (send_cmd(pdrv,CMD9, 0) == 0 && rcvr_datablock(pdrv,ptr, 16))		/* READ_CSD */
          res = RES_OK;
        deselect(pdrv);
	break;

      case MMC_GET_CID :		/* Receive CID as a data block (16 bytes) */
        if (send_cmd(pdrv,CMD10, 0) == 0 && rcvr_datablock(pdrv,ptr, 16))		/* READ_CID */
          res = RES_OK;
        deselect(pdrv);
        break;

      case MMC_GET_OCR :		/* Receive OCR as an R3 resp (4 bytes) */
        if (send_cmd(pdrv,CMD58, 0) == 0) {	/* READ_OCR */
          for (n = 4; n; n--) *ptr++ = _sd_spi[pdrv].write(0xFF);
          res = RES_OK;
        }
        deselect(pdrv);
        break;

      case MMC_GET_SDSTAT :	/* Receive SD statsu as a data block (64 bytes) */
        if (send_cmd(pdrv,ACMD13, 0) == 0) {	/* SD_STATUS */
          _sd_spi[pdrv].write(0xFF);
          if (rcvr_datablock(pdrv,ptr, 64)) res = RES_OK;
        }
        deselect(pdrv);
        break;

      case CTRL_POWER_OFF :	/* Power off */
        power_off();
        Stat[pdrv] |= STA_NOINIT;
        res = RES_OK;
        break;

#if _USE_ISDIO
      case ISDIO_READ:
        sdi = buff;
        if (send_cmd(pdrv,CMD48, 0x80000000 | (DWORD)sdi->func << 28 | (DWORD)sdi->addr << 9 | ((sdi->ndata - 1) & 0x1FF)) == 0) {
        for (int i = 100; (rc = _sd_spi[pdrv].write(0xFF)) == 0xFF && i>0;i--) ;
        if (rc == 0xFE) {
          for (bp = sdi->data, dc = sdi->ndata; dc; dc--) *bp++ = _sd_spi[pdrv].write(0xFF);
          for (dc = 514 - sdi->ndata; dc; dc--) _sd_spi[pdrv].write(0xFF);
          res = RES_OK;
        }
      }
      deselect(pdrv);
      break;

    case ISDIO_WRITE:
      sdi = buff;
      if (send_cmd(pdrv,CMD49, 0x80000000 | (DWORD)sdi->func << 28 | (DWORD)sdi->addr << 9 | ((sdi->ndata - 1) & 0x1FF)) == 0) {
        _sd_spi[pdrv].write(0xFF); _sd_spi[pdrv].write(0xFE);
        for (bp = sdi->data, dc = sdi->ndata; dc; dc--) _sd_spi[pdrv].write(*bp++);
        for (dc = 514 - sdi->ndata; dc; dc--) _sd_spi[pdrv].write(0xFF);
        if ((_sd_spi[pdrv].write(0xFF) & 0x1F) == 0x05) res = RES_OK;
      }
      deselect(pdrv);
      break;

    case ISDIO_MRITE:
      sdi = buff;
      if (send_cmd(pdrv,CMD49, 0x84000000 | (DWORD)sdi->func << 28 | (DWORD)sdi->addr << 9 | sdi->ndata >> 8) == 0) {
        _sd_spi[pdrv].write(0xFF); _sd_spi[pdrv].write(0xFE); _sd_spi[pdrv].write(sdi->ndata);
        for (dc = 513; dc; dc--) _sd_spi[pdrv].write(0xFF);
        if ((_sd_spi[pdrv].write(0xFF) & 0x1F) == 0x05) res = RES_OK;
      }
      deselect(pdrv);
      break;
#endif

    default:
      res = RES_PARERR;
  }

  return res;
}

#include "FATFileSystem/FATFileSystem.h"

#endif
