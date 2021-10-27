/**
 **************************************************************************************
 * @file    bt_app_interface.c
 * @brief   application interface
 *			蓝牙middleware和蓝牙播放模式、通话模式交互接口
 *
 * @author  KK
 * @version V1.0.0
 *
 * $Created: 2018-03-22 16:24:11$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include "type.h"
#include "bt_app_interface.h"

FUNC_SAVE_A2DP_DATA SaveA2dpStreamDataToBuffer = NULL;
void BtAppiFunc_SaveA2dpData(FUNC_SAVE_A2DP_DATA CallbackFunc)
{
	SaveA2dpStreamDataToBuffer = CallbackFunc;
}

FUNC_REFRESH_SBC_DECODER RefreshSbcDecoder = NULL;
void BtAppiFunc_RefreshSbcDecoder(FUNC_REFRESH_SBC_DECODER CallbackFunc)
{
	RefreshSbcDecoder = CallbackFunc;
}

FUNC_MESSAGE_SEND	BtMidMessageSend = NULL;
void BtAppiFunc_MessageSend(FUNC_MESSAGE_SEND MessageSendFunc)
{
	BtMidMessageSend = MessageSendFunc;
}

FUNC_GET_MEDIA_INFO GetMediaInfo = NULL;
void BtAppiFunc_GetMediaInfo(FUNC_GET_MEDIA_INFO CallbackFunc)
{
	GetMediaInfo = CallbackFunc;
}

FUNC_SAVE_SCO_DATA SaveHfpScoDataToBuffer = NULL;
void BtAppiFunc_SaveScoData(FUNC_SAVE_SCO_DATA CallbackFunc)
{
	SaveHfpScoDataToBuffer = CallbackFunc;
}

