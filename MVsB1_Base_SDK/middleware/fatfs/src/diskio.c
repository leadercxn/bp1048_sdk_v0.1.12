/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/
#include "type.h"
#include "integer.h"
#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* FatFs lower layer API */

#include "sd_card.h"

#include "otg_host_udisk.h"//需添加宏开关

/* Definitions of physical drive number for each drive */
#define DEV_RAM		2	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		0	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		1	/* Example: Map USB MSD to physical drive 2 */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive number to identify the drive */
)
{
	DSTATUS stat = STA_NOINIT;
//	int result;

	switch (pdrv) {
	case DEV_RAM :
		//result = RAM_disk_status();

		// translate the result code here

		return stat;

	case DEV_MMC :
		stat = RES_OK;
		return stat;

	case DEV_USB :
		//result = USB_disk_status();

		// translate the result code here
		stat = RES_OK;

		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive number to identify the drive */
)
{
	DSTATUS stat = RES_ERROR;
//	int result;

	switch (pdrv) {
	case DEV_RAM :
		return stat;

	case DEV_MMC :
		stat = RES_OK;
 		return stat;

	case DEV_USB :
		stat = RES_OK;
 		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive number to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res = RES_ERROR;
//	int result;

	switch (pdrv) {
	case DEV_RAM :
		// translate the arguments here

		// translate the result code here

		return res;

	case DEV_MMC :
		if(!SDCard_ReadBlock(sector, buff, count))
		{
			res = RES_OK;
		}

		return res;

	case DEV_USB :
		if(UDiskReadBlock(sector, buff, count))
			res = RES_OK;
 		return res;
		// translate the result code here
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive number to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res = RES_ERROR;
//	int result;

	switch (pdrv) {
	case DEV_RAM :
		// translate the arguments here


		// translate the result code here

		return res;

	case DEV_MMC :
		if(!SDCard_WriteBlock(sector, (uint8_t *)buff, count))
		{
			res = RES_OK;
		}
		return res;

	case DEV_USB :
		res = UDiskWriteBlock(sector, (void*)buff, count) ? RES_OK : RES_ERROR;
		// translate the result code here
		return res;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive number (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res = RES_OK;
//	int result;

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

DWORD get_fattime (void)
{
    return ((DWORD)(2010 - 1980) << 25) | ((DWORD)1 << 16) | ((DWORD)0 << 11) | ((DWORD)0 << 5) | ((DWORD)0 >> 1);
}

