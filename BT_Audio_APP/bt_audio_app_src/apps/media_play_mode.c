/**
 **************************************************************************************
 * @file    media_play_mode.c
 * @brief   
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2017-12-28 18:00:00$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <string.h>
#include "type.h"
#include "irqn.h"
#include "app_config.h"
#include "app_message.h"
#include "chip_info.h"
#include "dac.h"
#include "gpio.h"
#include "dma.h"
#include "dac.h"
#include "audio_adc.h"
#include "rtos_api.h"
#include "adc_interface.h"
#include "dac_interface.h"
#include "debug.h"
#include "audio_effect.h"
#include "recorder_service.h"
#include "main_task.h"
#include "audio_core_api.h"
#include "media_play_mode.h"
#include "decoder_service.h"
#include "device_service.h"
#include "audio_core_service.h"
#include "media_play_api.h"
#include "mode_switch_api.h"
#include "remind_sound_service.h"
#include "powercontroller.h"
#include "deepsleep.h"
#include "backup_interface.h"
#include "breakpoint.h"
#include "otg_device_hcd.h"
#include "otg_device_standard_request.h"
#include "ctrlvars.h"
#include "audio_vol.h"
#include "timeout.h"
#include "ffpresearch.h"
#include "browser_parallel.h"
#include "browser_tree.h"

extern uint32_t CmdErrCnt;

#if defined(CFG_APP_USB_PLAY_MODE_EN) || defined(CFG_APP_CARD_PLAY_MODE_EN) || defined(CFG_FUNC_RECORDER_EN)

void SendPreviewPlayNextMsg(void);
static void MediaPlayRunning(uint16_t msgId);
#ifdef CFG_FUNC_RECORDER_EN
static void MediaRecEnter(void);
#endif
#define MEDIA_PLAY_TASK_STACK_SIZE				768//512//
#define MEDIA_PLAY_TASK_PRIO					3
#define MEDIA_PLAY_NUM_MESSAGE_QUEUE			10
#define MEDIA_PLAY_DECODER_SOURCE_NUM			APP_SOURCE_NUM
//osMutexId SDIOMutex = NULL;
//osMutexId UDiskMutex = NULL;

typedef struct _mediaPlayContext
{
	xTaskHandle 		taskHandle;
	MessageHandle		msgHandle;
	MessageHandle		parentMsgHandle;

	TaskState			state;
	DEV_ID				Device;	//当前播放的设备

	QueueHandle_t 		audioMutex;
	QueueHandle_t		pcmBufMutex;

	uint8_t				SourceNum;//播放器和回放使用不同通道，前者有音效
	uint16_t 			*SourceDecoder;
	AudioCoreContext 	*AudioCoreMediaPlay;

	//play
	uint32_t 			SampleRate;
#if	defined(CFG_FUNC_REMIND_SOUND_EN)	//没有播放器模式提示音时可去除此项。
	bool				ModeRemindWait;
#endif

#ifdef CFG_FUNC_RECORDER_EN
		TaskState			RecorderSync;
#endif
		TaskState 			DecoderSync;
	//used Service 
}MediaPlayContext;


/**根据appconfig缺省配置:DMA 8个通道配置**/
/*1、cec需PERIPHERAL_ID_TIMER3*/
/*2、SD卡录音需PERIPHERAL_ID_SDIO RX/TX*/
/*3、在线串口调音需PERIPHERAL_ID_UART1 RX/TX,建议使用USB HID，节省DMA资源*/
/*4、线路输入需PERIPHERAL_ID_AUDIO_ADC0_RX*/
/*5、Mic开启需PERIPHERAL_ID_AUDIO_ADC1_RX，mode之间通道必须一致*/
/*6、Dac0开启需PERIPHERAL_ID_AUDIO_DAC0_TX mode之间通道必须一致*/
/*7、DacX需开启PERIPHERAL_ID_AUDIO_DAC1_TX mode之间通道必须一致*/
/*注意DMA 8个通道配置冲突:*/
/*a、UART在线调音和DAC-X有冲突,默认在线调音使用USB HID*/
/*b、UART在线调音与HDMI/SPDIF模式冲突*/
static const uint8_t DmaChannelMap[29] = {
	255,//PERIPHERAL_ID_SPIS_RX = 0,	//0
	255,//PERIPHERAL_ID_SPIS_TX,		//1
#ifdef CFG_APP_HDMIIN_MODE_EN
	5,//PERIPHERAL_ID_TIMER3,			//2
#else
	255,//PERIPHERAL_ID_TIMER3,			//2
#endif
	4,//PERIPHERAL_ID_SDIO_RX,			//3
	4,//PERIPHERAL_ID_SDIO_TX,			//4
	255,//PERIPHERAL_ID_UART0_RX,		//5
	255,//PERIPHERAL_ID_TIMER1,			//6
	255,//PERIPHERAL_ID_TIMER2,			//7
	255,//PERIPHERAL_ID_SDPIF_RX,		//8 SPDIF_RX /TX same chanell
	255,//PERIPHERAL_ID_SDPIF_TX,		//8 SPDIF_RX /TX same chanell
	255,//PERIPHERAL_ID_SPIM_RX,		//9
	255,//PERIPHERAL_ID_SPIM_TX,		//10
	255,//PERIPHERAL_ID_UART0_TX,		//11
	
#ifdef CFG_COMMUNICATION_BY_UART	
	7,//PERIPHERAL_ID_UART1_RX,			//12
	6,//PERIPHERAL_ID_UART1_TX,			//13
#else
	255,//PERIPHERAL_ID_UART1_RX,		//12
	255,//PERIPHERAL_ID_UART1_TX,		//13
#endif

	255,//PERIPHERAL_ID_TIMER4,			//14
	255,//PERIPHERAL_ID_TIMER5,			//15
	255,//PERIPHERAL_ID_TIMER6,			//16
	0,//PERIPHERAL_ID_AUDIO_ADC0_RX,	//17
	1,//PERIPHERAL_ID_AUDIO_ADC1_RX,	//18
	2,//PERIPHERAL_ID_AUDIO_DAC0_TX,	//19
	3,//PERIPHERAL_ID_AUDIO_DAC1_TX,	//20
	255,//PERIPHERAL_ID_I2S0_RX,		//21
#if	(defined(CFG_RES_AUDIO_I2SOUT_EN )&&(CFG_RES_I2S_PORT==0))
	7,//PERIPHERAL_ID_I2S0_TX,			//22
#else	
	255,//PERIPHERAL_ID_I2S0_TX,		//22
#endif	
	255,//PERIPHERAL_ID_I2S1_RX,		//23
#if	(defined(CFG_RES_AUDIO_I2SOUT_EN )&&(CFG_RES_I2S_PORT==1))
	7,	//PERIPHERAL_ID_I2S1_TX,		//24
#else
	255,//PERIPHERAL_ID_I2S1_TX,		//24
#endif
	255,//PERIPHERAL_ID_PPWM,			//25
	255,//PERIPHERAL_ID_ADC,     		//26
	255,//PERIPHERAL_ID_SOFTWARE,		//27
};

static MediaPlayContext*	MediaPlayCt;


void MediaPlayResRelease(void)
{
	if(MediaPlayCt->SourceDecoder != NULL)
	{
		APP_DBG("SourceDecoder\n");
		osPortFree(MediaPlayCt->SourceDecoder);
		MediaPlayCt->SourceDecoder = NULL;
	}
}

