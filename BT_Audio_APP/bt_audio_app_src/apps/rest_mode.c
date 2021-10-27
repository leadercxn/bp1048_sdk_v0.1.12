/**
 **************************************************************************************
 * @file    rest_mode.c
 * @brief   
 *
 * @author  Pi
 * @version V1.0.0
 *
 * $Created: 2018-09-26 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include <string.h>
#include "type.h"
#include "irqn.h"
#include "gpio.h"
#include "dma.h"
#include "rtos_api.h"
#include "app_message.h"
#include "app_config.h"
#include "debug.h"
#include "audio_core_api.h"
#include "main_task.h"
#include "recorder_service.h"
#include "audio_core_service.h"
#include "decoder_service.h"
#include "remind_sound_service.h"
#include "audio_effect.h"
#include "breakpoint.h"
#include "otg_device_hcd.h"
#include "otg_device_standard_request.h"
#include "backup_interface.h"
#include "timeout.h"
#include "ctrlvars.h"
#include "audio_vol.h"
#include "rest_mode.h"

#ifdef CFG_APP_REST_MODE_EN

#define REST_PLAY_TASK_STACK_SIZE		512//1024
#define REST_PLAY_TASK_PRIO				3
#define REST_NUM_MESSAGE_QUEUE			5


typedef struct _RestPlayContext
{
	xTaskHandle 		taskHandle;
	MessageHandle		msgHandle;
	MessageHandle		parentMsgHandle;

	TaskState			state;
	AudioCoreContext 	*AudioCoreRest;
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	uint16_t*			Source1Decoder;
	TaskState			DecoderSync;
	bool				ExitRemind;
	TIMER				ExitTimer;
#endif

	//play
	uint32_t 			SampleRate; //带提示音时，如果不重采样，要避免采样率配置冲突
}RestPlayContext;


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

static  RestPlayContext*		RestPlayCt = NULL;


static void RestPlayModeCreating(uint16_t msgId);
static void RestPlayModeStarting(uint16_t msgId);
static void RestPlayModeStopping(uint16_t msgId);
static void RestPlayModeStopped(void);
static void RestPlayRunning(uint16_t msgId);
uint8_t RestDecoderSourceNum(void);

static bool RestPlay_Init(MessageHandle parentMsgHandle)
{
//System config
	DMA_ChannelAllocTableSet((uint8_t *)DmaChannelMap);
//Task & App Config
	RestPlayCt = (RestPlayContext*)osPortMalloc(sizeof(RestPlayContext));
	if(RestPlayCt == NULL)
	{
		return FALSE;
	}
	memset(RestPlayCt, 0, sizeof(RestPlayContext));
	RestPlayCt->msgHandle = MessageRegister(REST_NUM_MESSAGE_QUEUE);
	if(RestPlayCt->msgHandle == NULL)
	{
		return FALSE;
	}
	RestPlayCt->parentMsgHandle = parentMsgHandle;
	RestPlayCt->state = TaskStateCreating;
	RestPlayCt->SampleRate = CFG_PARA_SAMPLE_RATE;

	//Core Source1 para
	RestPlayCt->AudioCoreRest = (AudioCoreContext*)&AudioCore;
	//Audio init
//	//note Soure0.和sink0已经在main app中配置，不要随意配置
	//InCore1 buf
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	RestPlayCt->Source1Decoder = (uint16_t*)osPortMalloc(mainAppCt.SamplesPreFrame * 2 * 2);//One Frame
	if(RestPlayCt->Source1Decoder == NULL)
	{
		return FALSE;
	}
	memset(RestPlayCt->Source1Decoder, 0, mainAppCt.SamplesPreFrame * 2 * 2);//2K

	//Core Soure2 Para
	DecoderSourceNumSet(REMIND_SOURCE_NUM);
	RestPlayCt->AudioCoreRest->AudioSource[REMIND_SOURCE_NUM].Enable = 0;
	RestPlayCt->AudioCoreRest->AudioSource[REMIND_SOURCE_NUM].FuncDataGet = DecodedPcmDataGet;
	RestPlayCt->AudioCoreRest->AudioSource[REMIND_SOURCE_NUM].FuncDataGetLen = NULL;
	RestPlayCt->AudioCoreRest->AudioSource[REMIND_SOURCE_NUM].IsSreamData = FALSE;//Decoder
	RestPlayCt->AudioCoreRest->AudioSource[REMIND_SOURCE_NUM].PcmFormat = 2; //stereo
	RestPlayCt->AudioCoreRest->AudioSource[REMIND_SOURCE_NUM].PcmInBuf = (int16_t *)RestPlayCt->Source1Decoder;
	RestPlayCt->ExitRemind = FALSE;
#endif

	//Core Process
#ifdef CFG_FUNC_AUDIO_EFFECT_EN
    #ifdef CFG_FUNC_MIC_KARAOKE_EN
	RestPlayCt->AudioCoreRest->AudioEffectProcess = (AudioCoreProcessFunc)AudioEffectProcess;
	#else
	RestPlayCt->AudioCoreRest->AudioEffectProcess = (AudioCoreProcessFunc)AudioMusicProcess;
	#endif
#else
	RestPlayCt->AudioCoreRest->AudioEffectProcess = (AudioCoreProcessFunc)AudioBypassProcess;
#endif
//	RestPlayCt->AudioCoreRest->AudioSource[MIC_SOURCE_NUM].Enable = 0;
	return TRUE;
}

//创建从属services
static void RestPlayModeCreate(void)
{
	bool NoService = TRUE;
	
	// Create service task
#if defined(CFG_FUNC_REMIND_SBC)
	DecoderServiceCreate(RestPlayCt->msgHandle, DECODER_BUF_SIZE_SBC, DECODER_FIFO_SIZE_FOR_SBC);//提示音格式决定解码器内存消耗
	NoService = FALSE;
#elif defined(CFG_FUNC_REMIND_SOUND_EN)
	DecoderServiceCreate(RestPlayCt->msgHandle, DECODER_BUF_SIZE_MP3, DECODER_FIFO_SIZE_FOR_MP3);
	NoService = FALSE;
#endif
	if(NoService)
	{
		RestPlayModeCreating(MSG_NONE);
	}
}

//All of services is created
//Send CREATED message to parent
static void RestPlayModeCreating(uint16_t msgId)
{
	MessageContext		msgSend;
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(msgId == MSG_DECODER_SERVICE_CREATED)
	{
		APP_DBG("Rest:Decoder created\n");
		RestPlayCt->DecoderSync = TaskStateReady;
	}
	if(RestPlayCt->DecoderSync == TaskStateReady)
#endif
	{
		msgSend.msgId		= MSG_REST_PLAY_MODE_CREATED;
		MessageSend(RestPlayCt->parentMsgHandle, &msgSend);
		RestPlayCt->state = TaskStateReady;
	}
}

static void RestPlayModeStart(void)
{
	bool NoService = TRUE;
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	DecoderServiceStart();
	RestPlayCt->DecoderSync = TaskStateStarting;
	NoService = FALSE;
#endif
	if(NoService)
	{
		RestPlayModeStarting(MSG_NONE);
	}
	else
	{
		RestPlayCt->state = TaskStateStarting;
	}
}

static void RestPlayModeStarting(uint16_t msgId)
{
	MessageContext		msgSend;
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(msgId == MSG_DECODER_SERVICE_STARTED)
	{
		APP_DBG("Rest:Decoder startted\n");
		RestPlayCt->DecoderSync = TaskStateRunning;
	}
	if(RestPlayCt->DecoderSync == TaskStateRunning)
#endif
	{
		msgSend.msgId		= MSG_REST_PLAY_MODE_STARTED;
		MessageSend(RestPlayCt->parentMsgHandle, &msgSend);
		RestPlayCt->state = TaskStateRunning;
#ifdef CFG_FUNC_ALARM_EN
		if(!mainAppCt.AlarmRemindStart)
#endif
		{
#ifdef CFG_FUNC_REMIND_SOUND_EN
			RemindSoundServiceItemRequest(SOUND_REMIND_GUANJI, FALSE);//SOUND_REMIND_STANDBYM
#else
			RestModeGotoDeepSleep();
#endif
		}
	}
}

static void RestPlayModeStop(void)
{
	bool NoService = TRUE;
#if	defined(CFG_FUNC_REMIND_SOUND_EN) 
	if(RestPlayCt->DecoderSync != TaskStateNone && RestPlayCt->DecoderSync != TaskStateStopped)
	{//解码器是 随app kill
		DecoderServiceStop();
		RestPlayCt->DecoderSync = TaskStateStopping;
		NoService = FALSE;
	}
#endif
	RestPlayCt->state = TaskStateStopping;
	if(NoService)
	{
		RestPlayModeStopped();
	}
}

static void RestPlayModeStopping(uint16_t msgId)//部分子service 随用随kill
{
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(msgId == MSG_DECODER_SERVICE_STOPPED)
	{
		RestPlayCt->DecoderSync = TaskStateNone;
	}
#endif
	if((RestPlayCt->state == TaskStateStopping)
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
		&& (RestPlayCt->DecoderSync == TaskStateNone)
#endif
	)
	{
		RestPlayModeStopped();
	}
}

static void RestPlayModeStopped(void)
{
	MessageContext		msgSend;
	//Set para
	
	//clear msg
	MessageClear(RestPlayCt->msgHandle);
	
	//Set state
	RestPlayCt->state = TaskStateStopped;

	//reply
	msgSend.msgId		= MSG_REST_PLAY_MODE_STOPPED;
	MessageSend(RestPlayCt->parentMsgHandle, &msgSend);
}

void RestModeGotoDeepSleep(void)
{//GOTO deepsleep
	MessageContext	msgSend;
	msgSend.msgId = MSG_DEEPSLEEP;
	MessageSend(GetMainMessageHandle(), &msgSend);
}

void RestModeGotoPowerDown(void)
{//GOTO Power down
	extern void SystemGotoPowerDown(void);
	SystemGotoPowerDown();
}

static void RestPlayEntrance(void * param)
{
	MessageContext		msgRecv;
	
	APP_DBG("Rest:App\n");
	// Create services
	RestPlayModeCreate();
#if (defined(CFG_FUNC_AUDIO_EFFECT_EN) && (defined(CFG_COMMUNICATION_BY_USB ))) || defined(CFG_APP_USB_AUDIO_MODE_EN)
	//Rest 模式下关USB中断，不进行调音
	NVIC_DisableIRQ(Usb_IRQn);
	OTG_DeviceDisConnect();
#endif
#ifdef CFG_FUNC_BREAKPOINT_EN
	BackupInfoUpdata(BACKUP_SYS_INFO);
#endif
	while(1)
	{
		MessageRecv(RestPlayCt->msgHandle, &msgRecv, 50);//没消息时50ms进一次。
		if(RestPlayCt->state != TaskStateRunning)
		{
			switch(msgRecv.msgId)//警告：在此段代码，禁止新增提示音插播位置。
			{	
				case MSG_DECODER_SERVICE_CREATED:
					RestPlayModeCreating(msgRecv.msgId);
					break;
		
				case MSG_TASK_START:
					RestPlayModeStart();
					break;
				case MSG_DECODER_SERVICE_STARTED:
					//RemindSound request		
					RestPlayModeStarting(msgRecv.msgId);
					break;

				case MSG_TASK_STOP: //Created 时stop
#ifdef CFG_COMMUNICATION_BY_USB
					NVIC_DisableIRQ(Usb_IRQn);
					OTG_DeviceDisConnect();
#endif
					RestPlayModeStop();
					break;

				case MSG_DECODER_SERVICE_STOPPED:
					RestPlayModeStopping(msgRecv.msgId);
					break;
			}
		}
		else //RestPlayCt->state == TaskStateRunning
		{
			RestPlayRunning(msgRecv.msgId);
		}
	}
}

static void RestPlayRunning(uint16_t msgId)
{
	MessageContext		msgSend;
	switch(msgId)
	{
		case MSG_TASK_STOP:

#ifdef CFG_COMMUNICATION_BY_USB
			NVIC_DisableIRQ(Usb_IRQn);
			OTG_DeviceDisConnect();
#endif
//#ifdef CFG_FUNC_REMIND_SOUND_EN
//			if(!SoftFlagGet(SoftFlagDeepSleepRequest) 	//注意，如果系统准备deepsleep，不要播放开机提示音。
//					&& RemindSoundServiceItemRequest(SOUND_REMIND_KAIJI, FALSE))
//			{
//				RestPlayCt->ExitRemind = TRUE;
//				TimeOutSet(&RestPlayCt->ExitTimer, 300);
//			}
//			else
//#endif
			{
				RestPlayModeStop();
#ifdef CFG_FUNC_REMIND_SOUND_EN
				RemindSoundServiceReset();
#endif
			}
			break;

#ifdef	CFG_FUNC_POWERKEY_EN
		case MSG_TASK_POWERDOWN:
			APP_DBG("Rest:MSG receive PowerDown, Please breakpoint\n");
			SystemPowerDown();
			break;
#endif

#if defined(CFG_FUNC_REMIND_SOUND_EN)
		case MSG_DECODER_STOPPED:
			msgSend.msgId = msgId;
			MessageSend(GetRemindSoundServiceMessageHandle(), &msgSend);//提示音期间转发解码器消息。

			break;			

		case MSG_REMIND_SOUND_PLAY_REQUEST_FAIL:
		case MSG_DECODER_RESET://解码器出让和回收，临界消息,提示音结束标记。
			RestModeGotoDeepSleep();
			break;

#endif
		default:
			CommonMsgProccess(msgId);
			break;
	}
#ifdef CFG_FUNC_ALARM_EN
	if(mainAppCt.AlarmRemindStart)
	{
		if(RemindSoundServiceRequestStatus())
		{
			#ifdef CFG_FUNC_REMIND_SOUND_EN
			RemindSoundServiceItemRequest(SOUND_REMIND_CALLRING, FALSE);//SOUND_REMIND_STANDBYM
			#endif
			if(mainAppCt.AlarmRemindCnt)
			{
				mainAppCt.AlarmRemindCnt--;				
			}
			else
			{
				mainAppCt.AlarmRemindStart = 0;
				RestPlayModeStop();
				RemindSoundServiceReset();
			}
		}		
	}
#endif
#if defined(CFG_FUNC_REMIND_SOUND_EN)
	if(RestPlayCt->ExitRemind &&  IsTimeOut(&RestPlayCt->ExitTimer) && !SoftFlagGet(SoftFlagDecoderRemind))
	{
		RestPlayModeStop();
		RemindSoundServiceReset();
	}
#endif
}

/***************************************************************************************
 *
 * APIs
 *
 */
