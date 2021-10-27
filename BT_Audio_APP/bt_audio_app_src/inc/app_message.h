/**
 **************************************************************************************
 * @file    app_message.h
 * @brief   
 *
 * @author  halley
 * @version V1.0.0
 *
 * $Created: 2016-6-29 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */


#ifndef __APP_MESSAGE_H__
#define __APP_MESSAGE_H__

#include "type.h"
// Task:����state��MSG; Operation:������State��MSG
/*************************************************************************************
 *
 * Message defines
 *
 */

typedef enum 
{
/** generally */
	MSG_NONE,
/** Require down to task(mode app or service) */
	MSG_TASK_CREATE,
	MSG_TASK_START,
	MSG_TASK_RESUME,		//�ָ�����
	MSG_TASK_PAUSE,			//��ͣ���񣬹��ܸ�λ
	MSG_TASK_STOP,
	MSG_TASK_POWERDOWN,		//PowerKey����Ӧ���·�����Ϣ��taskӦ���д���
	MSG_TASK_DEEPSLEEP,		//��maintaskת��
	MSG_TASK_BTSNIFF,
	MSG_POWERCHAGE_POWERDOWN,		//��ʹ��Powerdown��ʾ��ʹ��
	MSG_APP_CREATED,
	MSG_APP_STARTED,
	MSG_APP_STOPPED,//������Ϣ��һ��
	MSG_APP_EXIT,
	MSG_APP_RES_RELEASE,
	MSG_APP_RES_RELEASE_SUC,//�ͷųɹ�
	MSG_APP_RES_MALLOC,
	MSG_APP_RES_MALLOC_SUC,//����ɹ�
	MSG_APP_RES_INIT,
	MSG_APP_RES_INIT_SUC,//��ʼ���ɹ�
	
/** Main App*/
	MSG_MAINAPP_GROUP					= 0x0100,
/** Operation Msg */	
	MSG_MAINAPP_NEXT_MODE,

/*****************app msg***********/

/** Media Play Mode -Create by mainapp */
	MSG_WAITING_PLAY_MODE_GROUP			= 0x0200,
/** Task msg */
	MSG_WAITING_PLAY_MODE_CREATED,
	MSG_WAITING_PLAY_MODE_STARTED,
	MSG_WAITING_PLAY_MODE_PAUSED,
	MSG_WAITING_PLAY_MODE_STOPPED,
/** Operation Msg */
	MSG_WAITING_PERMISSION_MODE,//�����׸�ģʽ������


/** Media Play Mode -Create by mainapp */
	MSG_MEDIA_PLAY_MODE_GROUP			= 0x0300,
/** Task msg */
	MSG_MEDIA_PLAY_MODE_CREATED,
	MSG_MEDIA_PLAY_MODE_STARTED,
	MSG_MEDIA_PLAY_MODE_PAUSED,
	MSG_MEDIA_PLAY_MODE_STOPPED,
	MSG_MEDIA_PLAY_BROWER_UP,
	MSG_MEDIA_PLAY_BROWER_DN,
	MSG_MEDIA_PLAY_BROWER_ENTER,
	MSG_MEDIA_PLAY_BROWER_RETURN,
	MSG_MEDIA_PLAY_MODE_FAILURE,

/** BT PLAY Mode -Create by mainapp */
	MSG_BT_PLAY_MODE_GROUP				= 0x0400,
/** Task msg */
	MSG_BT_PLAY_MODE_CREATED,
	MSG_BT_PLAY_MODE_STARTED,
	MSG_BT_PLAY_MODE_PAUSED,
	MSG_BT_PLAY_MODE_STOPPED,
/** Operation Msg */
	MSG_BT_PLAY_VOLUME_SET,

/*** from bt callback ***/
	MSG_BT_PLAY_DECODER_START,
	MSG_BT_PLAY_STATE_CHANGED,			//A2DP����״̬�ı�
	MSG_BT_PLAY_SYNC_VOLUME_CHANGED,	//����ͬ��-�����ı�
	MSG_BT_PLAY_STREAM_PASUE,			//A2DP��������ͣ

/*** from bt ai msg ***/
	MSG_DEVICE_SERVICE_AI_ON,
	MSG_DEVICE_SERVICE_AI_OFF, //����Ϣ��һ��device���ƣ�BLE�����ʱ�Կ�

	MSG_BT_STATE_CONNECTED,  //����״̬����,main_task��������ʾ������
	MSG_BT_STATE_DISCONNECT, //�Ͽ�״̬����,main_task��������ʾ������			
	MSG_BT_A2DP_STREAMING,   //��ʼ��������,��ģʽ�л�����,���ո��¼������д���
	
/** BT HF Mode -Create by mainapp */
	MSG_BT_HF_MODE_GROUP				= 0x0500,
/** Task msg */
	MSG_BT_HF_MODE_CREATED,
	MSG_BT_HF_MODE_STARTED,
	MSG_BT_HF_MODE_PAUSED,
	MSG_BT_HF_MODE_STOPPED,
	MSG_BT_HF_MODE_RUN_CONFIG,			//ģʽ���в���ready�󣬿�ʼ��ʼ��������صĲ���
	MSG_BT_HF_MODE_REMIND_PLAY,			//����绰��δ��ͨǰ,������ʾ��
	MSG_BT_HF_MODE_CODEC_UPDATE,		//incoming call��codec type���±ȳ�ʼ��������Ҫ���¶�Ӧ��ͨ·��������Ӧ�ĳ�ʼ�����ݽṹ
/** Operation Msg */
	MSG_BT_HF_TRANS_CHANGED,
	MSG_BT_HF_CALL_REJECT,				//�ܽӵ绰
	MSG_BT_HF_REDAIL_LAST_NUM,			//�ز��������
	MSG_BT_HF_VOICE_RECOGNITION,		//������������

/** BT RECORD Mode -Create by mainapp */
	MSG_BT_RECORD_MODE_GROUP		    = 0x0600,
/** Task msg */
	MSG_BT_RECORD_MODE_CREATED,
	MSG_BT_RECORD_MODE_STARTED,
	MSG_BT_RECORD_MODE_PAUSED,
	MSG_BT_RECORD_MODE_STOPPED,
	
/** Line Mode -Create by mainapp */
	MSG_LINE_AUDIO_MODE_GROUP			= 0x0700,
/** Task msg */
	MSG_LINE_AUDIO_MODE_CREATED,
	MSG_LINE_AUDIO_MODE_STARTED,
	MSG_LINE_AUDIO_MODE_PAUSED,
	MSG_LINE_AUDIO_MODE_STOPPED,

/** Hdmi Mode -Create by mainapp */
	MSG_HDMI_AUDIO_MODE_GROUP			= 0x0800,
/** Task msg */
	MSG_HDMI_AUDIO_MODE_CREATED,
	MSG_HDMI_AUDIO_MODE_STARTED,
	MSG_HDMI_AUDIO_MODE_PAUSED,
	MSG_HDMI_AUDIO_MODE_STOPPED,
/** Operation Msg */
	MSG_HDMI_AUDIO_ARC_ONOFF,

/** Spdif Mode -Create by mainapp */
	MSG_SPDIF_AUDIO_MODE_GROUP			= 0x0900,
/** Task msg */
	MSG_SPDIF_AUDIO_MODE_CREATED,
	MSG_SPDIF_AUDIO_MODE_STARTED,
	MSG_SPDIF_AUDIO_MODE_PAUSED,
	MSG_SPDIF_AUDIO_MODE_STOPPED,

/** Radio Mode -Create by mainapp */
	MSG_RADIO_AUDIO_MODE_GROUP			= 0x0A00,
/** Task msg */
	MSG_RADIO_AUDIO_MODE_CREATED,
	MSG_RADIO_AUDIO_MODE_STARTED,
	MSG_RADIO_AUDIO_MODE_PAUSED,
	MSG_RADIO_AUDIO_MODE_STOPPED,
/**  Operation Msg */
	MSG_RADIO_PLAY_SCAN,
	MSG_RADIO_PLAY_SCAN_UP,
	MSG_RADIO_PLAY_SCAN_DN,
	MSG_RADIO_PLAY_PRE,
	MSG_RADIO_PLAY_NEXT,

/** USB Device PLAY Mode -Create by mainapp */
	MSG_USB_DEVICE_MODE_GROUP			= 0x0B00,
/** Task msg */
	MSG_USB_DEVICE_MODE_CREATED,
	MSG_USB_DEVICE_MODE_STOPPED,
	MSG_USB_DEVICE_MODE_STARTED,
/**  Operation Msg */
	MSG_DEVICE_SERVICE_USB_DEVICE_IN,
	MSG_DEVICE_SERVICE_USB_DEVICE_OUT,

/** Rest Mode -Create by mainapp */
	MSG_REST_PLAY_MODE_GROUP			= 0x0C00,
/** Task msg */
	MSG_REST_PLAY_MODE_CREATED,
	MSG_REST_PLAY_MODE_STARTED,
	MSG_REST_PLAY_MODE_PAUSED,
	MSG_REST_PLAY_MODE_STOPPED,

/** I2SIN Mode -Create by mainapp */
	MSG_I2SIN_AUDIO_MODE_GROUP			= 0x0D00,
/** Task msg */
	MSG_I2SIN_AUDIO_MODE_CREATED,
	MSG_I2SIN_AUDIO_MODE_STARTED,
	MSG_I2SIN_AUDIO_MODE_PAUSED,
	MSG_I2SIN_AUDIO_MODE_STOPPED,

/******************service msg***************/
/** Encoder Service -Create by App */
	MSG_ENCODER_SERVICE_GROUP	        = 0x7700,
/** Task msg */
	MSG_ENCODER_SERVICE_CREATED,
	MSG_ENCODER_SERVICE_STARTED,
	MSG_ENCODER_SERVICE_PAUSED,
	MSG_ENCODER_SERVICE_STOPPED,

/** Recorder Service -Create by app */
	MSG_MEDIA_RECORDER_SERVICE_GROUP	= 0x7800,
/** Task msg */
	MSG_MEDIA_RECORDER_SERVICE_CREATED,
	MSG_MEDIA_RECORDER_SERVICE_STARTED,
	MSG_MEDIA_RECORDER_SERVICE_PAUSED,
	MSG_MEDIA_RECORDER_GO_PAUSED,
	MSG_MEDIA_RECORDER_SERVICE_STOPPED,
/** Operation Msg */
	MSG_MEDIA_RECORDER_RUN,
	MSG_MEDIA_RECORDER_STOP,
	MSG_MEDIA_RECORDER_STOPPED,
	MSG_MEDIA_RECORDER_ERROR,

/** Bluetooth Stack service - Create by maintask or app*/
	MSG_BTSTACK_GROUP					= 0x7900,
	MSG_BTSTACK_RX_INT,		//rx data arrival
	MSG_BTSTACK_BB_ERROR,
	MSG_BTSTACK_SNIFF_STANDBY,
	MSG_BTSTACK_SNIFF_ENTER,
	MSG_BTSTACK_SNIFF_EXIT,
	MSG_BTSTACK_DEEPSLEEP,

/** Audio Core Service -Create by maintask */ 
	MSG_AUDIO_CORE_SERVICE_GROUP		= 0x7A00,
/** Task msg */
	MSG_AUDIO_CORE_SERVICE_CREATED,
	MSG_AUDIO_CORE_SERVICE_STARTED,	
	MSG_AUDIO_CORE_SERVICE_PAUSED,
	MSG_AUDIO_CORE_SERVICE_STOPPED,
	MSG_AUDIO_CORE_HOLD,
	MSG_AUDIO_CORE_FRAME_SIZE_CHANGE,
	MSG_AUDIO_CORE_EFFECT_CHANGE,
/** Operation Msg */

/** Remind sound Service -Create by maintask */
	MSG_REMIND_SOUND_SERVICE_GROUP		= 0x7B00,
/** Task msg */
	MSG_REMIND_SOUND_SERVICE_CREATED,
	MSG_REMIND_SOUND_SERVICE_STARTED, 
	MSG_REMIND_SOUND_SERVICE_PAUSED,
	MSG_REMIND_SOUND_SERVICE_STOPPED,
/** Operation Msg */
	MSG_REMIND_SOUND_NEED_DECODER,
	MSG_REMIND_SOUND_ITEM_END,
	MSG_REMIND_SOUND_PLAY_STOPPED,
	MSG_REMIND_SOUND_PLAY_RESET,
	MSG_REMIND_SOUND_PLAY_END,
	MSG_REMIND_SOUND_PLAY_REQUEST,
	MSG_REMIND_SOUND_PLAY_REQUEST_FAIL,
	MSG_REMIND_SOUND_PLAY,
	MSG_REMIND_SOUND_PLAY_RENEW,
	MSG_REMIND_SOUND_PLAY_START,//һ����ʾ�����ſ�ʼ
	MSG_REMIND_SOUND_PLAY_DONE,//һ����ʾ�����Ž���

/** Decoder Service -Create by mode app */
	MSG_DECODER_SERVICE_GROUP			= 0x7C00,
/** Task msg */
	MSG_DECODER_SERVICE_CREATED,
	MSG_DECODER_SERVICE_STARTED, 
	MSG_DECODER_SERVICE_PAUSED,
	MSG_DECODER_SERVICE_RESUMED,
	MSG_DECODER_SERVICE_STOPPED,
	MSG_DECODER_SERVICE_FREED,
	MSG_DECODER_SERVICE_ERR,
	MSG_DECODER_SERVICE_FIFO_EMPTY,
	MSG_DECODER_SERVICE_SONG_PLAY_END,
	MSG_DECODER_SERVICE_UPDATA_PLAY_TIME,
	MSG_DECODER_SERVICE_DISK_ERROR, //�豸�쳣
/** Operation Msg */
	MSG_DECODER_PLAY,
	MSG_DECODER_PAUSE,
	MSG_DECODER_RESUME,
	MSG_DECODER_STOP,
	MSG_DECODER_FF,
	MSG_DECODER_FB,
	MSG_DECODER_RESET,

	MSG_DECODER_STOPPED,
	MSG_DECODER_PLAYING,
	MSG_DECODER_INITED,

/** Display Service -Create by mainapp */
	MSG_DISPLAY_SERVICE_GROUP			= 0x7E00,
	/** Task msg */
	MSG_DISPLAY_SERVICE_CREATED,
	MSG_DISPLAY_SERVICE_STARTED,
	MSG_DISPLAY_SERVICE_PAUSED,
	MSG_DISPLAY_SERVICE_STOPPED,
	/** Display Msg */
	MSG_DISPLAY_SERVICE_DEV,
	MSG_DISPLAY_SERVICE_BT_UNLINK,
	MSG_DISPLAY_SERVICE_BT_LINKED,
	MSG_DISPLAY_SERVICE_LINE,
	MSG_DISPLAY_SERVICE_RADIO,
	MSG_DISPLAY_SERVICE_MEDIA,
	MSG_DISPLAY_SERVICE_FILE_NUM,
	MSG_DISPLAY_SERVICE_NUMBER,
	MSG_DISPLAY_SERVICE_STATION,
	MSG_DISPLAY_SERVICE_SEARCH_STATION,
	MSG_DISPLAY_SERVICE_VOL,
	MSG_DISPLAY_SERVICE_MUSIC_VOL,
	MSG_DISPLAY_SERVICE_MIC_VOL,
	MSG_DISPLAY_SERVICE_TRE,
	MSG_DISPLAY_SERVICE_BAS,
	MSG_DISPLAY_SERVICE_3D,
	MSG_DISPLAY_SERVICE_VB,
	MSG_DISPLAY_SERVICE_SHUNNING,
	MSG_DISPLAY_SERVICE_VOCAL_CUT,
	MSG_DISPLAY_SERVICE_MUTE,
	MSG_DISPLAY_SERVICE_EQ,
	MSG_DISPLAY_SERVICE_REPEAT,
	MSG_DISPLAY_SERVICE_PLAY,
	MSG_DISPLAY_SERVICE_PAUSE,
	MSG_DISPLAY_SERVICE_RTC_TIME,
	
/** Device Service -Create by mainapp */
	MSG_DEVICE_SERVICE_GROUP			= 0x7F00,
/** Task msg */
	MSG_DEVICE_SERVICE_CREATED,
	MSG_DEVICE_SERVICE_STARTED,
	MSG_DEVICE_SERVICE_PAUSED,
	MSG_DEVICE_SERVICE_STOPPED,
/** Operation Msg */
	MSG_DEVICE_SERVICE_LINE_IN,
	MSG_DEVICE_SERVICE_LINE_OUT,
	MSG_DEVICE_SERVICE_CARD_IN,
	MSG_DEVICE_SERVICE_CARD_OUT,
	MSG_DEVICE_SERVICE_CARD_OUT_JITTER,
	MSG_DEVICE_SERVICE_DISK_IN,
	MSG_DEVICE_SERVICE_DISK_OUT,
	MSG_DEVICE_SERVICE_HDMI_IN,
	MSG_DEVICE_SERVICE_HDMI_OUT,
	//BT CALL MODE
	MSG_DEVICE_SERVICE_BTHF_IN,
	MSG_DEVICE_SERVICE_BTHF_OUT,
	MSG_DEVICE_SERVICE_ENTER_BTHF_MODE,
	MSG_DEVICE_SERVICE_BTRECORD_IN,
	MSG_DEVICE_SERVICE_BTRECORD_OUT,
	//BT PLAY MODE
	MSG_DEVICE_SERVICE_BTPLAY_IN,	
	
	MSG_DEVICE_SERVICE_BATTERY_LOW,
	
	MSG_DEVICE_SERVICE_BP_SYS_INFO,			//�������ϵͳ��Ϣ����
	MSG_DEVICE_SERVICE_BP_PLAYER_INFO,		//�������ý�岥����Ϣ����
	MSG_DEVICE_SERVICE_BP_PLAYER_INFO_2NVM,	//�������ý�岥����Ϣ��NVM����Ҫ����ʱ�䵽S
	MSG_DEVICE_SERVICE_BP_RADIO_INFO,		//�������ý�岥����Ϣ����
	MSG_DEVICE_SERVICE_BP_ALL_INFO,			//�������������Ϣͬʱ����
	
	/** User Com Msg */
	MSG_POWERDOWN,			//PowerKey����Ӧ���·�����Ϣ��taskӦ���д���
	MSG_DEEPSLEEP,			//����ģʽ���ϵ㴦��������
	MSG_SOFT_POWER,         //�ⲿCMOS��·������Ϣ
	MSG_RTC_SET,
	MSG_RTC_UP,
	MSG_RTC_DOWN,	
	MSG_RTC_DISP_TIME,
	MSG_RTC_SET_TIME,
	MSG_RTC_SET_ALARM,
	MSG_RTC_ALARMING,
	MSG_RTC_SNOOZE,
	MSG_REMIND,             //��ʾ������
	MSG_LANG,               //��Ӣ���л�
	MSG_UPDATE,             //MVA����ȷ�ϼ�
	MSG_MODE,
	MSG_POWER,
	MSG_PLAY_PAUSE,
	MSG_STOP,						
	MSG_FF_START,
    MSG_FB_START,
	MSG_FF_FB_END,
    MSG_PRE,
    MSG_NEXT,
    MSG_REPEAT,	            //����ģʽ�л�
    MSG_REPEAT_AB,          //ABѭ��ģʽ
    MSG_FOLDER_MODE,		// �򿪡��ر��ļ��в���ģʽ
    MSG_FOLDER_NEXT,		// ��һ���ļ���
    MSG_FOLDER_PRE,			// ��һ���ļ���
    MSG_BROWSE,			    // �ļ����
    MSG_FOLDER_ERGE,	// 
    MSG_FOLDER_GUSHI,	// 
    MSG_FOLDER_GUOXUE,	//  
    MSG_FOLDER_YINGYU,	// 
    MSG_REC,
    MSG_REC_PLAYBACK,
    MSG_REC_FILE_DEL,	
    MSG_REC_MUSIC,          //�Ƿ������¼��ѡ��
    MSG_MENU,			    //�˵����������Ƶ�
    MSG_NUM_0,	
	MSG_NUM_1,	
	MSG_NUM_2,	
	MSG_NUM_3,	
	MSG_NUM_4,	
	MSG_NUM_5,	
	MSG_NUM_6,	
	MSG_NUM_7,	
	MSG_NUM_8,	
	MSG_NUM_9,	
    MSG_MAIN_VOL_UP,
    MSG_MAIN_VOL_DW,
	MSG_MUSIC_VOLUP,
	MSG_MUSIC_VOLDOWN,
	MSG_MIC_VOLUP,
	MSG_MIC_VOLDOWN,
	MSG_MIC_EFFECT_UP,
	MSG_MIC_EFFECT_DW,
	MSG_MIC_TREB_UP,
	MSG_MIC_TREB_DW,
	MSG_MIC_BASS_UP,
	MSG_MIC_BASS_DW,
	MSG_MUSIC_TREB_UP,
	MSG_MUSIC_TREB_DW,
	MSG_MUSIC_BASS_UP,
	MSG_MUSIC_BASS_DW,
	MSG_PITCH_UP,            //�����
 	MSG_PITCH_DN,            //�����	
	MSG_VOCAL_CUT,	
	MSG_MUTE,
	MSG_EQ,
	MSG_3D,
	MSG_VB,	
	MSG_REMIND1,
 	MSG_EFFECTMODE,
    MSG_MIC_FIRST,     
	MSG_BT_AI,
	MSG_BT_XM_AI_START,
	MSG_BT_XM_AI_STOP,
	MSG_BT_CONNECT_CTRL,     //�ֶ�����/�Ͽ�����
	MSG_BT_RST,              //����reset���ָ���������
	MSG_BT_ENTER_DUT_MODE,   //����DUT����ģʽ
	
/******************adc level ,Sliding resistance msg, Reservations***************/
    MSG_ADC_LEVEL_MSG_START       = 0x9000,
    MSG_ADC_LEVEL_CH1             = 0x9100,
    MSG_ADC_LEVEL_CH2             = 0x9200,
    MSG_ADC_LEVEL_CH3             = 0x9300,
	MSG_ADC_LEVEL_CH4             = 0x9400,
	MSG_ADC_LEVEL_CH5             = 0x9500,
	MSG_ADC_LEVEL_CH6             = 0x9600,
	MSG_ADC_LEVEL_CH7             = 0x9700,
	MSG_ADC_LEVEL_CH8             = 0x9800,
	MSG_ADC_LEVEL_CH9             = 0x9900,
	MSG_ADC_LEVEL_CH10            = 0x9a00,
	MSG_ADC_LEVEL_CH11            = 0x9b00,
	MSG_ADC_LEVEL_CH12            = 0x9c00,
	MSG_ADC_LEVEL_CH13            = 0x9d00,
	MSG_ADC_LEVEL_CH14            = 0x9e00,
	MSG_ADC_LEVEL_MSG_END         = 0x9f00,   
} MessageId;

#define MAX_RECV_MSG_TIMEOUT		0xFFFFFFFF


/*************************************************************************************
 *
 * Module Message defines
 *
 *************************************************************************************/

typedef enum
{
	TaskStateNone	= 0,
	TaskStateCreating,
	TaskStateReady, // TaskStateCreated
	TaskStateStarting,
	TaskStateRunning, // TaskStateStarted
	TaskStatePausing,
	TaskStatePaused,
	TaskStateResuming,
	TaskStateStopping,
	TaskStateStopped,
	TaskStateError,
}TaskState;

#endif /*__APP_MESSAGE_H__*/