bool MediaPlayResMalloc(uint16_t SampleLen)
{
	MediaPlayCt->SourceDecoder = (uint16_t*)osPortMallocFromEnd(SampleLen * 2 * 2);//2K
	if(MediaPlayCt->SourceDecoder == NULL)
	{
		return FALSE;
	}
	memset(MediaPlayCt->SourceDecoder, 0, SampleLen * 2 * 2);//2K
	return TRUE;
}

void MediaPlayResInit(void)
{
	if(MediaPlayCt->SourceDecoder != NULL)
	{
		MediaPlayCt->AudioCoreMediaPlay->AudioSource[MediaPlayCt->SourceNum].PcmInBuf = (int16_t *)MediaPlayCt->SourceDecoder;
	}
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(MediaPlayCt->SourceDecoder != NULL)
	{
		MediaPlayCt->AudioCoreMediaPlay->AudioSource[REMIND_SOURCE_NUM].PcmInBuf = (int16_t *)MediaPlayCt->SourceDecoder;
	}
#endif
}

/**
 * @func        MediaPlayInit
 * @brief       Media模式参数配置，资源初始化
 * @param       MessageHandle parentMsgHandle  
 * @Output      None
 * @return      bool
 * @Others      任务块、Dac、AudioCore配置，数据源自DecoderService
 * @Others      数据流从Decoder到audiocore配有函数指针，audioCore到Dac同理，由audiocoreService任务按需驱动
 * Record
 */
static bool MediaPlayInit(MessageHandle parentMsgHandle)
{
	//DMA channel
	DMA_ChannelAllocTableSet((uint8_t*)DmaChannelMap);

	MediaPlayCt = (MediaPlayContext*)osPortMalloc(sizeof(MediaPlayContext));
	if(MediaPlayCt == NULL)
	{
		return FALSE;
	}
	
	memset(MediaPlayCt, 0, sizeof(MediaPlayContext));
	MediaPlayCt->msgHandle = MessageRegister(MEDIA_PLAY_NUM_MESSAGE_QUEUE);
	if(MediaPlayCt->msgHandle == NULL)
	{
		return FALSE;
	}
	MediaPlayCt->parentMsgHandle = parentMsgHandle;
	MediaPlayCt->state = TaskStateCreating;
	/* Create media audio services */

	MediaPlayCt->SampleRate = CFG_PARA_SAMPLE_RATE;//默认初始化采样率

	MediaPlayCt->AudioCoreMediaPlay = (AudioCoreContext*)&AudioCore;

	if(!MediaPlayResMalloc(mainAppCt.SamplesPreFrame))
	{
		APP_DBG("MediaPlayResMalloc Res Error!\n");
		return FALSE;
	}

	//Audio init
	//Soure.
#ifdef CFG_FUNC_RECORDER_EN
#ifdef CFG_FUNC_REMIND_SOUND_EN
	SoftFlagDeregister(SoftFlagMediaPlayRecRemind);
#endif
	SoftFlagDeregister(SoftFlagMediaRectate);
	if(GetSystemMode() == AppModeCardPlayBack || GetSystemMode() == AppModeUDiskPlayBack || GetSystemMode() == AppModeFlashFsPlayBack)
	{
		MediaPlayCt->SourceNum = PLAYBACK_SOURCE_NUM;
	}
	else
#endif
	{
		MediaPlayCt->SourceNum = MEDIA_PLAY_DECODER_SOURCE_NUM;
	}
	DecoderSourceNumSet(MediaPlayCt->SourceNum);
	MediaPlayCt->AudioCoreMediaPlay->AudioSource[MediaPlayCt->SourceNum].Enable = 0;
	MediaPlayCt->AudioCoreMediaPlay->AudioSource[MediaPlayCt->SourceNum].FuncDataGet = DecodedPcmDataGet;
	MediaPlayCt->AudioCoreMediaPlay->AudioSource[MediaPlayCt->SourceNum].FuncDataGetLen = NULL;
	MediaPlayCt->AudioCoreMediaPlay->AudioSource[MediaPlayCt->SourceNum].IsSreamData = FALSE;//Decoder
	MediaPlayCt->AudioCoreMediaPlay->AudioSource[MediaPlayCt->SourceNum].PcmFormat = 2; //stereo
	MediaPlayCt->AudioCoreMediaPlay->AudioSource[MediaPlayCt->SourceNum].PcmInBuf = (int16_t *)MediaPlayCt->SourceDecoder;
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	MediaPlayCt->AudioCoreMediaPlay->AudioSource[REMIND_SOURCE_NUM].Enable = 0;
	MediaPlayCt->AudioCoreMediaPlay->AudioSource[REMIND_SOURCE_NUM].FuncDataGet = DecodedPcmDataGet;
	MediaPlayCt->AudioCoreMediaPlay->AudioSource[REMIND_SOURCE_NUM].FuncDataGetLen = NULL;
	MediaPlayCt->AudioCoreMediaPlay->AudioSource[REMIND_SOURCE_NUM].IsSreamData = FALSE;//Decoder
	MediaPlayCt->AudioCoreMediaPlay->AudioSource[REMIND_SOURCE_NUM].PcmFormat = 2; //stereo
	MediaPlayCt->AudioCoreMediaPlay->AudioSource[REMIND_SOURCE_NUM].PcmInBuf = (int16_t *)MediaPlayCt->SourceDecoder;
#endif
	
#ifdef CFG_FUNC_AUDIO_EFFECT_EN
    #ifdef CFG_FUNC_MIC_KARAOKE_EN
	MediaPlayCt->AudioCoreMediaPlay->AudioEffectProcess = (AudioCoreProcessFunc)AudioEffectProcess;
	#else
	MediaPlayCt->AudioCoreMediaPlay->AudioEffectProcess = (AudioCoreProcessFunc)AudioMusicProcess;
	#endif
#else
	MediaPlayCt->AudioCoreMediaPlay->AudioEffectProcess = (AudioCoreProcessFunc)AudioBypassProcess;
#endif
#ifdef CFG_FUNC_RECORD_FLASHFS
	if(GetSystemMode() == AppModeFlashFsPlayBack)
	{
		MediaPlayCt->Device = DEV_ID_FLASHFS;
	}
	else
#endif
	{
		MediaPlayCt->Device = (GetSystemMode() == AppModeUDiskAudioPlay || GetSystemMode() == AppModeUDiskPlayBack) ? DEV_ID_USB : DEV_ID_SD;
	}
	APP_DBG("Dev %d\n", MediaPlayCt->Device);

	//播放控制结构体提前申请
	if(gpMediaPlayer != NULL)
	{
		APP_DBG("player is reopened\n");
	}
	else
	{
		gpMediaPlayer = osPortMalloc(sizeof(MEDIA_PLAYER));
		if(gpMediaPlayer == NULL)
		{
			APP_DBG("gpMediaPlayer malloc error\n");
			return FALSE;
		}
	}
	
	memset(gpMediaPlayer, 0, sizeof(MEDIA_PLAYER));
#ifdef	FUNC_SPECIFY_FOLDER_PLAY_EN
	gpMediaPlayer->StoryPlayByIndexFlag=TRUE;
	StoryVarInit();
#endif
	gpMediaPlayer->AccRamBlk = (uint8_t*)osPortMalloc(MAX_ACC_RAM_SIZE);
	if(gpMediaPlayer->AccRamBlk == NULL)
	{
		APP_DBG("AccRamBlk malloc error\n");
		return FALSE;
	}

#if defined(CFG_FUNC_BREAKPOINT_EN) && (defined(CFG_APP_USB_PLAY_MODE_EN) || defined(CFG_APP_CARD_PLAY_MODE_EN))
	if(GetSystemMode() == AppModeUDiskAudioPlay || GetSystemMode() == AppModeCardAudioPlay)
	{
		gpMediaPlayer->CurPlayMode = BPDiskSongPlayModeGet();
	}
#endif	
#if	defined(CFG_FUNC_REMIND_SOUND_EN)	//没有播放器模式提示音时可去除此项。
	MediaPlayCt->ModeRemindWait = FALSE;
#endif
#ifdef FUNC_BROWSER_PARALLEL_EN
	BrowserVarDefaultSet();
#endif

#ifdef CFG_FUNC_RECORDER_EN
	MediaPlayCt->RecorderSync = TaskStateNone;
#endif
	MediaPlayCt->DecoderSync=TaskStateNone;
	return TRUE;
}

