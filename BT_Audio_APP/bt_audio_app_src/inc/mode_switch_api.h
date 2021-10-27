/**
 **************************************************************************************
 * @file    mode_switch_api.h
 * @brief   mode switch api
 *
 * @author  halley
 * @version V1.0.0
 *
 * $Created: 2016-6-29 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#ifndef __MODE_SWITCH_API_H__
#define __MODE_SWITCH_API_H__

#include "type.h"
#include "rtos_api.h"

//Bit	Mask   (max 32)//for app & resource

#define AppResourceDac			BIT(0)

#define AppResourceLineIn		BIT(1)

#define AppResourceCard			BIT(2)

#define AppResourceUDisk		BIT(3)

#define AppResourceBtPlay		BIT(4)

#define AppResourceHdmiIn		BIT(5)

#define AppResourceUsbDevice	BIT(6)

#define AppResourceRadio 		BIT(7)

#define AppResourceSpdif	    BIT(8)

#define AppResourceI2SIn	    BIT(9)

#define AppResourceCardForPlay	BIT(10)//ּ�ڵǼǲ��������ԣ����ע���ע��������ɨ��ͳ�ʼ������ʱע��

#define AppResourceUDiskForPlay	BIT(11)//ּ�ڵǼǲ�U�����ԣ����ע���ע������Uɨ��ͳ�ʼ������ʱע��

#define AppResourceFlashFs		BIT(12) //�Ǽ�flashfsϵͳ����


#define AppResourceBtHf			BIT(13)

#define AppResourceBtRecord		BIT(14)

#define AppResourceRest			BIT(15)

#define AppResourceMask			0xFFFFFFFF


void ResourceRegister(uint32_t Resources);
void ResourceDeregister(uint32_t Resources);
uint32_t ResourceValue(uint32_t Resources);


typedef enum
{
	AppModeIdle				= 0,
	AppModeWaitingPlay,//û��������Դʱ
	AppModeRestPlay,
	AppModeLineAudioPlay,
	AppModeCardAudioPlay,
	AppModeUDiskAudioPlay,
	AppModeBtAudioPlay,
	AppModeBtHfPlay,
	AppModeBtRecordPlay,//����K��¼��(����ͨ����·)
	AppModeHdmiAudioPlay,
	AppModeUsbDevicePlay,
	AppModeRadioAudioPlay,
	AppModeOpticalAudioPlay,//����
	AppModeCoaxialAudioPlay,//ͬ��
	AppModeI2SInAudioPlay,
	AppModeCardPlayBack,
	AppModeUDiskPlayBack,	
	AppModeFlashFsPlayBack,
}AppMode;

typedef enum
{
	ModeCreate				= 0,
	ModeStart,
	ModeStop,
	ModeKill,
}ModeStateAct;

void ResourcePreSet(void);

bool ModeResumeMask(AppMode Mode);

int32_t ModeStateSet(ModeStateAct ActionAct);

MessageHandle GetAppMessageHandle(void);

uint16_t AppStateMsgAck(uint16_t Msg);

uint8_t	 GetAppDecoderChannel(void);


AppMode FindPriorityMode(void);

/**
 * @func        FindResourceMode
 * @brief       ���ݣ����룩Resources������ʹ�ô���Դ����߼���ģʽ�����ں���Ȳ�
 * @param       uint32_t Resources  
 * @Output      None
 * @return      AppMode
 * @Others      
 * Record
 */
AppMode FindResourceMode(uint32_t Resources);

AppMode NextAppMode(AppMode mode);
bool CheckModeResource(AppMode mode);

AppMode FindModeByPlugInMsg(uint16_t Msg);

//���Ը���
char* ModeNameStr(AppMode Mode);


#endif /* __MODE_SWITCH_API_H__ */

