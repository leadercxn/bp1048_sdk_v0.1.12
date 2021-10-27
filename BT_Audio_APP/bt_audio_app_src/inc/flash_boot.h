#ifndef __FLASH_BOOT_H__
#define __FLASH_BOOT_H__

#include "app_config.h"
#include "flash_config.h"
/*
�汾˵������ǰΪV2.2.0�汾
���ڣ�2021��2��2��
*/
#define FLASH_BOOT_EN      1

//TX PIN
#define BOOT_UART_TX_OFF	0//�رմ���
#define BOOT_UART_TX_A0		1
#define BOOT_UART_TX_A1		2
#define BOOT_UART_TX_A6		3
#define BOOT_UART_TX_A10	4
#define BOOT_UART_TX_A19	5
#define BOOT_UART_TX_A25	6
#define BOOT_UART_TX_PIN	BOOT_UART_TX_OFF

//����������
#define BOOT_UART_BAUD_RATE_9600	0
#define BOOT_UART_BAUD_RATE_11520	1
#define BOOT_UART_BAUD_RATE_256000	2
#define BOOT_UART_BAUD_RATE_512000	3
#define BOOT_UART_BAUD_RATE_1000000	4
#define BOOT_UART_BAUD_RATE_1500000	5
#define BOOT_UART_BAUD_RATE_2000000	6
#define BOOT_UART_BAUD_RATE		BOOT_UART_BAUD_RATE_512000

#define BOOT_UART_CONFIG	((BOOT_UART_BAUD_RATE<<4)+BOOT_UART_TX_PIN)


#define JUDGEMENT_STANDARD		0x55//�˴���ֵ�ָ�4bit���4bit����4bitΪF��code���汾��������Ϊ5��code��CRC����������
									//��4bitΪF��������code��Ҫ�õ����ռ伴�������ռ䣻Ϊ5ʱ���ʶ����codeǰȫ������оƬ���ݣ���������ȫƬ����������flash��ʼ64K�Լ����4K��

///�����ӿڶ���
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

#define UP_PORT				(BTTOOL_OFF + PCTOOL_ON + UDisk_ON + SD_PORT)//����Ӧ�������������Щ�����ӿ�


#if FLASH_BOOT_EN
extern const unsigned char flash_data[];
#endif

#define USER_CODE_RUN_START		0 	//����������ֱ�����пͻ�����
#define UPDAT_OK				1 	//���������������ɹ�
#define NEEDLESS_UPDAT			2	//���������󣬵���������

#endif

