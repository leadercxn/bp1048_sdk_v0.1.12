/**
 **************************************************************************************
 * @file    bt_app_interface.h
 * @brief   application interface
 * 			BT�м�������ӿڷ�װ
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

	MSG_BT_MID_ACCESS_MODE_IDLE,	//�����������״̬

	//COMMON
	MSG_BT_MID_STACK_INIT,			//����Э��ջ��ʼ�����
	MSG_BT_MID_STATE_CONNECTED,		//�������ӳɹ�״̬
	MSG_BT_MID_STATE_DISCONNECT,	//�����Ͽ�����״̬
	MSG_BT_MID_STATE_FAST_ENABLE,	//������ʼ����ɺ�״̬���£��ڿ������ٿ��ػ�ʱ����Ҫ��Ե�ǰģʽ���д���

	//A2DP
	MSG_BT_MID_PLAY_STATE_CHANGE,	//����״̬�ı�
	MSG_BT_MID_VOLUME_CHANGE,		//����ͬ��
	MSG_BT_MID_STREAM_PAUSE,		//������ͣ

	
	MSG_BT_MID_AVRCP_PANEL_KEY,		

	//HFP
	MSG_BT_MID_HFP_CONNECTED,
	MSG_BT_MID_HFP_PLAY_REMIND,		//ͨ��ģʽ�º���绰����������ʾ��
	MSG_BT_MID_HFP_PLAY_REMIND_END,	//ͨ��ģʽ��ֹͣ������ʾ��
	MSG_BT_MID_HFP_CODEC_TYPE_UPDATE,//ͨ�����ݸ�ʽ����
	MSG_BT_MID_HFP_TASK_RESUME,		//ͨ��ģʽ�»ָ�ͨ��

	//HFP RCORD
	MSG_BT_MID_HFP_RECORD_MODE_ENTER,//����ͨ��¼��ģʽ
	MSG_BT_MID_HFP_RECORD_MODE_EXIT,//�˳�ͨ��¼��ģʽ
	MSG_BT_MID_HFP_RECORD_MODE_DEREGISTER,//ע��ͨ��¼��ģʽ
}BtMidMessageId;

//A2DP��Ƶ������ӿ�
typedef int16_t (*FUNC_SAVE_A2DP_DATA)(uint8_t* Param, uint16_t ParamLen);
void BtAppiFunc_SaveA2dpData(FUNC_SAVE_A2DP_DATA CallbackFunc);
extern FUNC_SAVE_A2DP_DATA SaveA2dpStreamDataToBuffer;

//SBC�������
typedef void (*FUNC_REFRESH_SBC_DECODER)(void);
void BtAppiFunc_RefreshSbcDecoder(FUNC_REFRESH_SBC_DECODER CallbackFunc);
extern FUNC_REFRESH_SBC_DECODER RefreshSbcDecoder;


//message���͵�APP��
typedef void (*FUNC_MESSAGE_SEND)(BtMidMessageId messageId, uint8_t Param);
void BtAppiFunc_MessageSend(FUNC_MESSAGE_SEND MessageSendFunc);
extern FUNC_MESSAGE_SEND BtMidMessageSend;


//������Ϣ
typedef void (*FUNC_GET_MEDIA_INFO)(void *Param);
void BtAppiFunc_GetMediaInfo(FUNC_GET_MEDIA_INFO CallbackFunc);
extern FUNC_GET_MEDIA_INFO GetMediaInfo;

//hfp sco���ݱ���ӿ�
typedef int16_t (*FUNC_SAVE_SCO_DATA)(uint8_t* Param, uint16_t ParamLen);
void BtAppiFunc_SaveScoData(FUNC_SAVE_SCO_DATA CallbackFunc);
extern FUNC_SAVE_SCO_DATA SaveHfpScoDataToBuffer;

