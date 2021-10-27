/**
 **************************************************************************************
 * @file    decoder_service.c
 * @brief   
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2018-1-10 20:21:00$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include <stdio.h>
#include <string.h>

#include "type.h"
#include "app_message.h"
#include "rtos_api.h"
#include "mvstdio.h"
#include "audio_decoder_api.h"
#include "audio_core_service.h"
#include "decoder_service.h"
#include "mcu_circular_buf.h"
#include "app_config.h"
#include "core_d1088.h"
#include "debug.h"
#include "main_task.h"

#ifdef CFG_APP_BT_MODE_EN
#include "bt_config.h"
#include "bt_play_api.h"
#include "bt_manager.h"
#if (BT_HFP_SUPPORT == ENABLE)
#include "bt_hf_mode.h"
#include "bt_hf_api.h"
#include "bt_record_api.h"
#endif
#endif
#include "audio_adjust.h"
#include "resampler_polyphase.h"
#include "ff.h"
#include "ffpresearch.h"
#include "audio_common.h"

static void DecoderProcess(void);
static int16_t SaveDecodedPcmData(int16_t * PcmData, uint16_t PcmDataLen);
void DecoderStopProcess(void);
extern FileType SongFileType;

#define DECODER_SERVICE_SIZE				384//640//1024
#define DECODER_SERVICE_PRIO				4
#define DECODER_SERVICE_TIMEOUT				1	/*unit ms */
#define DECODER_SERVICE_NUM_MESSAGE_QUEUE	16
#define SBC_DECODER_FIFO_MIN				(119*2)//(119*17)
#define AAC_DECODER_FIFO_MIN				(50)//(600)
#define MSBC_DECODER_FIFO_MIN				57*2//238
#define	SRC_OUT_BUF_LEN						(MAX_FRAME_SAMPLES * 2 * 4 + 48) //ּ�ڿ���ת����֡�����<=256����á�

typedef struct _DecoderServiceContext
{
	xTaskHandle 		taskHandle;
	MessageHandle		msgHandle;
	MessageHandle		parentMsgHandle;
	TaskState			serviceState;

	/*for audio decode init*/
	int32_t				ioType;
	int32_t				decoderType;
	uint32_t			DecoderSize;

	/* for decoded out fifo */
	MCU_CIRCULAR_CONTEXT DecoderCircularBuf;
	int16_t				*DecoderFifo;
	uint16_t			DecoderFifoSize; //
	osMutexId			DecoderPcmFifoMutex;
	int8_t				*decoder_buf;
	uint8_t				Channel;//Decoderfifo��������������

	/* for save decoded PCM data, unit: Sample*/
	uint32_t			pcmDataSize;
	uint32_t			savedSize;
	uint16_t			*toSavePos;

	DecoderState		decoderState;

	uint32_t			StepTime;				//�������

	/*for play timer*/
	uint32_t			DecoderSamples;			//��ǰ��δ����(ָʾ)ʱ��Ĳ�����
	uint32_t			DecoderCurPlayTime;		//��ǰ���ŵ�ʱ��
	uint32_t			DecoderLastPlayTime;

#ifdef CFG_FUNC_MIXER_SRC_EN
	ResamplerPolyphaseContext	*ResamplerCt;
	int16_t*					SRCOutBuf;
#endif
}DecoderServiceContext;

static 	DecoderServiceContext 		DecoderServiceCt;
static uint8_t	DecoderSourecNum;

/**
 * @func        DecoderService_Init
 * @brief       DecoderService��ʼ��
 * @param       MessageHandle parentMsgHandle  
 * @param		BufSize	Decoder buffer Size:4K @wav? 19K @mp3,40K @flac
 * @Output      None
 * @return      bool
 * @Others      ���������buf�����������ơ�
 * Record
 */
static bool DecoderService_Init(MessageHandle parentMsgHandle, uint32_t BufSize, uint16_t FifoSize)
{
	//DecoderServiceCt = (DecoderServiceContext)osPortMalloc(sizeof(DecoderServiceContext));
	memset(&DecoderServiceCt, 0, sizeof(DecoderServiceContext));

	/* register message handle */
	if((DecoderServiceCt.msgHandle = MessageRegister(DECODER_SERVICE_NUM_MESSAGE_QUEUE)) == NULL)
	{
		return FALSE;
	}
	DecoderServiceCt.parentMsgHandle = parentMsgHandle;
	DecoderServiceCt.serviceState = TaskStateCreating;

	DecoderServiceCt.DecoderSize = BufSize;
	DecoderServiceCt.DecoderFifoSize = FifoSize;

#ifdef CFG_FUNC_MIXER_SRC_EN
	DecoderServiceCt.ResamplerCt = (ResamplerPolyphaseContext*)osPortMalloc(sizeof(ResamplerPolyphaseContext));
	if(DecoderServiceCt.ResamplerCt == NULL)
	{
		return FALSE;
	}
//	if((DecoderServiceCt.ResamplerCt = (ResamplerPolyphaseContext*)osPortMalloc(sizeof(ResamplerPolyphaseContext))) == NULL)
//	{
//		return FALSE;
//	}
	memset(DecoderServiceCt.ResamplerCt, 0, sizeof(ResamplerPolyphaseContext));
	DecoderServiceCt.SRCOutBuf = (int16_t*)osPortMalloc(SRC_OUT_BUF_LEN);
	if(DecoderServiceCt.SRCOutBuf == NULL)
	{
		return FALSE;
	}

//	if((DecoderServiceCt.SRCOutBuf = (int16_t*)osPortMalloc(SRC_OUT_BUF_LEN)) == NULL)
//	{
//		return FALSE;
//	}
#endif

	if(!DecoderServiceCt.DecoderSize || DecoderServiceCt.DecoderFifoSize < DECODER_FIFO_SIZE_MIN)
	{
		return FALSE;
	}

	DecoderServiceCt.decoder_buf = (int8_t*)osPortMalloc(DecoderServiceCt.DecoderSize);
	if(DecoderServiceCt.decoder_buf == NULL)//DECODER_BUF_SIZE);
	{
		return FALSE;
	}
//	if((DecoderServiceCt.decoder_buf = (int8_t*)osPortMalloc(DecoderServiceCt.DecoderSize)) == NULL)//DECODER_BUF_SIZE);
//	{
//		return FALSE;
//	}
	memset(DecoderServiceCt.decoder_buf, 0, DecoderServiceCt.DecoderSize);
	
	if((DecoderServiceCt.DecoderPcmFifoMutex = xSemaphoreCreateMutex()) == NULL)
	{
		return FALSE;
	}

	DecoderServiceCt.DecoderFifo = (int16_t*)osPortMalloc(DecoderServiceCt.DecoderFifoSize);
	if(DecoderServiceCt.DecoderFifo == NULL)
	{
		return FALSE;
	}

//	if((DecoderServiceCt.DecoderFifo = (int16_t*)osPortMalloc(DecoderServiceCt.DecoderFifoSize)) == NULL)
//	{
//		return FALSE;
//	}
	memset(DecoderServiceCt.DecoderFifo, 0, DecoderServiceCt.DecoderFifoSize);
	MCUCircular_Config(&DecoderServiceCt.DecoderCircularBuf, DecoderServiceCt.DecoderFifo, DecoderServiceCt.DecoderFifoSize);

	return TRUE;
}