//创建从属services
static void MediaPlayModeCreate(void)
{
#ifdef CFG_FUNC_RECORDER_EN
	if(GetSystemMode() == AppModeCardPlayBack || GetSystemMode() == AppModeUDiskPlayBack || GetSystemMode() ==AppModeFlashFsPlayBack)
	{
		DecoderServiceCreate(MediaPlayCt->msgHandle, DECODER_BUF_SIZE_MP3, DECODER_FIFO_SIZE_FOR_MP3);//DECODER_BUF_SIZE_MP3
	}
	else
#endif
	{
		DecoderServiceCreate(MediaPlayCt->msgHandle, DECODER_BUF_SIZE, DECODER_FIFO_SIZE_FOR_PLAYER);
	}
	MediaPlayCt->DecoderSync=TaskStateCreating;
}

//All of services is created
//Send CREATED message to parent
static void MediaPlayModeCreating(uint16_t msgId)
{
	MessageContext		msgSend;

	if(msgId == MSG_DECODER_SERVICE_CREATED)
	{
		APP_DBG("Decoder service created\n");
		MediaPlayCt->state = TaskStateReady;
		msgSend.msgId		= MSG_MEDIA_PLAY_MODE_CREATED;
		MessageSend(MediaPlayCt->parentMsgHandle, &msgSend);
	}
}

static void MediaPlayModeStart(void)
{
	DecoderServiceStart();
	MediaPlayCt->state = TaskStateStarting;
}

static void MediaPlayerStart()
{
	MessageContext		msgSend;
	if(!MediaPlayerInitialize(MediaPlayCt->Device, 1, 1))//初始化设备
	{
		msgSend.msgId		= MSG_MEDIA_PLAY_MODE_FAILURE;
		MessageSend(MediaPlayCt->parentMsgHandle, &msgSend);
		//避免设备故障的模式，反复进入后失败退出。特别是只有一个可用模式时。
		if(GetSystemMode() == AppModeCardAudioPlay)
		{
			ResourceDeregister(AppResourceCardForPlay);
		}
		else if(GetSystemMode() == AppModeUDiskAudioPlay)
		{
			ResourceDeregister(AppResourceUDiskForPlay);
		}
		return;
	}
	else
	{
		AudioCoreSourceEnable(MediaPlayCt->SourceNum);
		AudioCoreSourceUnmute(MediaPlayCt->SourceNum, TRUE, TRUE);
		DecoderPlay();

#ifdef CFG_FUNC_DISPLAY_EN
	msgSend.msgId = MSG_DISPLAY_SERVICE_FILE_NUM;
	MessageSend(GetDisplayMessageHandle(), &msgSend);
#endif
	}
}

static void MediaPlayModeStarting(uint16_t msgId)
{
	MessageContext		msgSend;
	if(msgId == MSG_DECODER_SERVICE_STARTED)
	{
		APP_DBG("Decoder service started\n");

		MediaPlayCt->state = TaskStateRunning;
		msgSend.msgId		= MSG_MEDIA_PLAY_MODE_STARTED;
		MessageSend(MediaPlayCt->parentMsgHandle, &msgSend);
		
#ifdef CFG_FUNC_REMIND_SOUND_EN
		if(GetSystemMode() == AppModeCardAudioPlay)
		{
			if(!SoftFlagGet(SoftFlagRemindMask))
			{
				MediaPlayCt->ModeRemindWait = RemindSoundServiceItemRequest(SOUND_REMIND_CARDMODE, FALSE);
			}
			else
			{
				SoftFlagDeregister(SoftFlagRemindMask);
				if(SoftFlagGet(SoftFlagDiscDelayMask))
				{
					SoftFlagDeregister(SoftFlagDiscDelayMask);
					RemindSoundServiceItemRequest(SOUND_REMIND_DISCONNE, FALSE);
				}
			}
		}
		else if(GetSystemMode() == AppModeUDiskAudioPlay)
		{
			if(!SoftFlagGet(SoftFlagRemindMask))
			{
				MediaPlayCt->ModeRemindWait = RemindSoundServiceItemRequest(SOUND_REMIND_UPANMODE, FALSE);
			}
			else
			{
				SoftFlagDeregister(SoftFlagRemindMask);
				if(SoftFlagGet(SoftFlagDiscDelayMask))
				{
					SoftFlagDeregister(SoftFlagDiscDelayMask);
					RemindSoundServiceItemRequest(SOUND_REMIND_DISCONNE, FALSE);
				}
			}
		}

		if(MediaPlayCt->ModeRemindWait)
		{
			MediaPlayCt->state = TaskStatePaused;//提示音先播放，再（阻塞式)初始化设备,播放器进程暂停等提示音反馈。
		}
		else
#endif
		{
			MediaPlayerStart();
		}
	}
}

static void MediaPlayModeStop(void)
{
#ifdef CFG_FUNC_RECORDER_EN
	if(MediaPlayCt->RecorderSync != TaskStateNone)
	{//此service 随用随Kill
		APP_DBG("media rec stop\n");
		MediaRecorderServiceStop();
		MediaPlayCt->RecorderSync = TaskStateStopping;
		//NoService = FALSE;
	}
#endif

	DecoderServiceStop();
	MediaPlayCt->DecoderSync=TaskStateStopping;

	MediaPlayCt->state = TaskStateStopping;
}

static void MediaPlayModeStopping(uint16_t msgId)
{
	MessageContext		msgSend;
	
	if(msgId == MSG_DECODER_SERVICE_STOPPED)
	{
		MediaPlayCt->DecoderSync = TaskStateNone;
	}
#ifdef CFG_FUNC_RECORDER_EN
	else if(msgId == MSG_MEDIA_RECORDER_SERVICE_STOPPED)
	{
		MediaPlayCt->RecorderSync = TaskStateNone;
		APP_DBG("RecorderKill");
		MediaRecorderServiceKill();
	}
#endif

	if(MediaPlayCt->DecoderSync == TaskStateNone 
#ifdef CFG_FUNC_RECORDER_EN
				&& (MediaPlayCt->RecorderSync == TaskStateNone)
#endif
	&& MediaPlayCt->state == TaskStateStopping)
	{
		APP_DBG("Decoder service Stopped\n");
		//Set para

		//clear msg
		MessageClear(MediaPlayCt->msgHandle);

		//Set state
		MediaPlayCt->state = TaskStateStopped;

		//reply
		msgSend.msgId = MSG_MEDIA_PLAY_MODE_STOPPED;
		MessageSend(MediaPlayCt->parentMsgHandle, &msgSend);
	}
}
#ifdef DEL_REC_FILE_EN
extern void DelRecFile(void);
#endif

