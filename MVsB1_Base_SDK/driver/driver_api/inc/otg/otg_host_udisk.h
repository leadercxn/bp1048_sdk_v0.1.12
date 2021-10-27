/**
 *****************************************************************************
 * @file     otg_host_udisk.h
 * @author   Owen
 * @version  V1.0.0
 * @date     7-September-2015
 * @brief    host mass-storage module driver interface
 *****************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2015 MVSilicon </center></h2>
 */
#ifndef __UDISK_H__
#define __UDISK_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 

#include <string.h>
#include "type.h"
#include "timeout.h"
//#include "delay.h"
#include "debug.h"

//#include "otg_detect.h"
#include "otg_host_hcd.h"
#include "otg_host_standard_enum.h"


/**
 * Mass Storage device structure
 */
typedef struct _UDISK
{
	uint8_t		MaxLUN;
	uint8_t		LuNum;
	uint16_t	BlockSize;
	uint32_t	LastLBA;	
	PIPE_INFO	BulkInEp;
	PIPE_INFO	BulkOutEp;
}UDISK;


#pragma pack(1)
/**
 * command block wrapper
 */
typedef struct _CBW
{
	uint32_t	Signature;
	uint32_t	Tag;
	uint32_t	DataTransferLength;
	uint8_t		Flags;
	uint8_t		Lun;
	uint8_t		Length;
	uint8_t		CDB[16];
} CBW;
#pragma pack()


#define US_BULK_CB_SIGN			0x43425355	//spells out USBC
#define US_BULK_FLAG_IN			0x80
#define US_BULK_FLAG_OUT		0x00


/**
 * command status wrapper
 */
typedef struct _CSW
{
	uint32_t	Signature;
	uint32_t	Tag;
	uint32_t	Residue;
	uint8_t		Status;
} CSW;


#define US_BULK_CS_SIGN			0x53425355	// spells out 'USBS'
#define US_BULK_STAT_OK			0
#define US_BULK_STAT_FAIL		1
#define US_BULK_STAT_PHASE		2


// SCSI opcodes
#define TEST_UNIT_READY       0x00
#define REQUEST_SENSE  		  0x03
#define READ_6                0x08
#define WRITE_6               0x0A
#define SEEK_6                0x0B
#define INQUIRY               0x12
#define MODE_SELECT           0x15
#define MODE_SENSE            0x1A
#define START_STOP_UNIT       0x1B
#define ALLOW_MEDIUM_REMOVAL  0x1E
#define	READ_FORMAT_CAPACITY  0x23
#define READ_CAPACITY         0x25
#define READ_10               0x28
#define WRITE_10              0x2A
#define SEEK_10               0x2B
#define VERYFY                0x2F
#define MODE_SELECT_10        0x55
#define MODE_SENSE_10         0x5A
#define READ_12               0xA8
#define WRITE_12              0xAA
#define READ_DISC_STRUCTURE   0xAD
#define DVD_MACHANISM_STATUS  0xBD
#define REPORT_KEY			  0xA4
#define SEND_KEY			  0xA3
#define READ_TOC			  0x43
#define READ_MSF			  0xB9


/**
 * @brief  read blocks
 * @param  BlockNum block number
 * @param  Buf 读缓冲区指针
 * @param  BlockCnt 要读的block个数
 * @return 1-成功，0-失败
 */
bool UDiskReadBlock(uint32_t BlockNum, void* Buf, uint8_t BlockCnt);


/**
 * @brief  write blocks
 * @param  BlockNum block number
 * @param  Buf 写缓冲区指针
 * @param  BlockCnt 要写的block个数
 * @return 1-成功，0-失败
 */
bool UDiskWriteBlock(uint32_t BlockNum, void* Buf, uint8_t BlockCnt);


/**
 * @brief  enum master
 * @param  none
 * @return 1-成功，0-失败
 */
bool UDiskInit(void);


#ifdef __cplusplus
}
#endif // __cplusplus 

#endif //__UDISK_H__