static void DecoderServiceEntrance(void * param)
{
	MessageContext		msgRecv;
	MessageContext		msgSend;

	DecoderServiceCt.serviceState = TaskStateReady;

	/* Send message to parent*/
	msgSend.msgId		= MSG_DECODER_SERVICE_CREATED;
	MessageSend(DecoderServiceCt.parentMsgHandle, &msgSend);

	while(1)
	{
		MessageRecv(DecoderServiceCt.msgHandle, &msgRecv, DECODER_SERVICE_TIMEOUT);

		switch(msgRecv.msgId)
		{
			case MSG_TASK_RESUME:
				APP_DBG("Decoder:Resume\n");
			case MSG_TASK_START:
				if(DecoderServiceCt.serviceState == TaskStateReady)
				{
					msgSend.msgId		= MSG_DECODER_SERVICE_STARTED;
					MessageSend(DecoderServiceCt.parentMsgHandle, &msgSend);				
					DecoderServiceCt.serviceState = TaskStateRunning;
					SoftFlagRegister(SoftFlagDecoderRun);
				}
				break;

			case MSG_TASK_PAUSE:
				DecoderDeinit();
				DecoderServiceCt.decoderState = DecoderStateNone;
				DecoderServiceCt.serviceState = TaskStateReady;
				msgSend.msgId		= MSG_DECODER_SERVICE_PAUSED;
				MessageSend(DecoderServiceCt.parentMsgHandle, &msgSend);	
				break;

			case MSG_TASK_STOP:
				APP_DBG("MSG_TASK_STOP\n");
				SoftFlagDeregister(SoftFlagDecoderRun);
				{
					//Set para
					DecoderDeinit();
					DecoderServiceCt.decoderState = DecoderStateNone;

					//clear msg
					MessageClear(DecoderServiceCt.msgHandle);
					
					//Set state
					DecoderServiceCt.serviceState = TaskStateStopped;

					//reply
					msgSend.msgId		= MSG_DECODER_SERVICE_STOPPED;
					MessageSend(DecoderServiceCt.parentMsgHandle, &msgSend);
				}
				break;
			case MSG_DECODER_RESET:
				msgSend.msgId		= MSG_DECODER_RESET;
				MessageSend(DecoderServiceCt.parentMsgHandle, &msgSend);
				DecoderServiceCt.decoderState = DecoderStateNone;
				osMutexLock(DecoderServiceCt.DecoderPcmFifoMutex);
				MCUCircular_Config(&DecoderServiceCt.DecoderCircularBuf, DecoderServiceCt.DecoderFifo, DecoderServiceCt.DecoderFifoSize);
				osMutexUnlock(DecoderServiceCt.DecoderPcmFifoMutex);
				break;
			
			case MSG_DECODER_PAUSE:
				DecoderServiceCt.decoderState = DecoderStatePause;
				break;
			case MSG_DECODER_PLAY:
				if(DecoderServiceCt.decoderState == DecoderStateInitialized)
				{
					//APP_DBG("Play @state:%d\n", DecoderServiceCt.serviceState);
					DecoderServiceCt.decoderState = DecoderStatePlay;
				}
				break;
			case MSG_DECODER_RESUME:
				if(DecoderServiceCt.decoderState != DecoderStateNone)
				{
					DecoderServiceCt.decoderState = DecoderStatePlay;
				}
				break;

			case MSG_DECODER_STOP:
				DecoderServiceCt.decoderState = DecoderStateStop;
				break;

			case MSG_DECODER_FF:
				{
					uint32_t Timer = NULL;
					Timer = DecoderServiceCt.DecoderCurPlayTime * 1000 + DecoderServiceCt.StepTime;
					APP_DBG("PlayTime = %d\n", (int)Timer);
					if(Timer > audio_decoder->song_info->duration)
					{
						DecoderServiceCt.decoderState = DecoderStatePause;
						APP_DBG("SONG FF END; Play next song.\n");
						msgSend.msgId		= MSG_NEXT;
						MessageSend(GetAppMessageHandle(), &msgSend);

						break;
					}
					audio_decoder_seek(Timer);
					DecoderServiceCt.DecoderSamples = 0;
					DecoderServiceCt.DecoderCurPlayTime += DecoderServiceCt.StepTime / 1000;
				}
				break;

			case MSG_DECODER_FB:
				{
					uint32_t Timer = 0;

					if(DecoderServiceCt.DecoderCurPlayTime * 1000 == 0)
					{
						//DecoderServiceCt.decoderState = DecoderStatePause;
						APP_DBG("backward to 0, then pause the player\n");
						msgSend.msgId		= MSG_PLAY_PAUSE;
						MessageSend(GetAppMessageHandle(), &msgSend);
						break;
					} //���˵�����������ͣ�����ֱ���ɿ����˼������¿�ʼ����(�ο�PC�˿ṷ&QQ�ͻ��˶�����������)
					else if(DecoderServiceCt.DecoderCurPlayTime * 1000 < DecoderServiceCt.StepTime)
					{
						DecoderServiceCt.DecoderCurPlayTime = 0;
					}
					else
					{
						Timer = DecoderServiceCt.DecoderCurPlayTime * 1000 - DecoderServiceCt.StepTime;
						audio_decoder_seek(Timer);
						DecoderServiceCt.DecoderSamples = 0;
						DecoderServiceCt.DecoderCurPlayTime -= DecoderServiceCt.StepTime / 1000;
					}
				}
				break;

			default:
				break;
		}
		if(DecoderServiceCt.serviceState == TaskStateRunning)
		{
			DecoderProcess();

			{
				MessageContext		msgSend;
				msgSend.msgId		= MSG_NONE;
				MessageSend(GetAudioCoreServiceMsgHandle(), &msgSend);
			}
		}
	}
}