bool RestPlayCreate(MessageHandle parentMsgHandle)
{
	bool		ret = TRUE;

	ret = RestPlay_Init(parentMsgHandle);
	if(ret)
	{
		RestPlayCt->taskHandle = NULL;
		xTaskCreate(RestPlayEntrance,
					"RestPlay",
					REST_PLAY_TASK_STACK_SIZE,
					NULL, REST_PLAY_TASK_PRIO,
					&RestPlayCt->taskHandle);
		if(RestPlayCt->taskHandle == NULL)
		{
			ret = FALSE;
		}
	}
	else
	{
		APP_DBG("Rest:app create fail!\n");
	}
	return ret;
}

bool RestPlayStart(void)
{
	MessageContext		msgSend;

	if(RestPlayCt == NULL)
	{
		return FALSE;
	}
	msgSend.msgId		= MSG_TASK_START;
	return MessageSend(RestPlayCt->msgHandle, &msgSend);
}

bool RestPlayPause(void)
{
	MessageContext		msgSend;
	if(RestPlayCt == NULL)
	{
		return FALSE;
	}
	msgSend.msgId		= MSG_TASK_PAUSE;
	return MessageSend(RestPlayCt->msgHandle, &msgSend);
}

bool RestPlayResume(void)
{
	MessageContext		msgSend;
	if(RestPlayCt == NULL)
	{
		return FALSE;
	}
	msgSend.msgId		= MSG_TASK_RESUME;
	return MessageSend(RestPlayCt->msgHandle, &msgSend);
}