static void MediaPlayEntrance(void * param)
{
	MessageContext		msgRecv;
#ifdef CFG_FUNC_DISPLAY_EN
    MessageContext      msgSend;
#endif

	MediaPlayModeCreate();
#ifdef CFG_RES_IR_NUMBERKEY
	Number_select_flag = 0;
	Number_value = 0;
#endif
#ifdef CFG_FUNC_AUDIO_EFFECT_EN	
	AudioEffectModeSel(mainAppCt.EffectMode, 2);//0=init hw,1=effect,2=hw+effect
#ifdef CFG_COMMUNICATION_BY_UART
	UART1_Communication_Init((void *)(&UartRxBuf[0]), 1024, (void *)(&UartTxBuf[0]), 1024);
#endif
#endif

	APP_DBG("App\n");

	//模式进来之后进行保存一次断点
#ifdef CFG_FUNC_BREAKPOINT_EN
	if(GetSystemMode() == AppModeCardAudioPlay || GetSystemMode() == AppModeUDiskAudioPlay)
	{
		BackupInfoUpdata(BACKUP_SYS_INFO);
	}
#endif

//	//如果进入模式AudioCore为静音状态，则unmute
//	if(IsAudioPlayerMute() == TRUE)
//	{
//		AudioPlayerMute();
//	}
#if (CFG_RES_MIC_SELECT) && defined(CFG_FUNC_AUDIO_EFFECT_EN)
	AudioCoreSourceUnmute(MIC_SOURCE_NUM, TRUE, TRUE);
#endif
	while(1)
	{
#ifdef CFG_RES_IR_NUMBERKEY
		MessageRecv(MediaPlayCt->msgHandle, &msgRecv, 100);
#else
		MessageRecv(MediaPlayCt->msgHandle, &msgRecv, MAX_RECV_MSG_TIMEOUT);
#endif
		switch(msgRecv.msgId)//此段代码禁止新增提示音
		{
			case MSG_DECODER_SERVICE_CREATED:
				MediaPlayModeCreating(msgRecv.msgId);
				break;

			case MSG_TASK_START:
				MediaPlayModeStart();
				break;

			case MSG_DECODER_SERVICE_STARTED:
				MediaPlayModeStarting(msgRecv.msgId);
				break;

			case MSG_DECODER_STOPPED:
				MediaPlaySetDecoderState(DecoderStateNone);//设已停止 状态
#ifdef CFG_FUNC_RECORDER_EN		
				if(SoftFlagGet(SoftFlagMediaToRecMode))
				{
					SoftFlagDeregister(SoftFlagMediaToRecMode);
					SoftFlagRegister(SoftFlagMediaRectate);
					
#ifdef CFG_FUNC_REMIND_SOUND_EN
					if(RemindSoundServiceItemRequest(SOUND_REMIND_LUYIN, FALSE))
					{
						SoftFlagRegister(SoftFlagMediaPlayRecRemind);
					}
					osTaskDelay(350);//
#endif
					MediaRecEnter();
					break;
				}
#endif
#ifndef CFG_FUNC_REMIND_SOUND_EN
#ifdef FUNC_BROWSER_PARALLEL_EN
				if(GetMediaPlayMode() == PLAY_MODE_BROWSER && GetBrowserPlay_state() == Browser_Play_Normal)
				{
					APP_DBG("browser song refresh\n");
					MediaPlayerSongBrowserRefresh();
				}
#ifdef DEL_REC_FILE_EN
				else if(SoftFlagGet(SoftFlagDecRecFile))
				{
					DelRecFile();
					MediaPlayerPlayPause();
					SoftFlagDeregister(SoftFlagDecRecFile);
				}
#endif
				else
#else
#ifdef DEL_REC_FILE_EN
				if(SoftFlagGet(SoftFlagDecRecFile))
				{
					DelRecFile();
					MediaPlayerPlayPause();
					SoftFlagDeregister(SoftFlagDecRecFile);
				}
				else
#endif
#endif
				{
					if(!SoftFlagGet(SoftFlagDecoderSwitch) && MediaPlayCt->state == TaskStateRunning)//&&(!SoftFlagGet(SoftFlagDecRepeatoffEndPlay)))
					{
						//MediaPlayerSongRefresh();
						if((GetMediaPlayerState() != PLAYER_STATE_IDLE)
						&& (GetMediaPlayerState() != PLAYER_STATE_STOP))
						{
							APP_DBG("MediaPlayerSongRefresh()");
							//MediaPlayerSongRefresh();
							if(!MediaPlayerSongRefresh())
							{
								MessageContext		msgSend;

								msgSend.msgId		= MSG_NEXT;
								MessageSend(MediaPlayCt->msgHandle, &msgSend);
							}
						}
						else
						{
							APP_DBG("Media Player State is stop\n");
						}
					 }
				}
				break;
#else
				if(SoftFlagGet(SoftFlagDecoderRemind))
				{
					MessageSend(GetRemindSoundServiceMessageHandle(), &msgRecv);
				}
#ifdef FUNC_BROWSER_PARALLEL_EN
				else if(GetMediaPlayMode() == PLAY_MODE_BROWSER && GetBrowserPlay_state() == Browser_Play_Normal)
				{
					APP_DBG("browser song refresh\n");
					MediaPlayerSongBrowserRefresh();
				}
#endif
#ifdef DEL_REC_FILE_EN
				else if(SoftFlagGet(SoftFlagDecRecFile))
				{
					DelRecFile();
					MediaPlayerPlayPause();
					SoftFlagDeregister(SoftFlagDecRecFile);
				}
#endif
				else if(!SoftFlagGet(SoftFlagDecoderSwitch) && MediaPlayCt->state == TaskStateRunning)
				{
					APP_DBG("others song refresh\n");
					//MediaPlayerSongRefresh();
					if((GetMediaPlayerState() != PLAYER_STATE_IDLE)
					&& (GetMediaPlayerState() != PLAYER_STATE_STOP))
					{
						APP_DBG("MediaPlayerSongRefresh()");
						if(!MediaPlayerSongRefresh())
						{
							if(gpMediaPlayer->SongSwitchFlag == TRUE)
							{
								MessageContext		msgSend;

								msgSend.msgId		= MSG_PRE;
								MessageSend(GetAppMessageHandle(), &msgSend);
							}
							else
							{
								MessageContext		msgSend;

								msgSend.msgId		= MSG_NEXT;
								MessageSend(GetAppMessageHandle(), &msgSend);
							}
						}
					}
					else
					{
						APP_DBG("Media Player State is stop\n");
					}
				}
				break;

			case MSG_REMIND_SOUND_NEED_DECODER:
				APP_DBG("Stop for remind\n");
				if(MediaPlayCt->ModeRemindWait)
				{
					APP_DBG("first remind, decoder switch out\n");
					AudioCoreSourceMute(MediaPlayCt->SourceNum, TRUE, TRUE);
					AudioCoreSourceDisable(MediaPlayCt->SourceNum);
					SoftFlagRegister(SoftFlagDecoderRemind);//出让解码器
					RemindSoundServicePlay();
				}
				else if((!SoftFlagGet(SoftFlagDecoderMask & ~SoftFlagDecoderApp)) && MediaPlayCt->state == TaskStateRunning)					
				{
					AudioCoreSourceMute(MediaPlayCt->SourceNum, TRUE, TRUE);
					SoftFlagRegister(SoftFlagDecoderSwitch);
					DecoderReset();//发起解码器复位，准备出让。
				}
				break;

			case MSG_REMIND_SOUND_PLAY_REQUEST_FAIL:
				if(MediaPlayCt->state == TaskStateRunning)//非running时，考虑提示音请求失败，继续设备init。
				{
					break;
				}
			case MSG_DECODER_RESET://解码器回收。
			if(MediaPlayCt->ModeRemindWait)
				{
					APP_DBG("first remind, decoder switch in\n");
					MediaPlaySetDecoderState(DecoderStateNone);
					DecoderSourceNumSet(MediaPlayCt->SourceNum);
					SoftFlagDeregister(SoftFlagDecoderRemind);//回收解码器
					SoftFlagDeregister(SoftFlagDecoderSwitch);
					MediaPlayCt->ModeRemindWait = FALSE;
					MediaPlayerStart();
					MediaPlayCt->state = TaskStateRunning;
				}
				else if(MediaPlayCt->state == TaskStateRunning)
				{
					MediaPlaySetDecoderState(DecoderStateNone);
					if(SoftFlagGet(SoftFlagDecoderSwitch))//只有app使用解码
					{
						APP_DBG("decoder switch out\n");
						AudioCoreSourceDisable(MediaPlayCt->SourceNum);
						SoftFlagRegister(SoftFlagDecoderRemind);//出让解码器
						RemindSoundServicePlay();
					}
					else//非app使用解码器时 回收
					{
						DecoderSourceNumSet(MediaPlayCt->SourceNum);
						SoftFlagDeregister(SoftFlagDecoderRemind);//回收解码器
		//				AudioCoreSourceUnmute(MediaPlayCt->SourceNum, TRUE, TRUE);
#ifdef CFG_FUNC_RECORDER_EN
						if(!SoftFlagGet(SoftFlagMediaRectate))
#endif
						{
							APP_DBG("112 MSG_DECODER_RESET \n");
							if((GetMediaPlayerState() != PLAYER_STATE_IDLE)
							&& (GetMediaPlayerState() != PLAYER_STATE_STOP))
							{
								APP_DBG("MediaPlayerSongRefresh()");
								//MediaPlayerSongRefresh();
								if(!MediaPlayerSongRefresh())
								{
									if(gpMediaPlayer->SongSwitchFlag == TRUE)
									{
										MessageContext		msgSend;

										msgSend.msgId		= MSG_PRE;
										MessageSend(GetAppMessageHandle(), &msgSend);
									}
									else
									{
										MessageContext		msgSend;

										msgSend.msgId		= MSG_NEXT;
										MessageSend(GetAppMessageHandle(), &msgSend);
									}
								}
							}
							else
							{
								APP_DBG("Media Player State is stop\n");
							}
						}
					}
					SoftFlagDeregister(SoftFlagDecoderSwitch);
				}
				break;
#endif

			case MSG_TASK_STOP:
#if 0//CFG_COMMUNICATION_BY_USB
				if(GetSystemMode() != AppModeUDiskAudioPlay)
				{
					NVIC_DisableIRQ(Usb_IRQn);
					OTG_DeviceDisConnect();
				}
#endif
#ifdef CFG_FUNC_REMIND_SOUND_EN
				if(MediaPlayCt->ModeRemindWait || MediaPlayCt->state == TaskStateRunning)
				{
					RemindSoundServiceReset();
				}
#endif
				MediaPlayModeStop();
				break;
			case MSG_DECODER_SERVICE_STOPPED:
#ifdef CFG_FUNC_RECORDER_EN			
			case MSG_MEDIA_RECORDER_SERVICE_STOPPED:
#endif
				MediaPlayModeStopping(msgRecv.msgId);
#ifdef CFG_FUNC_RECORDER_EN
				if(MediaPlayCt->RecorderSync == TaskStateNone)
				{
				if(SoftFlagGet(SoftFlagMediaRectate))
					{
					APP_DBG("RecorderKill\n");
					MediaRecorderServiceKill();
					vTaskDelay(50);//repeat play after rec
					MediaPlaySetDecoderState(DecoderStateNone);
					DecoderSourceNumSet(MediaPlayCt->SourceNum);
					SoftFlagDeregister(SoftFlagDecoderRemind);//回收解码器
					SoftFlagDeregister(SoftFlagDecoderSwitch);
					SoftFlagRegister(SoftFlagDecoderApp);
#ifdef CFG_FUNC_REMIND_SOUND_EN
					MediaPlayCt->ModeRemindWait = FALSE;
					RemindSoundServiceItemRequest(SOUND_REMIND_STOPREC, FALSE);
					osTaskDelay(350);
#endif

					SoftFlagDeregister(SoftFlagMediaRectate);
					
					MediaPlayerStart();
					MediaPlayCt->state = TaskStateRunning;
					
					}
				}
#endif	
				break;

			case MSG_APP_RES_RELEASE:
				MediaPlayResRelease();
				{
					MessageContext		msgSend;
					msgSend.msgId		= MSG_APP_RES_RELEASE_SUC;
					MessageSend(GetMainMessageHandle(), &msgSend);
				}
				break;
			case MSG_APP_RES_MALLOC:
				MediaPlayResMalloc(mainAppCt.SamplesPreFrame);
				{
					MessageContext		msgSend;
					msgSend.msgId		= MSG_APP_RES_MALLOC_SUC;
					MessageSend(GetMainMessageHandle(), &msgSend);
				}
				break;
			case MSG_APP_RES_INIT:
				MediaPlayResInit();
				{
					MessageContext		msgSend;
					msgSend.msgId		= MSG_APP_RES_INIT_SUC;
					MessageSend(GetMainMessageHandle(), &msgSend);
				}
				break;

			case MSG_REMIND_SOUND_PLAY_START:
				APP_DBG("MSG_REMIND_SOUND_PLAY_START: \n");
				break;
					
			case MSG_REMIND_SOUND_PLAY_DONE://提示音播放结束
				APP_DBG("MSG_REMIND_SOUND_PLAY_DONE: \n");
#ifdef CFG_FUNC_RECORDER_EN	
#ifdef	CFG_FUNC_REMIND_SOUND_EN
				SoftFlagDeregister(SoftFlagMediaPlayRecRemind);
#endif
#endif
				//AudioCoreSourceUnmute(APP_SOURCE_NUM, TRUE, TRUE);
				break;
#ifdef CFG_FUNC_RECORDER_EN	
			case MSG_REC:
				if(GetSystemMode() == AppModeCardPlayBack || GetSystemMode() == AppModeUDiskPlayBack || GetSystemMode() == AppModeFlashFsPlayBack)
					break;				
#ifdef	CFG_FUNC_RECORD_SD_UDISK
				if(ResourceValue(AppResourceCard) || ResourceValue(AppResourceUDisk))
				{			

					if(MediaPlayCt->RecorderSync == TaskStateNone)
					{
						if(!MediaRecordHeapEnough())
						{
							break;
						}
						SetMediaPlayerState(PLAYER_STATE_STOP);
						MediaPlayerDecoderStop();
						SoftFlagRegister(SoftFlagMediaToRecMode);
					}
					else if(MediaPlayCt->RecorderSync == TaskStateRunning)//再按录音键 停止
					{
#ifdef	CFG_FUNC_REMIND_SOUND_EN
						if(!SoftFlagGet(SoftFlagMediaPlayRecRemind))
#endif
						{
							MediaRecorderStop();
							MediaRecorderServiceStop();
							MediaPlayCt->RecorderSync = TaskStateStopping;
						}
					}
				}
#elif defined(CFG_FUNC_RECORD_FLASHFS)	
				if(MediaPlayCt->RecorderSync == TaskStateNone)
				 {
					SetMediaPlayerState(PLAYER_STATE_STOP);
					MediaPlayerDecoderStop();
					SoftFlagRegister(SoftFlagMediaToRecMode);
				 } 
				else if(MediaPlayCt->RecorderSync == TaskStateRunning)//再按录音键 停止
				{
					MediaRecorderStop();
					MediaRecorderServiceStop();
					MediaPlayCt->RecorderSync = TaskStateStopping;
				}
#endif
				
			break;
			
			case MSG_MEDIA_RECORDER_SERVICE_CREATED:
				MediaPlayCt->RecorderSync = TaskStateStarting;
				MediaRecorderServiceStart();	
				break;

			case MSG_MEDIA_RECORDER_SERVICE_STARTED:
				MediaRecorderRun();
				MediaPlayCt->RecorderSync = TaskStateRunning;
				break;

			case MSG_MEDIA_RECORDER_STOPPED:
				MediaRecorderServiceStop();
				MediaPlayCt->RecorderSync = TaskStateStopping;
				break;

			case MSG_MEDIA_RECORDER_ERROR:
				if(MediaPlayCt->RecorderSync == TaskStateRunning)
				{
					MediaRecorderStop();
					MediaRecorderServiceStop();
					MediaPlayCt->RecorderSync = TaskStateStopping;
				}
				break;
#endif 

			case MSG_PLAY_PAUSE:
#ifdef CFG_FUNC_RECORDER_EN	
			if(GetMediaRecorderMessageHandle() !=  NULL)
			{
				EncoderServicePause();
				break;
			}
#endif		
			if(MediaPlayCt->state != TaskStateRunning)
				break;
			APP_DBG("MSG_PLAY_PAUSE\n");
			MediaPlayerPlayPause();
#ifdef CFG_FUNC_DISPLAY_EN
			msgSend.msgId = MSG_DISPLAY_SERVICE_MEDIA;
			MessageSend(GetDisplayMessageHandle(), &msgSend);
#endif
			break;

				
			default:
				if(MediaPlayCt->state == TaskStateRunning
#ifdef CFG_FUNC_RECORDER_EN
						&&(!SoftFlagGet(SoftFlagMediaRectate))
#endif
						)
				{
				
				MediaPlayRunning(msgRecv.msgId);
#if (defined(FUNC_BROWSER_PARALLEL_EN) || defined(FUNC_BROWSER_TREE_EN))
#ifdef CFG_FUNC_RECORDER_EN
				if(!SoftFlagGet(SoftFlagMediaRectate))
#endif
				{
					BrowserMsgProcess(msgRecv.msgId);
				}

#endif
				}
				break;
		}
	}
}


