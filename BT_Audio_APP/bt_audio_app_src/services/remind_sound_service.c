/**
 **************************************************************************************
 * @file    remind_sound_service.c
 * @brief   
 *
 * @author  pi
 * @version V1.0.0
 *
 * $Created: 2018-2-27 13:06:47$
 *
 * @Copyright (C) 2018, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <string.h>
#include "app_config.h"
#include "rtos_api.h"
#include "app_message.h"
#include "type.h"
#include "spi_flash.h"
#include "debug.h"
#include "audio_utility.h"
#include "remind_sound_service.h"
#include "audio_core_api.h"
#include "decoder_service.h"
#include "device_service.h"
#include "audio_core_service.h"
#include "mcu_circular_buf.h"
#include "main_task.h"
#include "dac_interface.h"
#include "timeout.h"
#include "bt_manager.h"
#include "ctrlvars.h"

#ifdef CFG_FUNC_REMIND_SOUND_EN

#pragma pack(1)
typedef struct _SongClipsHdr
{
	char sync[4];
	uint32_t crc;
	uint8_t cnt;
} SongClipsHdr;
#pragma pack()

#pragma pack(1)
typedef struct _SongClipsEntry
{
	uint8_t id[8];
	uint32_t offset;
	uint32_t size;
} SongClipsEntry;
#pragma pack()

typedef enum _REMIND_SOUND_STATE
{
	REMIND_STANDBY,
	REMIND_WAIT_DECODER,
	REMIND_PLAY,
	REMIND_STOPPING,
	REMIND_STOPPED,//�ȴ�fifo���ݲ�
} REMINDSOUNDSTATE;

#define		REMIND_SOUND_ID_LEN			sizeof(((SongClipsEntry *)0)->id)


/***************************************************************************************
 *
 * External defines
 *
 */
#define REDMIN_SOUND_SERVICE_WAIT_PLAY_END			10
#define REMIND_SOUND_SERVICE_TASK_STACK_SIZE		512//1024
#define REMIND_SOUND_SERVICE_TASK_PRIO				3



/***************************************************************************************
 *
 * Internal defines
 *
 */

#define REMIND_SOUND_SERVICE_AUDIO_DECODER_IN_BUF_SIZE	1024 * 19
//Ӧ���У����������������Ŵ����Ҫһ����Ӧ���̣�����ʽ���ź͵Ǽ��ڼ䲻��Ӧ ����������͵Ǽǣ���ʾ������ͨ�� ʵʩ����ʱ����λ�����������뵱ǰλ״̬��
//�����������ź�ֻ����ʾ�������Լ���������������ǣ�һ��״̬���󣬿��������������ʾ������һֱʧ�ܣ�ֻ����ʾ��reset�ſɻָ���
//����Ӧ�ã��绰����������������������������app��ⲥ������Ϊ��ʱ���������ʾ�����ţ���ʾ�������ṩ���api����Ϣ����״̬�����App���̼���
typedef struct _RemindSoundServiceContext
{
	xTaskHandle 		taskHandle;
	MessageHandle		msgHandle;
	MessageHandle		parentMsgHandle;
	TaskState			RemindSoundServiceState;

	MemHandle			RemindMemHandle;
	
	uint32_t 			ConstDataAddr;
	uint32_t			ConstDataSize;
	uint32_t 			ConstDataOffset;
	uint8_t				RemindBlockBuf[REMIND_SOUND_ID_LEN * CFG_PARAM_REMIND_LIST_MAX];//������ʾ������
	MCU_CIRCULAR_CONTEXT	RemindBlockCircular;
	uint8_t				RequestRemind[REMIND_SOUND_ID_LEN];//������������ʾ����Ŀ
	REMINDSOUNDSTATE	RemindState;
	bool				IsBlock; //TRUE: ��ǰ�������� ��FALSE:��ǰ������
}RemindSoundServiceContext;

#define REMIND_FLASH_MAX_NUM		255 //flash��ʾ�������þ���
#define REMIND_FLASH_HDR_SIZE		0x1000 //��ʾ����Ŀ��Ϣ����С

#define REMIND_FLASH_READ_TIMEOUT 	100
#define REMIND_FLASH_ADDR(n) 		(REMIND_FLASH_STORE_BASE + sizeof(SongClipsEntry) * n + sizeof(SongClipsHdr))//flash��ʾ�������þ���

#define REMIND_FLASH_FLAG_STR		("MVUB")

