#ifndef __FLASH_BOOT_H__
#define __FLASH_BOOT_H__

#include "app_config.h"
#include "flash_config.h"
/*
版本说明：当前为V2.2.0版本
日期：2021年2月2日
*/
#define FLASH_BOOT_EN      1

//TX PIN
#define BOOT_UART_TX_OFF	0//关闭串口
#define BOOT_UART_TX_A0		1
#define BOOT_UART_TX_A1		2
#define BOOT_UART_TX_A6		3
#define BOOT_UART_TX_A10	4
#define BOOT_UART_TX_A19	5
#define BOOT_UART_TX_A25	6
#define BOOT_UART_TX_PIN	BOOT_UART_TX_OFF

//波特率配置
#define BOOT_UART_BAUD_RATE_9600	0
#define BOOT_UART_BAUD_RATE_11520	1
#define BOOT_UART_BAUD_RATE_256000	2
#define BOOT_UART_BAUD_RATE_512000	3
#define BOOT_UART_BAUD_RATE_1000000	4
#define BOOT_UART_BAUD_RATE_1500000	5
#define BOOT_UART_BAUD_RATE_2000000	6
#define BOOT_UART_BAUD_RATE		BOOT_UART_BAUD_RATE_512000

#define BOOT_UART_CONFIG	((BOOT_UART_BAUD_RATE<<4)+BOOT_UART_TX_PIN)


#define JUDGEMENT_STANDARD		0x55//此处数值分高4bit与低4bit：高4bit为F则code按版本号升级；为5则按code的CRC进行升级；
									//低4bit为F则在升级code需要用到多大空间即擦除多大空间；为5时则标识升级code前全部擦除芯片数据，即擦除“全片”（即除开flash开始64K以及最后4K）

///升级接口定义
#define	SD_OFF				0x00
#define SD_A15A16A17		0x1
#define SD_A20A21A22		0x2
#if CFG_RES_CARD_GPIO == SDIO_A15_A16_A17
#define SD_PORT				SD_A15A16A17
#else
#define SD_PORT				SD_A20A21A22
#endif

#define UDisk_OFF			0x00
#define UDisk_ON			0x4

#define PCTOOL_OFF			0x00
#define PCTOOL_ON			0x08

#define	BTTOOL_OFF			0X00
#define BTTOOL_ON			0X10

#define UP_PORT				(BTTOOL_OFF + PCTOOL_ON + UDisk_ON + SD_PORT)//根据应用情况决定打开那些升级接口


#if FLASH_BOOT_EN
extern const unsigned char flash_data[];
#endif

#define USER_CODE_RUN_START		0 	//无升级请求直接运行客户代码
#define UPDAT_OK				1 	//有升级请求，升级成功
#define NEEDLESS_UPDAT			2	//有升级请求，但无需升级

#endif