static void MediaPlayRunning(uint16_t msgId)
{
#ifdef CFG_FUNC_DISPLAY_EN
	MessageContext	msgSend;
#endif
	switch(msgId)
	{

#ifdef	CFG_FUNC_POWERKEY_EN
		case MSG_TASK_POWERDOWN:
			APP_DBG("PowerDown, Please breakpoint\n");
			SystemPowerDown();
			break;
#endif

		case MSG_REPEAT:
			APP_DBG("MSG_REPEAT\n");
			MediaPlayerSwitchPlayMode(PLAY_MODE_SUM);
#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_REPEAT;
            MessageSend(GetDisplayMessageHandle(), &msgSend);
#endif
			break;

		case MSG_FOLDER_PRE:
			if(CmdErrCnt >= 3)//临时处理U盘异常但是U盘插着的情况
			{
				MessageContext      msgSend;

				msgSend.msgId = MSG_MODE;
				MessageSend(MediaPlayCt->parentMsgHandle, &msgSend);
				osTaskDelay(5);
				//CmdErrCnt = 0;
			}
			MediaPlayerPreFolder();
			break;

		case MSG_FOLDER_NEXT:
			if(CmdErrCnt >= 3)//临时处理U盘异常但是U盘插着的情况
			{
				MessageContext      msgSend;

				msgSend.msgId = MSG_MODE;
				MessageSend(MediaPlayCt->parentMsgHandle, &msgSend);
				osTaskDelay(5);
				//CmdErrCnt = 0;
			}
			MediaPlayerNextFolder();
			break;
/*
		case MSG_PLAY_PAUSE:			
			APP_DBG("MSG_PLAY_PAUSE\n");
			MediaPlayerPlayPause();
#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_MEDIA;
            MessageSend(GetDisplayMessageHandle(), &msgSend);
#endif
			break;
	*/	
		case MSG_STOP:
			APP_DBG("MSG_STOP\n");
			MediaPlayerStop();
			break;

#ifdef DEL_REC_FILE_EN
		case MSG_REC_FILE_DEL:
			if(GetSystemMode() == AppModeCardPlayBack || GetSystemMode() == AppModeUDiskPlayBack || GetSystemMode() == AppModeFlashFsPlayBack)
			{
				if(GetMediaPlayerState() == PLAYER_STATE_PLAYING)
				{
					MediaPlayerStop();
					SoftFlagRegister(SoftFlagDecRecFile);
				}
				else
				{
					APP_DBG("not playback mode playing state\n");
				}
			}
			else
			{
				APP_DBG("not playback mode \n");
			}
			break;
#endif
		case MSG_DECODER_SERVICE_DISK_ERROR:
			APP_DBG("Disk ERROR\n");
			MediaPlayerStop();
			{
				MessageContext      msgSend;

				msgSend.msgId = MSG_MODE;
				MessageSend(MediaPlayCt->parentMsgHandle, &msgSend);
			}
			break;
		case MSG_DECODER_SERVICE_SONG_PLAY_END:
		//case MSG_DECODER_SERVICE_FF_END_SONG:
			if(GetDecoderState() == DecoderStatePause)//确保播放结束 ，依旧暂停态，减少stop消息交叉
			{
				APP_DBG("Last end, play next\n");
#ifdef CFG_FUNC_UDISK_DETECT
				if((GetSystemMode() == AppModeUDiskAudioPlay)
				&& (UDiskRemovedFlagGet() == TRUE))
				{
					break;
				}
#endif
				if(CmdErrCnt >= 3)//临时处理U盘异常但是U盘插着的情况
				{
					MessageContext      msgSend;

					msgSend.msgId = MSG_MODE;
					MessageSend(MediaPlayCt->parentMsgHandle, &msgSend);
					osTaskDelay(5);
				}
				if(gpMediaPlayer->CurPlayMode==PLAY_MODE_REPEAT_OFF)
				{
					gpMediaPlayer->CurFileIndex++;
					if(gpMediaPlayer->CurFileIndex > gpMediaPlayer->TotalFileSumInDisk)
					{
						gpMediaPlayer->CurFileIndex = 1;
						MediaPlayerStop();
					}
					else
					{
					gpMediaPlayer->CurFileIndex--;
					MediaPlayerDecoderStop();
					MediaPlayerNextSong(TRUE);
					}
				}
				else
				{
					MediaPlayerDecoderStop();
					MediaPlayerNextSong(TRUE);
				}
			}
			break;

		case MSG_NEXT:
#ifdef CFG_FUNC_REMIND_SOUND_EN
			//RemindSound request
			RemindSoundServiceItemRequest(SOUND_REMIND_XIAYISOU, FALSE);
#endif
			APP_DBG("play next\n");
			if(CmdErrCnt >= 3)//临时处理U盘异常但是U盘插着的情况
			{
				MessageContext      msgSend;

				msgSend.msgId = MSG_MODE;
				MessageSend(MediaPlayCt->parentMsgHandle, &msgSend);
				osTaskDelay(5);
			}
			MediaPlayerDecoderStop();
			MediaPlayerNextSong(FALSE);
			break;
		
		case MSG_PRE:
		//case MSG_DECODER_SERVICE_FB_END_SONG:
#ifdef CFG_FUNC_REMIND_SOUND_EN
			//RemindSound request
			RemindSoundServiceItemRequest(SOUND_REMIND_SHANGYIS, FALSE);
#endif
			APP_DBG("MSG_PRE\n");
			if(CmdErrCnt >= 3)//临时处理U盘异常但是U盘插着的情况
			{
				MessageContext      msgSend;

				msgSend.msgId = MSG_MODE;
				MessageSend(MediaPlayCt->parentMsgHandle, &msgSend);
				osTaskDelay(5);
			}
			MediaPlayerDecoderStop();
			MediaPlayerPreSong();
			break;
#ifdef FUNC_SPECIFY_FOLDER_PLAY_EN        
            case MSG_FOLDER_ERGE:
			case MSG_FOLDER_GUSHI:
			case MSG_FOLDER_GUOXUE:
			case MSG_FOLDER_YINGYU:
               APP_DBG("Story machine function\n");
				PlaySpecifyFolder(msgId-MSG_FOLDER_ERGE);
            break;
#endif            

		case MSG_FF_START:
			MediaPlayerFastForward();
			APP_DBG("MSG_FF_START\n");
			break;
		
		case MSG_FB_START:
			MediaPlayerFastBackward();
			APP_DBG("MSG_FB_START\n");
			break;

		case MSG_FF_FB_END:
			MediaPlayerFFFBEnd();
			APP_DBG("MSG_FF_FB_END\n");
			break;

#ifdef CFG_RES_IR_NUMBERKEY
		case MSG_NUM_0:
		case MSG_NUM_1:
		case MSG_NUM_2:
		case MSG_NUM_3:
		case MSG_NUM_4:
		case MSG_NUM_5:
		case MSG_NUM_6:
		case MSG_NUM_7:
		case MSG_NUM_8:
		case MSG_NUM_9:
			if(!Number_select_flag)
				Number_value = msgId - MSG_NUM_0;
			else
				Number_value = Number_value * 10 + (msgId - MSG_NUM_0);
			if(Number_value > 9999)
				Number_value = 0;
			Number_select_flag = 1;
			TimeOutSet(&Number_selectTimer, 2000);
            #ifdef CFG_FUNC_DISPLAY_EN
             msgSend.msgId = MSG_DISPLAY_SERVICE_NUMBER;
             MessageSend(GetDisplayMessageHandle(), &msgSend);
            #endif
			break;
#endif

		case MSG_DECODER_SERVICE_UPDATA_PLAY_TIME:
			MediaPlayerTimeUpdate();
			SendPreviewPlayNextMsg();
            #ifdef CFG_FUNC_DISPLAY_EN
			msgSend.msgId = MSG_DISPLAY_SERVICE_MEDIA;
			MessageSend(GetDisplayMessageHandle(), &msgSend);
            #endif
			break;

		case MSG_DECODER_PLAYING:
			AudioCoreSourceUnmute(APP_SOURCE_NUM, TRUE, TRUE);
			AudioCoreSourceUnmute(PLAYBACK_SOURCE_NUM, TRUE, TRUE);
			
			MediaPlaySetDecoderState(DecoderStatePlay);
			break;

		case MSG_REPEAT_AB:
			APP_DBG("MSG_REPEAT_AB\n");
			MediaPlayerRepeatAB();
			break;
			
		default:
			if(gpMediaPlayer->RepeatAB.RepeatFlag == REPEAT_OPENED
				&& (GetSystemMode() == AppModeCardAudioPlay || GetSystemMode() == AppModeUDiskAudioPlay))
			{
				MediaPlayerTimerCB();
			}
			CommonMsgProccess(msgId);
			break;
	}
#ifdef CFG_RES_IR_NUMBERKEY
	if(Number_select_flag)
	{
		if(IsTimeOut(&Number_selectTimer))
		{
			if((gpMediaPlayer->CurPlayMode ==PLAY_MODE_REPEAT_FOLDER) || (gpMediaPlayer->CurPlayMode == PLAY_MODE_RANDOM_FOLDER))
			{
				if((Number_value <= (gpMediaPlayer->PlayerFolder.FirstFileIndex + gpMediaPlayer->PlayerFolder.FileNumLen - 1)) && (Number_value))
				{
					MediaPlayerDecoderStop();
					gpMediaPlayer->CurFileIndex = Number_value;
					gpMediaPlayer->CurPlayTime = 0;
					gpMediaPlayer->SongSwitchFlag = 0;
					SetMediaPlayerState(PLAYER_STATE_PLAYING);
				}
			}
#ifdef FUNC_BROWSER_PARALLEL_EN
			else if(GetBrowserPlay_state()==Browser_Play_None)
			{
				APP_DBG("browser normal playing,not support ir number key select song \n");
			}
#endif
			else
			{
				if((Number_value <= gpMediaPlayer->TotalFileSumInDisk) && (Number_value))
				{
					MediaPlayerDecoderStop();
					gpMediaPlayer->CurFileIndex = Number_value;
					gpMediaPlayer->CurPlayTime = 0;
					gpMediaPlayer->SongSwitchFlag = 0;
					SetMediaPlayerState(PLAYER_STATE_PLAYING);
				}
			}
			Number_select_flag = 0;
		}
	}
#endif
}