static const unsigned short CrcCCITTTable[256] =
{
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
	0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
	0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
	0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
	0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
	0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
	0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
	0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
	0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
	0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
	0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
	0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
	0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
	0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
	0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
	0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
	0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
	0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
	0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
	0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
	0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
	0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
	0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};



/***************************************************************************************
 *
 * Internal varibles
 *
 */

#define REMIND_SOUND_SERVICE_NUM_MESSAGE_QUEUE		15

static RemindSoundServiceContext		RemindSoundServiceCt;

/** remind sound task name*/
const char RemindSoundServiceName[] = "RemindSoundService";

/***************************************************************************************
 *
 * Internal functions
 *
 */

/////////////////////////////////////////////////////////////////////////
uint32_t RemindSoundServiceFillStreamCallback(void *buffer, uint32_t length);


void RemindSoundReinit(void)
{
	RemindSoundServiceCt.RemindMemHandle.addr = NULL;//����Callback�� ʵ�ʴ˽ṹ��δ��������ʹ�á�����api������
	RemindSoundServiceCt.RemindMemHandle.mem_capacity = 0;
	RemindSoundServiceCt.RemindMemHandle.mem_len = 0;
	RemindSoundServiceCt.RemindMemHandle.p = 0;
	RemindSoundServiceCt.RemindState = REMIND_STANDBY;
	AudioCoreSourceDisable(REMIND_SOURCE_NUM);
	RemindSoundServiceCt.RequestRemind[0] = 0;
	RemindSoundServiceCt.IsBlock = 0;
	MCUCircular_Config(&RemindSoundServiceCt.RemindBlockCircular, RemindSoundServiceCt.RemindBlockBuf, REMIND_SOUND_ID_LEN * CFG_PARAM_REMIND_LIST_MAX);
}


static int32_t RemindSoundServiceInit(MessageHandle parentMsgHandle)
{
	memset(&RemindSoundServiceCt, 0, sizeof(RemindSoundServiceCt));
//����const data��ȫ�������Ҫ�����
	if(!sound_clips_all_crc())
	{
		SoftFlagRegister(SoftFlagNoRemind);//task����Ҫ����������maintask�޷�������
	}

//��ֻ���ͷ�����
//	SongClipsHdr	ConstDataHead;
//	if(FLASH_NONE_ERR != SpiFlashRead(REMIND_FLASH_STORE_BASE, (uint8_t*)&ConstDataHead,sizeof(SongClipsHdr), REMIND_FLASH_READ_TIMEOUT))
//	{
//		return -1;
//	}
//	if(memcmp(ConstDataHead.sync, REMIND_FLASH_FLAG_STR, sizeof(ConstDataHead.sync)) || !ConstDataHead.cnt)
//	{
//		SoftFlagRegister(SoftFlagNoRemind); //��ʾ������û��¼��
//	}

//	APP_DBG("%d remind items @ %x in flash\n", ConstDataHead.cnt, REMIND_FLASH_STORE_BASE);

	/* message handle */
	RemindSoundServiceCt.msgHandle = MessageRegister(REMIND_SOUND_SERVICE_NUM_MESSAGE_QUEUE);

	/* Parent message handle */
	RemindSoundServiceCt.parentMsgHandle = parentMsgHandle;
	RemindSoundServiceCt.RemindSoundServiceState = TaskStateCreating;
	RemindSoundReinit();
	return 0;
}



//����flash������ƣ����֧��255����ʾ����
static bool	RemindSoundServiceReadItemInfo(uint8_t *RemindItem)
{
	uint16_t j;
	SongClipsEntry SongClips;

	//���Ҷ�Ӧ��ConstDataId
	for(j = 0; j < SOUND_REMIND_TOTAL; j++)
	{
#ifdef CFG_FUNC_AUDIO_EFFECT_EN
		{
			extern TIMER EffectChangeTimer;
			TimeOutSet(&EffectChangeTimer, 500);
		}
#endif
		if(FLASH_NONE_ERR != SpiFlashRead(REMIND_FLASH_ADDR(j), (uint8_t *)&SongClips, sizeof(SongClipsEntry), REMIND_FLASH_READ_TIMEOUT))
		{
			return FALSE;
		}
		if(memcmp(&SongClips.id,RemindItem, sizeof(SongClips.id)) == 0)//�ҵ�
		{
			RemindSoundServiceCt.ConstDataOffset = 0;
			RemindSoundServiceCt.ConstDataAddr = SongClips.offset + REMIND_FLASH_STORE_BASE; //����������ʾ��bin ʹ����Ե�ַ
			RemindSoundServiceCt.ConstDataSize = SongClips.size;
			return TRUE;
		}
	}

	return FALSE;
}