//extern MemHandle SBC_MemHandle;
static void DecoderProcess(void)
{
	int32_t DecoderErrCode;
	static int32_t DecoderCnt = 0;
	uint16_t SampleForRetry = CFG_PARA_MAX_SAMPLES_PER_FRAME;//���ν����Ŀ����������������

	do{
		switch(DecoderServiceCt.decoderState)
		{
			case DecoderStateInitialized:
				break;

			case DecoderStatePlay:
				{
					DecoderServiceCt.decoderState	= DecoderStateDecoding;
					MessageContext		msgSend;
					msgSend.msgId		= MSG_DECODER_PLAYING;
					MessageSend(DecoderServiceCt.parentMsgHandle, &msgSend);
					osMutexLock(DecoderServiceCt.DecoderPcmFifoMutex);
					MCUCircular_Config(&DecoderServiceCt.DecoderCircularBuf, DecoderServiceCt.DecoderFifo, DecoderServiceCt.DecoderFifoSize);
					AudioCoreSourcePcmFormatConfig(DecoderSourecNum, (int)audio_decoder->song_info->num_channels);//����audiocore
					DecoderServiceCt.Channel = audio_decoder->song_info->num_channels;//���ñ���fifo
					//AudioCoreSourceUnmute(DecoderSourecNum, TRUE, TRUE);//�����������е��룬sdk����ʱ������磬Ч�����ԡ�
					osMutexUnlock(DecoderServiceCt.DecoderPcmFifoMutex);
				}
				break;

			case DecoderStateDecoding:
				if(RT_SUCCESS != audio_decoder_can_continue())
				{
					DecoderServiceCt.decoderState = DecoderStatePause;//DecoderStateStop;//���ݲ�����
					if(DecoderServiceCt.serviceState == TaskStateRunning)
					{
						MessageContext		msgSend;
						//DecoderServiceCt.serviceState = TaskStateStopped;
						msgSend.msgId		= MSG_DECODER_SERVICE_SONG_PLAY_END;
						MessageSend(DecoderServiceCt.parentMsgHandle, &msgSend);
					}
					return;
				}

	#ifdef CFG_APP_BT_MODE_EN
				//����ģʽ��SBC����,��Ҫ�ж�decoder_memˮλ,��������̫������������
				if(mainAppCt.appCurrentMode == AppModeBtAudioPlay)
				{
					if(!SoftFlagGet(SoftFlagDecoderRemind))
					{
						if(GetBtManager()->a2dpStreamType == BT_A2DP_STREAM_TYPE_SBC)
						{
							//sbc
							if(GetValidSbcDataSize() <= SBC_DECODER_FIFO_MIN)
							{
								return;
							}
						}
						else if(GetBtManager()->a2dpStreamType == BT_A2DP_STREAM_TYPE_AAC)
						{
							//aac
							//if(GetValidSbcDataSize() <= AAC_DECODER_FIFO_MIN)
							if(btManager.aacFrameNumber==0)
							{
								DBG("AAC empty\n");
								return;
							}
							if(btManager.aacFrameNumber)
								btManager.aacFrameNumber--;
						}
					}
				}
				
				if(mainAppCt.appCurrentMode == AppModeBtHfPlay)
				{
					if(!SoftFlagGet(SoftFlagDecoderRemind))
					{
						if(BtHf_MsbcDataLenGet() <= MSBC_DECODER_FIFO_MIN)
						{
							return;
						}
					}
				}
				
				if(mainAppCt.appCurrentMode == AppModeBtRecordPlay)
				{
					if(!SoftFlagGet(SoftFlagDecoderRemind))
					{
						if(BtRecord_MsbcDataLenGet() <= MSBC_DECODER_FIFO_MIN)
						{
							return;
						}
					}
				}
	#endif

				if(audio_decoder_decode() != RT_SUCCESS)
				{
					DecoderErrCode = audio_decoder_get_error_code();

					
#if defined(CFG_APP_USB_PLAY_MODE_EN) || defined(CFG_APP_CARD_PLAY_MODE_EN) || defined(CFG_FUNC_RECORDER_EN)
					if((SongFileType != FILE_TYPE_MP3)
						&& (SongFileType != FILE_TYPE_SBC)
						&& (SongFileType != FILE_TYPE_MSBC)
						&& (SongFileType != FILE_TYPE_WAV))
					{
						if(DecoderServiceCt.serviceState == TaskStateRunning)
						{
							extern uint32_t CmdErrCnt;
							MessageContext		msgSend;

							msgSend.msgId		= MSG_DECODER_SERVICE_SONG_PLAY_END;
							if(CmdErrCnt >= 3)
							{
								msgSend.msgId = MSG_DECODER_SERVICE_DISK_ERROR;
							}
							MessageSend(DecoderServiceCt.parentMsgHandle, &msgSend);
						}
						if(!SoftFlagGet(SoftFlagDecoderRemind))
							APP_DBG("decoder err, can not continue\n");
						return;
					}
#endif
#ifdef CFG_APP_BT_MODE_EN
					//sbc
					if((mainAppCt.appCurrentMode == AppModeBtAudioPlay) && (GetBtManager()->a2dpStreamType != BT_A2DP_STREAM_TYPE_AAC))
					{
						if(-123 == DecoderErrCode)
						{
							MessageContext		msgSend;

							DecoderCnt++;
							if(DecoderCnt > 300)
							{
								DecoderCnt = 0;
								DecoderServiceCt.decoderState = DecoderStatePause;
								msgSend.msgId		= MSG_DECODER_SERVICE_SONG_PLAY_END;
								MessageSend(DecoderServiceCt.parentMsgHandle, &msgSend);
								APP_DBG("END SONG FOR DECODER TIMEOUT\n");
							}
							//����Ϣ��app task��û�д���Ϊ��ʱ���ᵼ�¸�msg��ͣ�ķ��ͣ��Ӷ��屬��msg handle����Ϣ���У�����msg�ò�����Ӧ
							//��ʱ����
							/*else
							{
								msgSend.msgId		= MSG_DECODER_SERVICE_FIFO_EMPTY;
								MessageSend(DecoderServiceCt.parentMsgHandle, &msgSend);
							}*/
						}
						else
						{
							//���ִ���ʱ�����³�ʼ��decoder
							BtSbcDecoderRefresh();
						}
						return;
					}
#endif
					
					if(-127 == DecoderErrCode)
					{
						MessageContext		msgSend;

						DecoderCnt++;
						//�쳣,����500�ζ���������
#ifdef CFG_APP_BT_MODE_EN
						if((DecoderCnt > 10)&&(mainAppCt.appCurrentMode == AppModeBtAudioPlay)
							&&(GetBtManager()->a2dpStreamType == BT_A2DP_STREAM_TYPE_AAC))
						{
							DecoderCnt = 0;
							DecoderServiceCt.decoderState = DecoderStatePause;
							msgSend.msgId		= MSG_DECODER_SERVICE_SONG_PLAY_END;
							MessageSend(DecoderServiceCt.parentMsgHandle, &msgSend);
							//APP_DBG("END SONG FOR DECODER TIMEOUT\n");
						}
						else if(DecoderCnt > 500)
#else
						if(DecoderCnt > 500)
#endif
						{
							DecoderCnt = 0;
							DecoderServiceCt.decoderState = DecoderStatePause;
							//msgSend.msgId		= MSG_DECODER_SERVICE_SONG_PLAY_END;
							msgSend.msgId = MSG_DECODER_SERVICE_DISK_ERROR;
							MessageSend(DecoderServiceCt.parentMsgHandle, &msgSend);
							APP_DBG("END SONG FOR DECODER TIMEOUT\n");
						}
						//����Ϣ��app task��û�д���Ϊ��ʱ���ᵼ�¸�msg��ͣ�ķ��ͣ��Ӷ��屬��msg handle����Ϣ���У�����msg�ò�����Ӧ
						//��ʱ����
						/*else
						{
							msgSend.msgId		= MSG_DECODER_SERVICE_FIFO_EMPTY;
							MessageSend(DecoderServiceCt.parentMsgHandle, &msgSend);
						}*/
					}
					/*else
					{
						if(mainAppCt.appCurrentMode == AppModeBtAudioPlay)
						{
							//���ִ���ʱ�����³�ʼ��decoder
							BtSbcDecoderRefresh();
						}
					}*/
					return;
				}
				else if(DecoderCnt != 0)
				{
					APP_DBG("FIFO_EMPTY Cnt = %d\n",(int)DecoderCnt);
				}
				DecoderCnt = 0;
				//���㲥��ʱ��
				{
					uint32_t Temp;
					DecoderServiceCt.DecoderSamples += audio_decoder->song_info->pcm_data_length;
					Temp = DecoderServiceCt.DecoderSamples / audio_decoder->song_info->sampling_rate;
					DecoderServiceCt.DecoderSamples -= Temp * audio_decoder->song_info->sampling_rate;
					DecoderServiceCt.DecoderCurPlayTime += Temp;
					if((DecoderServiceCt.DecoderCurPlayTime ) != (DecoderServiceCt.DecoderLastPlayTime))
					{
						MessageContext		msgSend;

						msgSend.msgId		= MSG_DECODER_SERVICE_UPDATA_PLAY_TIME;
						MessageSend(DecoderServiceCt.parentMsgHandle, &msgSend);

						//DecoderServiceCt.DecoderLastPlayTime = DecoderServiceCt.DecoderCurPlayTime;
					}
					DecoderServiceCt.DecoderLastPlayTime = DecoderServiceCt.DecoderCurPlayTime;
				}
				DecoderServiceCt.decoderState = DecoderStateToSavePcmData;
			case DecoderStateToSavePcmData:
				{
					DecoderServiceCt.pcmDataSize	= audio_decoder->song_info->pcm_data_length * audio_decoder->song_info->num_channels;
					DecoderServiceCt.savedSize		= 0;
					DecoderServiceCt.toSavePos		= (uint16_t *)audio_decoder->song_info->pcm_addr;
					DecoderServiceCt.decoderState = DecoderStateSavePcmData;
					if(DecoderServiceCt.pcmDataSize == 0)//add by sam, 20180814������һ���ݴ����пͻ����������������lengthΪ0
					{
						DecoderServiceCt.decoderState = DecoderStateDecoding;
						SampleForRetry -= 1;//��������,�������0������ʱ�˳���
						break;
					}
				}

			case DecoderStateSavePcmData:
				{
					int32_t		savedSize;  //������ֵ

					savedSize = SaveDecodedPcmData((int16_t*)DecoderServiceCt.toSavePos,
								(DecoderServiceCt.pcmDataSize - DecoderServiceCt.savedSize) / audio_decoder->song_info->num_channels);

					if(savedSize > 0)
					{
						DecoderServiceCt.savedSize += savedSize * audio_decoder->song_info->num_channels;
						DecoderServiceCt.toSavePos += savedSize * audio_decoder->song_info->num_channels;
						if(DecoderServiceCt.savedSize == DecoderServiceCt.pcmDataSize) //�ϴν��������ȫ�� ����fifo
						{
							DecoderServiceCt.decoderState = DecoderStateDecoding;
							if(SampleForRetry > savedSize)
							{
								SampleForRetry -= savedSize;
								break;//���ⵥ�ν�������̫�٣�����OS�л������������buf����ʱ���ٴν��롣
							}
						}
					}
					SampleForRetry = 0;
				}
				break;

			case DecoderStateDeinitializing:
				DecoderServiceCt.decoderState = DecoderStateNone;
				break;

			case DecoderStateStop:
				DecoderCnt = 0;
				DecoderStopProcess();
				{
					MessageContext		msgSend;
					msgSend.msgId		= MSG_DECODER_STOPPED;
					MessageSend(DecoderServiceCt.parentMsgHandle, &msgSend);
				}
				break;

			case DecoderStatePause:
				//APP_DBG("Decoder play Pause\n");
				//DecoderServiceCt.decoderState = DecoderStateNone;
				break;

			default:
				break;
		}
	}while(SampleForRetry && (DecoderServiceCt.decoderState == DecoderStateSavePcmData || DecoderServiceCt.decoderState == DecoderStateDecoding));
}