/***************************************************************************************
 *
 * APIs
 *
 */
bool MediaPlayCreate(MessageHandle parentMsgHandle)
{
	bool		ret;

	ret = MediaPlayInit(parentMsgHandle);
	if(ret)
	{
		MediaPlayCt->taskHandle = NULL;
		xTaskCreate(MediaPlayEntrance, "MediaPlay", MEDIA_PLAY_TASK_STACK_SIZE, NULL, MEDIA_PLAY_TASK_PRIO, &MediaPlayCt->taskHandle);
		if(MediaPlayCt->taskHandle == NULL)
		{
			ret = FALSE;
		}
	}
	else
	{
		APP_DBG("MediaPlay app create fail!\n");
	}

	
	return ret;
}

bool MediaPlayStart(void)
{
	MessageContext		msgSend;
	if(MediaPlayCt == NULL)
	{
		return FALSE;
	}
	msgSend.msgId		= MSG_TASK_START;
	return MessageSend(MediaPlayCt->msgHandle, &msgSend);
}

bool MediaPlayPause(void)
{
	MessageContext		msgSend;
	if(MediaPlayCt == NULL)
	{
		return FALSE;
	}
	msgSend.msgId		= MSG_TASK_PAUSE;
	return MessageSend(MediaPlayCt->msgHandle, &msgSend);
}