//static void RemindSoundServiceRequestError(void)
//{
//	mv_mread_callback_unset();
//	AudioCoreSourceMute(REMIND_SOURCE_NUM, TRUE, TRUE);
//	AudioCoreSourceDisable(REMIND_SOURCE_NUM);
//	RemindSoundServiceCt.RequestRemind[0] = 0;//��Ӧ������Item��������Ϊ��ǡ�
//	RemindSoundServiceCt.IsBlock = 0;
//	if(!SoftFlagGet(SoftFlagDecoderApp))//appʹ�ý�����ʱ������flag����app����
//	{
//		SoftFlagDeregister(SoftFlagDecoderRemind);
//	}
//	//�����ڲ���stopped��Ϣ��ʹ��reset��������Ϣ����Ϊapp����ʾ��֮����ٽ罻�ӽ�������Ϣ��
//	//�Ͻ��ظ����ã�״̬������ȷ�Ͻ������ѻ�ȡ�����
////	if(RemindSoundServiceCt.RemindState != REMIND_GIVE_DECODER)
////	{
////		RemindSoundServiceCt.RemindState = REMIND_GIVE_DECODER;
////
////	}
//}

void RemindSoundServicePlayEnd(void)
{
	mv_mread_callback_unset(&RemindSoundServiceCt.RemindMemHandle);
	AudioCoreSourceMute(REMIND_SOURCE_NUM, TRUE, TRUE);
	AudioCoreSourceDisable(REMIND_SOURCE_NUM);
	RemindSoundServiceCt.IsBlock = 0;
	if(!SoftFlagGet(SoftFlagDecoderApp))//appʹ�ý�����ʱ������flag����app����
	{
		SoftFlagDeregister(SoftFlagDecoderRemind);
	}
	//�����ڲ���stopped��Ϣ��ʹ��reset��������Ϣ����Ϊapp����ʾ��֮����ٽ罻�ӽ�������Ϣ��
	//�Ͻ��ظ����ã�״̬������ȷ�Ͻ������ѻ�ȡ�����
}