static int16_t SaveDecodedPcmData(int16_t * PcmData, uint16_t PcmDataLen)
{
	int32_t		TargetSampleRate = CFG_PARA_SAMPLE_RATE;
	uint32_t	ProcessSampleLen, SpaceSampleLen, SavedSampleLen = 0;
#ifdef CFG_FUNC_SOFT_ADJUST_IN
	int32_t		SRADoneaLen;//SRA����Ч����
#endif
	uint32_t	FillSpaceLen = 0;//������fifo������
#ifdef CFG_FUNC_MIXER_SRC_EN
	uint16_t	SRCOutLimit;
	int16_t SRCDoneLen; 			//SRC֮������outbuf�����ݳ���
#endif

	if(PcmData == NULL || PcmDataLen == 0)
	{
		return -1;
	}
#if (BT_HFP_SUPPORT == ENABLE)
	if(GetSystemMode() == AppModeBtHfPlay)
	{
		TargetSampleRate = CFG_BTHF_PARA_SAMPLE_RATE;//ͨ��ģʽ������Ϊ16K
	}
#endif
	osMutexLock(DecoderServiceCt.DecoderPcmFifoMutex);
	{
		SpaceSampleLen = MCUCircular_GetSpaceLen(&DecoderServiceCt.DecoderCircularBuf)/(audio_decoder->song_info->num_channels * 2) - 4;
	}

#ifdef CFG_FUNC_MIXER_SRC_EN
	SRCOutLimit =  ((SRC_OUT_BUF_LEN - 48) * audio_decoder->song_info->sampling_rate) / (TargetSampleRate * audio_decoder->song_info->num_channels * 2);
#if defined(CFG_FUNC_SOFT_ADJUST_IN) && defined(CFG_APP_BT_MODE_EN)
	if((GetSystemMode() == AppModeBtAudioPlay)
	&& !SoftFlagGet(SoftFlagDecoderRemind))
	{
		if(SpaceSampleLen < MAX_FRAME_SAMPLES + 20)
		{
			osMutexUnlock(DecoderServiceCt.DecoderPcmFifoMutex);
			//*RetryLen = 0;
			return 0;///�ռ䲻�����˳�
		}
		ProcessSampleLen = MAX_FRAME_SAMPLES;
		if(ProcessSampleLen > SRCOutLimit)
		{
			ProcessSampleLen = SRCOutLimit;
		}
		//only debug
//		if(PcmDataLen < 128)
//		{
//			APP_DBG("!!!err\n");
//		}

		if(SoftFlagGet(SoftFlagBtSra) && !SoftFlagGet(SoftFlagDecoderRemind))
		{
			SRADoneaLen = AudioSourceSRAProcess(PcmData, (uint16_t)ProcessSampleLen);
		}
		else
		{
			SRADoneaLen = ProcessSampleLen;
		}
		if(audio_decoder->song_info->sampling_rate != TargetSampleRate)//�����ʺ�Ŀ������ʲ�һ�£���Ҫת����
		{
			SRCDoneLen = resampler_polyphase_apply(DecoderServiceCt.ResamplerCt, audioSourceAdjustCt->SraPcmOutnBuf, DecoderServiceCt.SRCOutBuf, SRADoneaLen);
			if(SRCDoneLen > 0)
			{
				MCUCircular_PutData(&DecoderServiceCt.DecoderCircularBuf, DecoderServiceCt.SRCOutBuf, SRCDoneLen * audio_decoder->song_info->num_channels * 2);
				FillSpaceLen += SRADoneaLen;
			}
		
		}
		else
		{
			MCUCircular_PutData(&DecoderServiceCt.DecoderCircularBuf, audioSourceAdjustCt->SraPcmOutnBuf, SRADoneaLen * audio_decoder->song_info->num_channels * 2);
			FillSpaceLen += SRADoneaLen;
		}
		SavedSampleLen = ProcessSampleLen;
	}
	else
#endif
	{
		if(audio_decoder->song_info->sampling_rate == TargetSampleRate)
		{
			ProcessSampleLen = SpaceSampleLen < PcmDataLen ? SpaceSampleLen : PcmDataLen;
			MCUCircular_PutData(&DecoderServiceCt.DecoderCircularBuf, PcmData, ProcessSampleLen * audio_decoder->song_info->num_channels * 2);
			FillSpaceLen = SavedSampleLen = ProcessSampleLen;
		}
		else if(SpaceSampleLen >= MAX_FRAME_SAMPLES)//������Ҫת����������ת������Ƭ����space��Сʱ���´ε����ٴ洦��
		{
			do
			{
				ProcessSampleLen = ((SpaceSampleLen - FillSpaceLen - 1) * audio_decoder->song_info->sampling_rate) / TargetSampleRate;//Ԥ��һ��ƫ�;
				if(ProcessSampleLen >= PcmDataLen - SavedSampleLen) //buf �� Ŀ�����ݶ�
				{
					if(PcmDataLen - SavedSampleLen <= MAX_FRAME_SAMPLES)
					{
						ProcessSampleLen = PcmDataLen - SavedSampleLen;
					}
					else if(PcmDataLen - SavedSampleLen - MAX_FRAME_SAMPLES < MAX_FRAME_SAMPLES / 2) //��������µ㣬������0.5~0.75֡����
					{
						ProcessSampleLen = (PcmDataLen - SavedSampleLen) / 2;
					}
					else
					{
						ProcessSampleLen = MAX_FRAME_SAMPLES;
					}
				}
				else // buf����Ŀ�����ݣ�����space ����
				{
					//if(PcmDataLen - SavedSampleLen >= MAX_FRAME_SAMPLES + MAX_FRAME_SAMPLES / 2)
					if(PcmDataLen - SavedSampleLen >= MAX_FRAME_SAMPLES)
					{
						if(ProcessSampleLen >= MAX_FRAME_SAMPLES)
						{
							ProcessSampleLen = MAX_FRAME_SAMPLES;
						}//else ProcessSampleLen���� ����space ���С�� space1/4���ѭ��
					}
					else
					{
						break;//space����ʱ�ȵȵ�
					}
				}
				if(ProcessSampleLen > SRCOutLimit)//����SRCOut buf��8Kת 44K֮�࣬��Ҫ���Ͳ������������������
				{
					ProcessSampleLen = SRCOutLimit;
				}
				SRCDoneLen = resampler_polyphase_apply(DecoderServiceCt.ResamplerCt, PcmData + SavedSampleLen * audio_decoder->song_info->num_channels, DecoderServiceCt.SRCOutBuf, ProcessSampleLen);
				if(SRCDoneLen > 0)
				{
					MCUCircular_PutData(&DecoderServiceCt.DecoderCircularBuf, DecoderServiceCt.SRCOutBuf, SRCDoneLen * audio_decoder->song_info->num_channels * 2);
					FillSpaceLen += SRCDoneLen;
				}
				SavedSampleLen += ProcessSampleLen;
			}while(SpaceSampleLen - FillSpaceLen >= MAX_FRAME_SAMPLES / 4 && PcmDataLen - SavedSampleLen >= MAX_FRAME_SAMPLES / 4 && audio_decoder->song_info->sampling_rate > TargetSampleRate);
		}//(FillSpaceLen  <= MAX_FRAME_SAMPLES) &&
	}
#else
	ProcessSampleLen = SpaceSampleLen < PcmDataLen ? SpaceSampleLen : PcmDataLen;
	MCUCircular_PutData(&DecoderServiceCt.DecoderCircularBuf, PcmData, ProcessSampleLen * audio_decoder->song_info->num_channels * 2);
	FillSpaceLen = SavedSampleLen = ProcessSampleLen;
#endif
	osMutexUnlock(DecoderServiceCt.DecoderPcmFifoMutex);
	return SavedSampleLen;
}