bool MediaPlayResume(void)
{
	MessageContext		msgSend;
	if(MediaPlayCt == NULL)
	{
		return FALSE;
	}
	msgSend.msgId		= MSG_TASK_RESUME;
	return MessageSend(MediaPlayCt->msgHandle, &msgSend);
}

bool MediaPlayStop(void)
{
	MessageContext		msgSend;
	if(MediaPlayCt == NULL)
	{
		return FALSE;
	}
	AudioCoreSourceMute(APP_SOURCE_NUM, TRUE, TRUE);
#if (CFG_RES_MIC_SELECT) && defined(CFG_FUNC_AUDIO_EFFECT_EN)
	AudioCoreSourceMute(MIC_SOURCE_NUM, TRUE, TRUE);
#endif
	vTaskDelay(30);

	msgSend.msgId		= MSG_TASK_STOP;
	return MessageSend(MediaPlayCt->msgHandle, &msgSend);
}

void SendPreviewPlayNextMsg(void)
{
	MessageContext		msgSend;
	if(gpMediaPlayer->CurPlayMode!=PLAY_MODE_PREVIEW_PLAY)return;

	if(gpMediaPlayer->CurPlayTime>=PREVIEW_PLAY_TIME)
	{
		msgSend.msgId		= MSG_NEXT;
		MessageSend(MediaPlayCt->msgHandle, &msgSend);
	}
}

