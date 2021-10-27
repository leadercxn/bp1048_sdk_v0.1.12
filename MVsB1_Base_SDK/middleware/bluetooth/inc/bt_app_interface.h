/**
 **************************************************************************************
 * @file    bt_app_interface.h
 * @brief   application interface
 * 			BT中间件函数接口封装
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

typedef enum
{
	//
	MSG_BT_MID_NONE = 0,
	MSG_BT_MID_UART_RX_INT,

	MSG_BT_MID_ACCESS_MODE_IDLE,	//蓝牙进入空闲状态

	//COMMON
	MSG_BT_MID_STACK_INIT,			//蓝牙协议栈初始化完成
	MSG_BT_MID_STATE_CONNECTED,		//蓝牙连接成功状态
	MSG_BT_MID_STATE_DISCONNECT,	//蓝牙断开连接状态
	MSG_BT_MID_STATE_FAST_ENABLE,	//蓝牙初始化完成后状态更新，在开启快速开关机时，需要针对当前模式进行处理

	//A2DP
	MSG_BT_MID_PLAY_STATE_CHANGE,	//播放状态改变
	MSG_BT_MID_VOLUME_CHANGE,		//音量同步
	MSG_BT_MID_STREAM_PAUSE,		//播放暂停

	
	MSG_BT_MID_AVRCP_PANEL_KEY,		

	//HFP
	MSG_BT_MID_HFP_CONNECTED,
	MSG_BT_MID_HFP_PLAY_REMIND,		//通话模式下呼入电话播放来电提示音
	MSG_BT_MID_HFP_PLAY_REMIND_END,	//通话模式下停止播放提示音
	MSG_BT_MID_HFP_CODEC_TYPE_UPDATE,//通话数据格式更新
	MSG_BT_MID_HFP_TASK_RESUME,		//通话模式下恢复通话

	//HFP RCORD
	MSG_BT_MID_HFP_RECORD_MODE_ENTER,//进入通话录音模式
	MSG_BT_MID_HFP_RECORD_MODE_EXIT,//退出通话录音模式
	MSG_BT_MID_HFP_RECORD_MODE_DEREGISTER,//注销通话录音模式
}BtMidMessageId;

//A2DP音频流保存接口
typedef int16_t (*FUNC_SAVE_A2DP_DATA)(uint8_t* Param, uint16_t ParamLen);
void BtAppiFunc_SaveA2dpData(FUNC_SAVE_A2DP_DATA CallbackFunc);
extern FUNC_SAVE_A2DP_DATA SaveA2dpStreamDataToBuffer;

//SBC解码更新
typedef void (*FUNC_REFRESH_SBC_DECODER)(void);
void BtAppiFunc_RefreshSbcDecoder(FUNC_REFRESH_SBC_DECODER CallbackFunc);
extern FUNC_REFRESH_SBC_DECODER RefreshSbcDecoder;


//message发送到APP层
typedef void (*FUNC_MESSAGE_SEND)(BtMidMessageId messageId, uint8_t Param);
void BtAppiFunc_MessageSend(FUNC_MESSAGE_SEND MessageSendFunc);
extern FUNC_MESSAGE_SEND BtMidMessageSend;


//歌曲信息
typedef void (*FUNC_GET_MEDIA_INFO)(void *Param);
void BtAppiFunc_GetMediaInfo(FUNC_GET_MEDIA_INFO CallbackFunc);
extern FUNC_GET_MEDIA_INFO GetMediaInfo;

//hfp sco数据保存接口
typedef int16_t (*FUNC_SAVE_SCO_DATA)(uint8_t* Param, uint16_t ParamLen);
void BtAppiFunc_SaveScoData(FUNC_SAVE_SCO_DATA CallbackFunc);
extern FUNC_SAVE_SCO_DATA SaveHfpScoDataToBuffer;