void DecoderStopProcess(void)
{
	APP_DBG("Decoder:play stop\n");
	DecoderServiceCt.DecoderLastPlayTime = 0;
	DecoderServiceCt.DecoderCurPlayTime = 0;
	DecoderServiceCt.DecoderSamples = 0;
	DecoderServiceCt.decoderState = DecoderStateNone;
}


/***************************************************************************************
 *
 * APIs
 *
 */
bool DecoderServiceCreate(MessageHandle parentMsgHandle, uint32_t BufSize, uint16_t FifoSize)
{
	bool		ret = TRUE;
	
	ret = DecoderService_Init(parentMsgHandle, BufSize, FifoSize);
	
	
	if(ret)
	{
		DecoderServiceCt.taskHandle = NULL;
		xTaskCreate(DecoderServiceEntrance,"DecoderService", DECODER_SERVICE_SIZE, NULL, DECODER_SERVICE_PRIO, &DecoderServiceCt.taskHandle);
		if(DecoderServiceCt.taskHandle == NULL)
		{
			ret = FALSE;
		}
	}
	if(!ret)
		APP_DBG("DecoderService create fail!\n");

	
	return ret;
}

void DecoderServiceStart(void)
{
	MessageContext		msgSend;
	if(DecoderServiceCt.msgHandle==NULL)
	{	
		return;
	}
	msgSend.msgId		= MSG_TASK_START;
	MessageSend(DecoderServiceCt.msgHandle, &msgSend);
}

