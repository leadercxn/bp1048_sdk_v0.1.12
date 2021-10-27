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

#define AppResourceCardForPlay	BIT(10)//旨在登记播卡可用性，插拔注册和注销，播卡扫描和初始化故障时注销

#define AppResourceUDiskForPlay	BIT(11)//旨在登记播U可用性，插拔注册和注销，播U扫描和初始化故障时注销

#define AppResourceFlashFs		BIT(12) //登记flashfs系统可用


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
	AppModeWaitingPlay,//没有输入音源时
	AppModeRestPlay,
	AppModeLineAudioPlay,
	AppModeCardAudioPlay,
	AppModeUDiskAudioPlay,
	AppModeBtAudioPlay,
	AppModeBtHfPlay,
	AppModeBtRecordPlay,//蓝牙K歌录音(基于通话链路)
	AppModeHdmiAudioPlay,
	AppModeUsbDevicePlay,
	AppModeRadioAudioPlay,
	AppModeOpticalAudioPlay,//光纤
	AppModeCoaxialAudioPlay,//同轴
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
 * @brief       根据（插入）Resources，返回使用此资源的最高级别模式，用于后插先播
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

//调试辅助
char* ModeNameStr(AppMode Mode);


#endif /* __MODE_SWITCH_API_H__ */