bool MediaPlayKill(void)
{
	if(MediaPlayCt == NULL)
	{
		return FALSE;
	}
	if(GetSystemMode() == AppModeUDiskAudioPlay || GetSystemMode() == AppModeUDiskPlayBack)
	{
		if(UDiskMutex != NULL)
		{
			//osMutexLock(UDiskMutex);
			while(osMutexLock_1000ms(UDiskMutex) != 1)
			{
				WDG_Feed();
			}
		}
	}
	else if((GetSystemMode() == AppModeCardAudioPlay || GetSystemMode() == AppModeCardPlayBack))
	{
		if(SDIOMutex)
		{
			osMutexLock(SDIOMutex);
		}
	}
	MediaPlayerCloseSongFile();

//注意此处，如果在TaskStateCreated,进入stop，它尚未init。
	AudioCoreProcessConfig((void*)AudioNoAppProcess);
	AudioCoreSourceDisable(MediaPlayCt->SourceNum);
	//AudioCoreSourceUnmute(MediaPlayCt->SourceNum, TRUE, TRUE);
	
#ifndef CFG_FUNC_MIXER_SRC_EN
#ifdef CFG_RES_AUDIO_DACX_EN
	AudioDAC_SampleRateChange(ALL, CFG_PARA_SAMPLE_RATE);//恢复
#endif
#ifdef CFG_RES_AUDIO_DAC0_EN
	AudioDAC_SampleRateChange(DAC0, CFG_PARA_SAMPLE_RATE);//恢复
#endif
#endif

	//Kill used services
	DecoderServiceKill();
#ifdef CFG_FUNC_RECORDER_EN
	if(MediaPlayCt->RecorderSync != TaskStateNone)//当录音创建失败时，需要强行回收
	{
		APP_DBG("media rec task del\n");
		SoftFlagDeregister(SoftFlagMediaRectate);
		MediaRecorderServiceKill();
		MediaPlayCt->RecorderSync = TaskStateNone;
	}
#endif

	if(GetSystemMode() == AppModeUDiskAudioPlay || GetSystemMode() == AppModeUDiskPlayBack)
	{
		f_unmount(MEDIA_VOLUME_STR_U);
		osMutexUnlock(UDiskMutex);
#ifdef CFG_FUNC_UDISK_DETECT
		UDiskRemovedFlagSet(1);
#endif
	}
	else if((GetSystemMode() == AppModeCardAudioPlay || GetSystemMode() == AppModeCardPlayBack))
	{
#ifdef CFG_RES_CARD_USE
		f_unmount(MEDIA_VOLUME_STR_C);
		SDCardDeinit(CFG_RES_CARD_GPIO);
		osMutexUnlock(SDIOMutex);
#endif
	}
	
	//task	先删任务，再删邮箱，收资源
	if(MediaPlayCt->taskHandle != NULL)
	{
		vTaskDelete(MediaPlayCt->taskHandle);
		MediaPlayCt->taskHandle = NULL;
	}

	//Msgbox
	if(MediaPlayCt->msgHandle != NULL)
	{
		MessageDeregister(MediaPlayCt->msgHandle);
		MediaPlayCt->msgHandle = NULL;
	}

//	DMA_ChannelClose(PERIPHERAL_ID_SDIO1_RX);
//	DMA_ChannelClose(PERIPHERAL_ID_SDIO1_TX);

	//PortFree
	ffpresearch_deinit();
	if(gpMediaPlayer->AccRamBlk!= NULL)
	{
		osPortFree(gpMediaPlayer->AccRamBlk);
		gpMediaPlayer->AccRamBlk = NULL;
	}
	MediaPlayerDeinitialize();
	if(MediaPlayCt->SourceDecoder != NULL)
	{
		osPortFree(MediaPlayCt->SourceDecoder);
		MediaPlayCt->SourceDecoder = NULL;
	}
	//osPortFree(MediaPlayCt.AudioCoreMediaPlay);
	MediaPlayCt->AudioCoreMediaPlay = NULL;

	osPortFree(MediaPlayCt);
	MediaPlayCt = NULL;

#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	AudioEffectsDeInit();
#endif
	return TRUE;
}

MessageHandle GetMediaPlayMessageHandle(void)
{
	if(MediaPlayCt == NULL)
	{
		return NULL;
	}
	return MediaPlayCt->msgHandle;
}

uint8_t MediaPlayDevice(void)
{
	if(MediaPlayCt == NULL)
	{
		return (uint8_t)(-1);
	}
	return MediaPlayCt->Device;
}
#endif

#ifdef CFG_FUNC_RECORDER_EN	
static void MediaRecEnter(void)
{
#ifdef	CFG_FUNC_RECORD_SD_UDISK
	if(ResourceValue(AppResourceCard) || ResourceValue(AppResourceUDisk))
	{
		if(MediaPlayCt->RecorderSync == TaskStateNone)
		{
			MediaRecorderServiceCreate(MediaPlayCt->msgHandle);
			MediaPlayCt->RecorderSync = TaskStateCreating;
		}
		
	}
	else
	{
		APP_DBG("error, no disk!!!\n");
	}
#elif defined(CFG_FUNC_RECORD_FLASHFS)
	if(MediaPlayCt->RecorderSync == TaskStateNone)
	{
		MediaRecorderServiceCreate(MediaPlayCt->msgHandle);
		MediaPlayCt->RecorderSync = TaskStateCreating;
	} 
	
#endif
}
#endif		