static bool RemindSoundServicePlayItem(void)
{
	bool ItemCheck = FALSE;
	while((MCUCircular_GetDataLen(&RemindSoundServiceCt.RemindBlockCircular)))//��������ʾ������
	{
		//����顣
		if(RemindSoundServiceReadItemInfo((uint8_t*)&(RemindSoundServiceCt.RemindBlockCircular.CircularBuf[RemindSoundServiceCt.RemindBlockCircular.R])))
		{
				APP_DBG("Block remind:Play Item\n");//��������ʾ����Ч��
				MCUCircular_AbortData(&RemindSoundServiceCt.RemindBlockCircular, REMIND_SOUND_ID_LEN);
				RemindSoundServiceCt.IsBlock = TRUE;
				ItemCheck = TRUE;
				break;
		}
		else
		{
			MCUCircular_AbortData(&RemindSoundServiceCt.RemindBlockCircular, REMIND_SOUND_ID_LEN);
		}
	}
	//�ٲ��������ʾ������
	if(!ItemCheck)
	{
		if(RemindSoundServiceCt.RequestRemind[0] && RemindSoundServiceReadItemInfo(RemindSoundServiceCt.RequestRemind))
		{
			APP_DBG("New remind:Play Item\n");//��������ʾ����Ч��
			RemindSoundServiceCt.RequestRemind[0] = 0;//�Ѳ��ţ�����
			RemindSoundServiceCt.IsBlock = FALSE;
			ItemCheck = TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	if(!SoftFlagGet(SoftFlagDecoderRun))
	{
		APP_DBG("Decoder Not Run\n");
		return FALSE;
	}
	RemindSoundServiceCt.RemindMemHandle.mem_len = 0;
	RemindSoundServiceCt.RemindMemHandle.p = 0;
	mv_mread_callback_set(&RemindSoundServiceCt.RemindMemHandle,RemindSoundServiceFillStreamCallback);

	if(DecoderInit(&RemindSoundServiceCt.RemindMemHandle, (uint32_t)IO_TYPE_MEMORY, (uint32_t)MP3_DECODER) != RT_SUCCESS)
	{
		APP_DBG("Remind:Decoder Init Error\n");
		return FALSE;
	}
	RemindSoundServiceCt.RemindState = REMIND_PLAY;
	{
		MessageContext		msgSend;
		msgSend.msgId = MSG_REMIND_SOUND_PLAY_START; //����������ʾ�����������Ϣ��������һ��Ŀ���ͣ�ҲҪע�ⲥ��ʧ�ܣ���Ϣ����������
		MessageSend(RemindSoundServiceCt.parentMsgHandle, &msgSend);
	}
	AudioCoreSourceMute(REMIND_SOURCE_NUM, TRUE, TRUE);
	osTaskDelay(10);
	DecoderSourceNumSet(REMIND_SOURCE_NUM);
	AudioCoreSourceEnable(REMIND_SOURCE_NUM);
	DecoderPlay();
	AudioCoreSourceUnmute(REMIND_SOURCE_NUM, TRUE, TRUE);
	
#ifndef CFG_FUNC_MIXER_SRC_EN
	SongInfo* PlayingSongInfo = audio_decoder_get_song_info();
#ifdef CFG_RES_AUDIO_DACX_EN
	AudioDAC_SampleRateChange(ALL, PlayingSongInfo->sampling_rate);
#endif
#ifdef CFG_RES_AUDIO_DAC0_EN
	AudioDAC_SampleRateChange(DAC0, PlayingSongInfo->sampling_rate);
#endif
	APP_DBG("Remind:DAC Sample rate = %d\n", (int)AudioDAC_SampleRateGet(0));
#endif
	return TRUE;
}

static void RemindSoundServiceEntrance(void * param)
{
	MessageContext		msgRecv;
	MessageContext		msgSend;
	uint32_t			WaitingMsg = MAX_RECV_MSG_TIMEOUT;
	msgSend.msgId		= MSG_REMIND_SOUND_SERVICE_CREATED;
	MessageSend(RemindSoundServiceCt.parentMsgHandle, &msgSend);

	while(1)
	{
		MessageRecv(RemindSoundServiceCt.msgHandle, &msgRecv, WaitingMsg);
		switch(msgRecv.msgId)
		{
			case MSG_TASK_START:
				msgSend.msgId		= MSG_REMIND_SOUND_SERVICE_STARTED;
				MessageSend(RemindSoundServiceCt.parentMsgHandle, &msgSend);
				RemindSoundServiceCt.RemindSoundServiceState = TaskStateRunning;
				break;
				
			case MSG_TASK_PAUSE:
				RemindSoundServiceCt.RemindSoundServiceState = TaskStatePaused;
				msgSend.msgId		= MSG_REMIND_SOUND_SERVICE_PAUSED;
				MessageSend(RemindSoundServiceCt.parentMsgHandle, &msgSend);
				break;
				
			case MSG_TASK_RESUME:
				RemindSoundServiceCt.RemindSoundServiceState = TaskStateRunning;
				break;
				
			case MSG_TASK_STOP:
				msgSend.msgId		= MSG_REMIND_SOUND_SERVICE_STOPPED;
				MessageSend(RemindSoundServiceCt.parentMsgHandle, &msgSend);
				break;

			case MSG_REMIND_SOUND_PLAY_REQUEST://��Ӧ ��Ч����ʾ����������
				if(!MCUCircular_GetDataLen(&RemindSoundServiceCt.RemindBlockCircular) && !RemindSoundServiceCt.RequestRemind[0])
				{//û�� �������������ʾ����Ŀ
					break;
				}
				if(!SoftFlagGet(SoftFlagDecoderRun))
				{
					RemindSoundReinit();
					msgSend.msgId = MSG_REMIND_SOUND_PLAY_REQUEST_FAIL;
					MessageSend(RemindSoundServiceCt.parentMsgHandle, &msgSend);
				}
				else if(RemindSoundServiceCt.RemindState == REMIND_STANDBY)
				{
					if(!SoftFlagGet(SoftFlagDecoderApp))//app ��ʹ�ý�����
					{
						APP_DBG("Remind:Play Direct\n");
						SoftFlagRegister(SoftFlagDecoderRemind);//�����ʾ��ռ�н��������ɲ�ע����
						RemindSoundServicePlayItem();//�������ʧ�ܣ���Ӱ��app
					}
					else //�ȴ�app �ͷŽ�����
					{
						APP_DBG("Remind:Wait Decoder\n");
						RemindSoundServiceCt.RemindState = REMIND_WAIT_DECODER;
						msgSend.msgId = MSG_REMIND_SOUND_NEED_DECODER;
						MessageSend(RemindSoundServiceCt.parentMsgHandle, &msgSend);//
					}
				}
				else if(RemindSoundServiceCt.RemindState == REMIND_PLAY && !RemindSoundServiceCt.IsBlock)
				{//������ �ɴ��
					RemindSoundServiceCt.RemindState = REMIND_STOPPING;
					DecoderMuteAndStop();
				}

				break;

			case MSG_REMIND_SOUND_PLAY: //��ʾ������;����ϢҪ������ȷ����������ֹͣ��
				if(SoftFlagGet(SoftFlagDecoderRemind) && RemindSoundServiceCt.RemindState == REMIND_WAIT_DECODER)
				{
					APP_DBG("Remind:Play\n");

					if(!RemindSoundServicePlayItem()) //����ʧ����Ҫ����
					{
						RemindSoundServicePlayEnd();
						{
							msgSend.msgId = MSG_REMIND_SOUND_PLAY_DONE;
							gCtrlVars.remind_type = REMIND_TYPE_KEY;
							MessageSend(RemindSoundServiceCt.parentMsgHandle, &msgSend);
						}
						RemindSoundServiceCt.RemindState = REMIND_STANDBY;
						DecoderReset();//�������л�
					}
				}
				break;

				//��ʾ��service�����͹��ܸ�λ������Ϣ���أ�������״̬����Ϣ���𷽱��ϡ�
			case MSG_REMIND_SOUND_PLAY_RESET:
				mv_mread_callback_unset(&RemindSoundServiceCt.RemindMemHandle);
				if(SoftFlagGet(SoftFlagDecoderRemind))
				{
					DecoderStop();
					SoftFlagDeregister(SoftFlagDecoderRemind);//Ҫ��app�Խ�������λ������ȱmv_mread_callback_unset
				}	
				AudioCoreSourceDisable(REMIND_SOURCE_NUM);
				RemindSoundServiceCt.RequestRemind[0] = 0;//Item��������Ϊ��ǡ�
				RemindSoundServiceCt.RemindState = REMIND_STANDBY;
				RemindSoundServiceCt.IsBlock = 0;
				MCUCircular_Config(&RemindSoundServiceCt.RemindBlockCircular, RemindSoundServiceCt.RemindBlockBuf, REMIND_SOUND_ID_LEN * CFG_PARAM_REMIND_LIST_MAX);
				break;

			case MSG_REMIND_SOUND_PLAY_END:
				if(SoftFlagGet(SoftFlagDecoderRemind))
				{
					DecoderMuteAndStop();
					RemindSoundServiceCt.RemindState = REMIND_STOPPING;
				}
				MCUCircular_Config(&RemindSoundServiceCt.RemindBlockCircular, RemindSoundServiceCt.RemindBlockBuf, REMIND_SOUND_ID_LEN * CFG_PARAM_REMIND_LIST_MAX);
				RemindSoundServiceCt.RequestRemind[0] = 0;//Item��������Ϊ��ǡ�
				break;

			case MSG_DECODER_STOPPED:
				if(SoftFlagGet(SoftFlagDecoderRemind)
						&& (RemindSoundServiceCt.RemindState == REMIND_STOPPING || RemindSoundServiceCt.RemindState == REMIND_PLAY))
				{
					RemindSoundServiceCt.RemindState = REMIND_STOPPED;
					WaitingMsg = REDMIN_SOUND_SERVICE_WAIT_PLAY_END;
				}
				else
				{
					APP_DBG("Remind:Decoder stop error\n");
				}
				break;
		}
		//�ȴ�decoder pcm fifo����
		if(SoftFlagGet(SoftFlagDecoderRemind) && RemindSoundServiceCt.RemindState == REMIND_STOPPED)
		{
			if(DecodedPcmDataLenGet() == 0)//��ʾ��pcm���ſ�
			{//������ʾ������

				if(RemindSoundServicePlayItem())//�к���������ʾ������
				{
					APP_DBG("Block remind:Play Item\n");//��������ʾ����Ч��
					WaitingMsg = MAX_RECV_MSG_TIMEOUT;
				}
				else //�޺�����ʾ��
				{
					APP_DBG("Remind Stopped\n");
					WaitingMsg = MAX_RECV_MSG_TIMEOUT;
					#ifdef CFG_FUNC_AUDIO_EFFECT_EN
					extern TIMER EffectChangeTimer;
					TimeOutSet(&EffectChangeTimer, 5);//��ʱ�޸ķ�������֤����ģʽ�л�ʱ���ܿ��ٽ��������������Ż�����ͻ�����⡣
					#endif
					RemindSoundServicePlayEnd();
					RemindSoundServiceCt.RemindState = REMIND_STANDBY;
					{
						gCtrlVars.remind_type = REMIND_TYPE_KEY;
						msgSend.msgId = MSG_REMIND_SOUND_PLAY_DONE;
						MessageSend(RemindSoundServiceCt.parentMsgHandle, &msgSend);
					}
					//ͨ��ʱ,��������,����������ʾ��������,�ٽ���ͨ��ģʽ
					SoftFlagDeregister(SoftFlagWaitBtRemindEnd);
					if(SoftFlagGet(SoftFlagDelayEnterBtHf))
					{
						SoftFlagDeregister(SoftFlagDelayEnterBtHf);

						msgSend.msgId = MSG_DEVICE_SERVICE_ENTER_BTHF_MODE;
						MessageSend(GetMainMessageHandle(), &msgSend);
					}
					DecoderReset();//�������л�
				}
			}
		}
		else
		{
			WaitingMsg = MAX_RECV_MSG_TIMEOUT;
		}

	}
	return ;
}


/***************************************************************************************
 *
 * APIs
 *
 */
MessageHandle GetRemindSoundServiceMessageHandle(void)
{
	return RemindSoundServiceCt.msgHandle;
}


int32_t RemindSoundServiceCreate(MessageHandle parentMsgHandle)
{
	int32_t		ret = 0;
	ret = RemindSoundServiceInit(parentMsgHandle);
	if(!ret)
	{
		RemindSoundServiceCt.taskHandle = NULL;
		xTaskCreate(RemindSoundServiceEntrance, RemindSoundServiceName, REMIND_SOUND_SERVICE_TASK_STACK_SIZE, NULL, REMIND_SOUND_SERVICE_TASK_PRIO, &RemindSoundServiceCt.taskHandle);
		if(RemindSoundServiceCt.taskHandle == NULL)
		{
			ret = -1;
		}
	}
	if(ret)
	{
		SoftFlagRegister(SoftFlagNoRemind);
		APP_DBG("Remind:%s create fail!\n", RemindSoundServiceName);
	}
	return ret;
}

void RemindSoundServiceStart(void)
{
	MessageContext		msgSend;
	msgSend.msgId		= MSG_TASK_START;
	MessageSend(RemindSoundServiceCt.msgHandle, &msgSend);
}

void RemindSoundServicePause(void)
{
	MessageContext		msgSend;
	msgSend.msgId		= MSG_TASK_PAUSE;
	MessageSend(RemindSoundServiceCt.msgHandle, &msgSend);
}

void RemindSoundServiceResume(void)
{
	MessageContext		msgSend;
	msgSend.msgId		= MSG_TASK_RESUME;
	MessageSend(RemindSoundServiceCt.msgHandle, &msgSend);
}

void RemindSoundServiceStop(void)
{
	MessageContext		msgSend;

	msgSend.msgId		= MSG_TASK_STOP;
	MessageSend(RemindSoundServiceCt.msgHandle, &msgSend);
}

void RemindSoundServiceKill(void)
{
	//task
	if(RemindSoundServiceCt.taskHandle != NULL)
	{
		vTaskDelete(RemindSoundServiceCt.taskHandle);
		RemindSoundServiceCt.taskHandle = NULL;
	}
	
	//Msgbox
	if(RemindSoundServiceCt.msgHandle != NULL)
	{
		MessageDeregister(RemindSoundServiceCt.msgHandle);
		RemindSoundServiceCt.msgHandle = NULL;
	}
	//PortFree...
}



//��ʾ�����󣬼�¼��Ŀ�ַ�����
//BlockPlay ָ���Ų�����ϣ���λ���⡣
bool RemindSoundServiceItemRequest(char *SoundItem, bool IsBlock)
{
	bool ret;
	MessageContext		msgSend;
	if(SoftFlagGet(SoftFlagNoRemind)
			|| !SoftFlagGet(SoftFlagDecoderRun))
	{
		msgSend.msgId		= MSG_REMIND_SOUND_PLAY_REQUEST_FAIL;
		MessageSend(RemindSoundServiceCt.parentMsgHandle, &msgSend);
		ret = FALSE;
	}
	else if(SoundItem == (char *)NO_REMIND
			|| (RemindSoundServiceCt.IsBlock && IsBlock && MCUCircular_GetSpaceLen(&RemindSoundServiceCt.RemindBlockCircular) <= REMIND_SOUND_ID_LEN))
	{
		return FALSE;
	}
	else
	{
		if(IsBlock)
		{
			MCUCircular_PutData(&RemindSoundServiceCt.RemindBlockCircular, SoundItem, REMIND_SOUND_ID_LEN);
		}
		else
		{
			memcpy(RemindSoundServiceCt.RequestRemind, SoundItem, REMIND_SOUND_ID_LEN);
		}
		//��ǰ���ڽ�����������ʱ��ʵ�������� ��ʾ�����Ž���MSG_DECODER_STOPPED�󲥿պ���������
		{
			msgSend.msgId		= MSG_REMIND_SOUND_PLAY_REQUEST;
			MessageSend(RemindSoundServiceCt.msgHandle, &msgSend);
		}
		ret = TRUE;
	}
	return ret;
}

//TRUE ��������ʾ������ FALSE��æ
bool RemindSoundServiceRequestStatus(void)
{
	if(RemindSoundServiceCt.RequestRemind[0] && RemindSoundServiceCt.IsBlock)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

void RemindSoundServicePlay(void)
{
	MessageContext		msgSend;
	
	msgSend.msgId		= MSG_REMIND_SOUND_PLAY;
	MessageSend(RemindSoundServiceCt.msgHandle, &msgSend);
	return;
}

//��ʾ����������λ��
void RemindSoundServiceReset(void)
{

	MessageContext		msgSend;
	msgSend.msgId		= MSG_REMIND_SOUND_PLAY_RESET;
	AudioCoreSourceMute(DecoderSourceNumGet(), TRUE, TRUE);
	vTaskDelay(10);
	MessageSend(RemindSoundServiceCt.msgHandle, &msgSend);

	return ;
}
//��ʾ���������������ţ�������ʽ
void RemindSoundServiceEnd(void)
{
	MessageContext		msgSend;

	msgSend.msgId		= MSG_REMIND_SOUND_PLAY_END;
	MessageSend(RemindSoundServiceCt.msgHandle, &msgSend);
	return ;
}


extern TIMER EffectChangeTimer;
uint32_t RemindSoundServiceFillStreamCallback(void *buffer, uint32_t length)
{
	int32_t RemainBytes;
	int32_t ReadBytes;
	MessageContext msgSend;
	MessageHandle msgHandle;

	if(length == 0)
	{
		return 0;
	}
	RemainBytes = RemindSoundServiceCt.ConstDataSize - RemindSoundServiceCt.ConstDataOffset;
	ReadBytes   = length > RemainBytes ? RemainBytes : length;
	if(ReadBytes == 0)
	{
		if(RemindSoundServiceCt.RemindState != REMIND_STOPPING )//������Ϣʱ���������ֹͣ��
		{
#if defined(CFG_BT_RING_LOCAL) && !defined(CFG_BT_NUMBER_REMIND)
			if(GetHfpState() == BT_HFP_STATE_INCOMING && (GetSystemMode() == AppModeBtHfPlay))
			{
				RemindSoundServiceCt.RemindState = REMIND_STANDBY;
				btManager.localringState = 1;
				SoftFlagDeregister(SoftFlagDecoderRemind);
				DecoderMuteAndStop();
				msgHandle = GetBtHfMessageHandle();
				msgSend.msgId = MSG_BT_HF_MODE_REMIND_PLAY;
				MessageSend(msgHandle, &msgSend);
				APP_DBG("RemindSoundServiceFillStreamCallback return\n");
				return 0;
			}
#endif
			RemindSoundServiceCt.RemindState = REMIND_STOPPING;
			DecoderStop();//�����������Զ�ֹͣ��
#ifdef CFG_FUNC_AUDIO_EFFECT_EN
			TimeOutSet(&EffectChangeTimer, 120);
#endif
		}
//		SoftFlagDeregister(SoftFlagDecoderRemind);//���ý������������岥��ʾ��ʱ����ʱ϶��
		return 0;	//�˴β���������
	}
#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	TimeOutSet(&EffectChangeTimer, 500);
#endif
	
	if(SpiFlashRead(RemindSoundServiceCt.ConstDataAddr + RemindSoundServiceCt.ConstDataOffset,
					buffer,
					ReadBytes,
					REMIND_FLASH_READ_TIMEOUT) != FLASH_NONE_ERR)
	{
		ReadBytes = 0;
		APP_DBG("Remind:read const data error!\r\n");
	}
	APP_DBG("Remind:%d@%d\n", (int)ReadBytes, (int)RemainBytes);
	RemindSoundServiceCt.ConstDataOffset += ReadBytes;
	
	return ReadBytes;
}

unsigned short CRC16(unsigned char *Buf, unsigned int BufLen, unsigned short CRC)
{
	unsigned int i;
	for(i = 0 ; i < BufLen; i++)
	{
		CRC = (CRC << 8) ^ CrcCCITTTable[((CRC >> 8) ^ *Buf++) & 0x00FF];
	}
	return CRC;
}

//��ʾ����Ŀ��������������У�飬Ӱ�쿪���ٶȡ�
bool sound_clips_all_crc(void)
{
	SongClipsHdr *hdr;
	SongClipsEntry *ptr;
	uint16_t crc=0, i, j, CrcRead;
	uint32_t FlashAddr, all_len = 0;
	uint8_t *data_ptr = NULL;
	//uint8_t *data_ptr1 = NULL;// bkd add for test 2019.4.22

	bool ret = TRUE;
	FlashAddr = REMIND_FLASH_STORE_BASE;

	data_ptr = (uint8_t *)osPortMalloc(REMIND_FLASH_HDR_SIZE);
	//data_ptr1=data_ptr;
	if(data_ptr == NULL)
	{
		return FALSE;
	}
	if(SpiFlashRead(FlashAddr,
					data_ptr,
					REMIND_FLASH_HDR_SIZE,
					REMIND_FLASH_READ_TIMEOUT) != FLASH_NONE_ERR)
	{
		APP_DBG("read const data error!\r\n");
		ret = FALSE;
	}
	else
	{

//		APP_DBG("\r\n");

//	for(i=0;i<REMIND_FLASH_HDR_SIZE;i++)
	//	{
		
		//APP_DBG("%x ",*data_ptr1);
		//data_ptr1++;
		
		//}
	//		APP_DBG("\r\n");

		ptr = (SongClipsEntry*)(data_ptr + sizeof(SongClipsHdr));
		hdr = (SongClipsHdr *)(data_ptr);
		if(strncmp(hdr->sync, "MVUB", 4) || !hdr->cnt)
		{
			APP_DBG("sync not found or no Item\n");
			ret = FALSE;
		}
		else
		{
			for(i = 0; i < hdr->cnt; i++)
			{
				all_len += ptr[i].size;
				for(j = 0; j < REMIND_SOUND_ID_LEN; j++)
				{
					APP_DBG("^%c", ((uint8_t *)&ptr[i].id)[j]);
				}
				APP_DBG("^/");
			}
			APP_DBG("\nALL clips size = %d\n", (int)all_len);
			if(REMIND_FLASH_STORE_BASE + REMIND_FLASH_HDR_SIZE + all_len >= REMIND_FLASH_STORE_OVERFLOW)
			{
				APP_DBG("Remind flash const data overflow.\n");
				ret = FALSE;
			}
			CrcRead = hdr->crc;
			crc = CRC16(data_ptr, 4, crc);
			crc = CRC16(data_ptr + 8, REMIND_FLASH_HDR_SIZE - 8, crc);
			FlashAddr += REMIND_FLASH_HDR_SIZE;
			while(all_len && ret)
			{
				if(all_len > REMIND_FLASH_HDR_SIZE)
				{
					i = REMIND_FLASH_HDR_SIZE;
				}
				else
				{
					i = all_len;
				}
				if(SpiFlashRead(FlashAddr,
								data_ptr,
								i,
								REMIND_FLASH_READ_TIMEOUT) != FLASH_NONE_ERR)
				{
					APP_DBG("read const data error!\r\n");
					ret = FALSE;
				}
				else
				{
					crc = CRC16(data_ptr, i, crc);
					FlashAddr += i;
					all_len -= i;	
				}
			}
			if(crc == CrcRead)
			{
				APP_DBG("Crc = %04X\n", crc);
			}
			else
			{
				APP_DBG("Crc error: %04X != %04X\n", crc, CrcRead);
				ret = FALSE;
			}
		}
	}
	osPortFree(data_ptr);
	return ret;
}

#endif