void DecoderServicePause(void)
{
	MessageContext		msgSend;
	if(DecoderServiceCt.msgHandle==NULL)
	{	
		return;
	}

	msgSend.msgId		= MSG_TASK_PAUSE;
	MessageSend(DecoderServiceCt.msgHandle, &msgSend);
}

void DecoderServiceResume(void)
{
	MessageContext		msgSend;
	if(DecoderServiceCt.msgHandle==NULL)
	{	
		return;
	}
	msgSend.msgId		= MSG_TASK_RESUME;
	MessageSend(DecoderServiceCt.msgHandle, &msgSend);
}

void DecoderServiceStop(void)
{
	MessageContext		msgSend;
	if(DecoderServiceCt.msgHandle==NULL)
	{	
		return;
	}

	msgSend.msgId		= MSG_TASK_STOP;
	MessageSend(DecoderServiceCt.msgHandle, &msgSend);
}

void DecoderServiceKill(void)
{
	//AudioCoreSourceUnmute(DecoderSourecNum, TRUE, TRUE);//������ͨ���ָ���̬��ע��:killʱ��������ʾ��ͨ������app��Դͨ����
	SoftFlagDeregister(SoftFlagDecoderRun);//����ǿ��ɱ�� ״̬�ָ���
	//PortFree... 
	if(DecoderServiceCt.DecoderPcmFifoMutex != NULL)
	{
		osMutexLock(DecoderServiceCt.DecoderPcmFifoMutex);
	}
	
	if(DecoderServiceCt.DecoderFifo != NULL)
	{
		osPortFree(DecoderServiceCt.DecoderFifo);
		DecoderServiceCt.DecoderFifo = NULL;
	}

	if(DecoderServiceCt.decoder_buf != NULL)
	{
		osPortFree(DecoderServiceCt.decoder_buf);
		DecoderServiceCt.decoder_buf = NULL;
	}
	if(DecoderServiceCt.DecoderPcmFifoMutex != NULL)
	{
		osMutexUnlock(DecoderServiceCt.DecoderPcmFifoMutex);
	}

	//task ��ɾ������ɾ���䣬����Դ
	if(DecoderServiceCt.taskHandle != NULL)
	{
		vTaskDelete(DecoderServiceCt.taskHandle);
		DecoderServiceCt.taskHandle = NULL;
	}

	//Msgbox
	if(DecoderServiceCt.msgHandle != NULL)
	{
		MessageDeregister(DecoderServiceCt.msgHandle);
		DecoderServiceCt.msgHandle = NULL;
	}

	if(DecoderServiceCt.DecoderPcmFifoMutex != NULL)
	{
		vSemaphoreDelete(DecoderServiceCt.DecoderPcmFifoMutex);
		DecoderServiceCt.DecoderPcmFifoMutex = NULL;
	}

#ifdef CFG_FUNC_MIXER_SRC_EN
	if(DecoderServiceCt.SRCOutBuf != NULL)
	{
		osPortFree(DecoderServiceCt.SRCOutBuf);
		DecoderServiceCt.SRCOutBuf = NULL;
	}

	if(DecoderServiceCt.ResamplerCt != NULL)
	{
		osPortFree(DecoderServiceCt.ResamplerCt);
		DecoderServiceCt.ResamplerCt = NULL;
	}
#endif	
}