bool RestPlayStop(void)
{
	MessageContext		msgSend;
	if(RestPlayCt == NULL)
	{
		return FALSE;
	}
	msgSend.msgId		= MSG_TASK_STOP;
	return MessageSend(RestPlayCt->msgHandle, &msgSend);
}

bool RestPlayKill(void)
{
	if(RestPlayCt == NULL)
	{
		return FALSE;
	}

	//Kill used services
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	AudioCoreSourceDisable(REMIND_SOURCE_NUM);
	DecoderServiceKill();
#endif
#if (CFG_RES_MIC_SELECT) //&& defined(CFG_FUNC_AUDIO_EFFECT_EN)
	RestPlayCt->AudioCoreRest->AudioSource[MIC_SOURCE_NUM].Enable = TRUE;
#endif
	//注意：AudioCore父任务调整到mainApp下，此处只关闭AudioCore通道，不关闭任务
	AudioCoreProcessConfig((void*)AudioNoAppProcess);

	//task
	if(RestPlayCt->taskHandle != NULL)
	{
		vTaskDelete(RestPlayCt->taskHandle);
		RestPlayCt->taskHandle = NULL;
	}

	//Msgbox
	if(RestPlayCt->msgHandle != NULL)
	{
		MessageDeregister(RestPlayCt->msgHandle);
		RestPlayCt->msgHandle = NULL;
	}

	//PortFree
	RestPlayCt->AudioCoreRest = NULL;

#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(RestPlayCt->Source1Decoder != NULL)
	{
		osPortFree(RestPlayCt->Source1Decoder);
		RestPlayCt->Source1Decoder = NULL;
	}
#endif

	osPortFree(RestPlayCt);
	RestPlayCt = NULL;
	APP_DBG("Rest:Kill Ct\n");

	return TRUE;
}

MessageHandle GetRestPlayMessageHandle(void)
{
	if(RestPlayCt != NULL)
	{
		return RestPlayCt->msgHandle;
	}
	else
	{
		return NULL;
	}
}

#endif