//����������������
void DecoderServiceMsg(void)
{
	MessageContext		msgSend;
	msgSend.msgId		= MSG_NONE;
	if(DecoderServiceCt.msgHandle==NULL)
	{	
		return;
	}
	MessageSend(DecoderServiceCt.msgHandle, &msgSend);
}

MessageHandle GetDecoderServiceMsgHandle(void)
{
	return DecoderServiceCt.msgHandle;
}

TaskState GetDecoderServiceState(void)
{
	return DecoderServiceCt.serviceState;
}

int32_t DecoderInit(void *io_handle, int32_t ioType, int32_t decoderType)
{
		int32_t 	ret;//= DECODER_NORMAL;
		if(DecoderServiceCt.decoderState != DecoderStateNone && DecoderServiceCt.decoderState != DecoderStateInitialized)
		{
			APP_DBG("Decoder State Error :%d \n", DecoderServiceCt.decoderState);
			return RT_FAILURE;
		}
		
		ret = audio_decoder_initialize((uint8_t*)DecoderServiceCt.decoder_buf, io_handle, ioType, decoderType);
		if(audio_decoder_get_context_size() >= DecoderServiceCt.DecoderSize)
		{
			APP_DBG("Decodersize set error!!!!!!!");
			return RT_FAILURE;//��ȫ���
		}
		if(ret == RT_SUCCESS)
		{
			DecoderServiceCt.decoderState = DecoderStateInitialized;
			//APP_DBG("[SONG_INFO]: ChannelCnt : %6d\n",		  (int)audio_decoder->song_info->num_channels);
			APP_DBG("[SONG_INFO]: SampleRate : %6d Hz\n",	  (int)audio_decoder->song_info->sampling_rate);
			//APP_DBG("[SONG_INFO]: BitRate	 : %6d Kbps\n",   (int)audio_decoder->song_info->bitrate / 1000);
			//APP_DBG("[SONG_INFO]: DecoderSize: %6d Bytes \n", (int)audio_decoder->decoder_size);
			if(audio_decoder->song_info->stream_type != STREAM_TYPE_FLAC && audio_decoder->song_info->sampling_rate > 48000)
			{
				return RT_FAILURE;//��ȡ�����ٶ����ޣ����β��ָ����ʸ�����
			}
			
#ifdef CFG_FUNC_MIXER_SRC_EN
			if(GetSystemMode() == AppModeBtHfPlay)
			{
				if(audio_decoder->song_info->sampling_rate != CFG_BTHF_PARA_SAMPLE_RATE)
				{
#ifdef CFG_BT_RING_LOCAL
					if(GetHfpState() == BT_HFP_STATE_INCOMING)
					{
						resampler_polyphase_init(DecoderServiceCt.ResamplerCt, audio_decoder->song_info->num_channels, RESAMPLER_POLYPHASE_SRC_RATIO_160_441);
					}
					else
#endif
					{
						resampler_polyphase_init(DecoderServiceCt.ResamplerCt, audio_decoder->song_info->num_channels, RESAMPLER_POLYPHASE_SRC_RATIO_160_441);
					}
				}
			}
			else
			{
				if (audio_decoder->song_info->sampling_rate != CFG_PARA_SAMPLE_RATE)
				{
					resampler_polyphase_init(DecoderServiceCt.ResamplerCt, audio_decoder->song_info->num_channels, Get_Resampler_Polyphase(audio_decoder->song_info->sampling_rate));
				}
			}
#endif
		}
		else
		{
			int32_t 	errCode;
			errCode = audio_decoder_get_error_code();
			APP_DBG("AudioDecoder init err code = %d\n", (int)errCode);
		}
		return ret;
}

void DecoderDeinit(void)
{
	DecoderServiceCt.decoderState = DecoderStateDeinitializing;
}


/**
 * @func        DecodedPcmDataGet
 * @brief       Decoder����������� ����API
 * @param       void * pcmData
                uint16_t sampleLen  
 * @Output      None
 * @return      uint16_t ȡ���Ĳ�������
 * @Others      ��������Ҫ������ָ����������ȡ�����ݣ�����ʱ����ʵ�ʳ���ȡ���ݡ�
 * Record
 */
uint16_t DecodedPcmDataGet(void * pcmData, uint16_t sampleLen)
{
	int16_t		dataSize;
	uint32_t	getSize;
	uint32_t 	SpacLen = 0;
	uint32_t 	ValLen = 0;

	if(DecoderServiceCt.DecoderPcmFifoMutex == NULL)
	{
		return 0;
	}

	if(DecoderServiceCt.serviceState != TaskStateRunning)
	{
		return 0;
	}

	getSize = sampleLen * 2 * DecoderServiceCt.Channel;//�������� * 2byte���* ͨ����
	if(getSize == 0)
	{
		return 0;
	}

	osMutexLock(DecoderServiceCt.DecoderPcmFifoMutex);
	SpacLen = MCUCircular_GetSpaceLen(&DecoderServiceCt.DecoderCircularBuf);

	ValLen = DecoderServiceCt.DecoderFifoSize - SpacLen;

	if(getSize > ValLen)
	{
		getSize = ValLen;
	}

	dataSize = MCUCircular_GetData(&DecoderServiceCt.DecoderCircularBuf, pcmData, getSize);
	osMutexUnlock(DecoderServiceCt.DecoderPcmFifoMutex);

	return dataSize / (2 * DecoderServiceCt.Channel);
}

/**
 * @func        DecodedPcmDataLenGet
 * @brief       Decoder�������FIFO�ڲ�������
 * @Output      None
 * @return      uint16_t FIFO�в�������
 * Record
 */
uint16_t DecodedPcmDataLenGet(void)
{
	if(DecoderServiceCt.DecoderPcmFifoMutex == NULL || DecoderServiceCt.serviceState != TaskStateRunning)
	{
		return 0;
	}
	return MCUCircular_GetDataLen(&DecoderServiceCt.DecoderCircularBuf) / (2 * DecoderServiceCt.Channel);
}

SongInfo * GetSongInfo(void)
{
	return audio_decoder->song_info;
}

//����������
void DecoderPlay(void)
{
	MessageContext		msgSend;
	if(DecoderServiceCt.msgHandle==NULL)
	{	
		return;
	}

	msgSend.msgId		= MSG_DECODER_PLAY;
	MessageSend(DecoderServiceCt.msgHandle, &msgSend);
}

//��������λ
void DecoderReset(void)
{
	MessageContext		msgSend;
	if(DecoderServiceCt.msgHandle==NULL)
	{	
		return;
	}

	msgSend.msgId		= MSG_DECODER_RESET;
	MessageSend(DecoderServiceCt.msgHandle, &msgSend);
}

//������ֹͣ
void DecoderStop(void)
{
	MessageContext		msgSend;
	if(DecoderServiceCt.msgHandle==NULL)
	{	
		return;
	}
	msgSend.msgId		= MSG_DECODER_STOP;
	MessageSend(DecoderServiceCt.msgHandle, &msgSend);
}

//������ֹͣ
void DecoderMuteAndStop(void)
{
	//MessageContext		msgSend;
	if(DecoderServiceCt.msgHandle==NULL)
	{	
		return;
	}

	AudioCoreSourceMute(DecoderSourecNum, TRUE, TRUE);
	vTaskDelay(20);
	DecoderStop();
}

void DecoderPause(void)
{
	MessageContext		msgSend;
	if(DecoderServiceCt.msgHandle==NULL)
	{	
		return;
	}
	if(DecoderServiceCt.decoderState <= DecoderStatePause)
	{
		return;
	}

	msgSend.msgId		= MSG_DECODER_PAUSE;
	MessageSend(DecoderServiceCt.msgHandle, &msgSend);
}

void DecoderResume(void)
{
	MessageContext		msgSend;
	if(DecoderServiceCt.msgHandle==NULL)
	{	
		return;
	}
	APP_DBG("Decoder Resume()\n");

	msgSend.msgId		= MSG_DECODER_RESUME;
	MessageSend(DecoderServiceCt.msgHandle, &msgSend);
}

//ע�⣬StepTime ��λΪ���룬ʵ�ʴ���ֵΪ������
void DecoderFF(uint32_t StepTime)
{
	MessageContext		msgSend;

	APP_DBG("Decoder FF()\n");
	APP_DBG("Decoder state = %d\n", DecoderServiceCt.decoderState);
	if(DecoderServiceCt.msgHandle==NULL)
	{	
		return;
	}
	if(DecoderServiceCt.decoderState <= DecoderStatePause)//Playing
	{
		return;
	}

	DecoderServiceCt.StepTime = StepTime;

	msgSend.msgId		= MSG_DECODER_FF;
	MessageSend(DecoderServiceCt.msgHandle, &msgSend);
}

//ע�⣬StepTime ��λΪ���룬ʵ�ʴ���ֵΪ������
void DecoderFB(uint32_t StepTime)
{
	MessageContext		msgSend;

	APP_DBG("Decoder FB()\n");
	if(DecoderServiceCt.msgHandle==NULL)
	{	
		return;
	}

	if(DecoderServiceCt.decoderState <= DecoderStatePause)//Playing
	{
		return;
	}

	DecoderServiceCt.StepTime = StepTime;

	msgSend.msgId		= MSG_DECODER_FB;
	MessageSend(DecoderServiceCt.msgHandle, &msgSend);
}

//������seek �ļ�����ʱ�� �룬��ͬ��ʽ��������Ȳ�����
bool DecoderSeek(uint32_t Time)
{
	APP_DBG("DecoderTaskSeek()\n");
	audio_decoder_seek(Time * 1000);
	DecoderServiceCt.DecoderSamples = 0;
	DecoderServiceCt.DecoderCurPlayTime = 0;
	if(audio_decoder_seek(Time * 1000) != RT_SUCCESS)
	{
		return FALSE;
	}
	DecoderServiceCt.DecoderCurPlayTime = Time;
	return TRUE;
}

uint32_t DecoderServicePlayTimeGet(void)
{
 	return DecoderServiceCt.DecoderCurPlayTime;
}

uint32_t GetDecoderState(void)
{
	return DecoderServiceCt.decoderState;
}

//�˱�����Ϊȫ�ֻ�ȡ����ҪΪ�ա�
uint8_t DecoderSourceNumGet(void)
{
	return DecoderSourecNum;
}

void DecoderSourceNumSet(uint8_t Num)
{
	DecoderSourecNum = Num;
}


