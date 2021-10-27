/**
 **************************************************************************************
 * @file    ai_mode.c
 * @brief   
 *
 * @author  Pi
 * @version V1.0.0
 *
 * $Created: 2019-8-15 13:06:47$
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
#include "delay.h"
#include "audio_adc.h"
#include "dac.h"
#include "adc_interface.h"
#include "dac_interface.h"
#include "audio_core_api.h"
#include "audio_core_service.h"
#include "decoder_service.h"
#include "remind_sound_service.h"
#include "main_task.h"
#include "audio_effect.h"
#include "powercontroller.h"
#include "deepsleep.h"
#include "backup_interface.h"
#include "breakpoint.h"
#include "otg_device_hcd.h"
#include "otg_device_standard_request.h"
#include "audio_vol.h"
#include "ctrlvars.h"
#include "mode_switch_api.h"
#include "reset.h"

#ifdef CFG_FUNC_AI_EN
	#include "ai.h"
#endif

#ifdef	CFG_XIAOAI_AI_EN
	#include "aivs_rcsp.h"
	#include "xm_xiaoai_api.h"
#endif


void AudioEffectModeSel(uint8_t mode, uint8_t init_flag);//0=hw,1=effect,2=hw+effect ff= no init
bool AiPlayCreate();
#ifdef CFG_FUNC_AI_EN

#define AI_PLAY_TASK_STACK_SIZE		256//1024
#define AI_PLAY_TASK_PRIO			3
#define AI_NUM_MESSAGE_QUEUE		10

typedef struct _AiPlayContext
{
	xTaskHandle 		taskHandle;
	MessageHandle		msgHandle;
	MessageHandle		parentMsgHandle;

	TaskState			state;

	AudioCoreContext 	*AudioCoreAi;
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	uint16_t*			SourceDecoder;
	TaskState			DecoderSync;
#endif

	//play
	uint32_t 			SampleRate; //带提示音时，如果不重采样，要避免采样率配置冲突

}AiPlayContext;

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
	255,//PERIPHERAL_ID_SDIO_RX,			//3
	255,//PERIPHERAL_ID_SDIO_TX,			//4
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
	255,//PERIPHERAL_ID_AUDIO_ADC0_RX,	//17
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

static  AiPlayContext*		AiPlayCt;


static void AiPlayModeCreating(uint16_t msgId);
static void AiPlayModeStarting(uint16_t msgId);
static void AiPlayModeStopping(uint16_t msgId);
static void AiPlayModeStopped(void);
static void AiPlayRunning(uint16_t msgId);


void AiPlayResRelease(void)
{
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(AiPlayCt->SourceDecoder != NULL)
	{
		APP_DBG("SourceDecoder\n");
		osPortFree(AiPlayCt->SourceDecoder);
		AiPlayCt->SourceDecoder = NULL;
	}
#endif
}

bool AiPlayResMalloc(uint16_t SampleLen)
{
	//InCore buf
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	//AiPlayCt->SourceDecoder = (uint16_t*)osPortMalloc(CFG_PARA_SAMPLES_PER_FRAME * 2 * 2);//One Frame
	AiPlayCt->SourceDecoder = (uint16_t*)osPortMallocFromEnd(SampleLen * 2 * 2);//One Frame
	if(AiPlayCt->SourceDecoder == NULL)
	{
		return FALSE;
	}
	memset(AiPlayCt->SourceDecoder, 0, SampleLen * 2 * 2);//2K
#endif

	return TRUE;
}

void AiPlayResInit(void)
{
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(AiPlayCt->SourceDecoder != NULL)
	{
		AiPlayCt->AudioCoreAi->AudioSource[REMIND_SOURCE_NUM].PcmInBuf = (int16_t *)AiPlayCt->SourceDecoder;
	}
#endif

}

/**
 * @func        AiPlay_Init
 * @brief       Ai模式参数配置，资源初始化
 * @param       MessageHandle parentMsgHandle
 * @Output      None
 * @return      bool
 * @Others      任务块、Adc、Dac、AudioCore配置
 * @Others      数据流从Adc到audiocore配有函数指针，audioCore到Dac同理，由audiocoreService任务按需驱动
 * Record
 */
static bool AiPlay_Init()//(MessageHandle parentMsgHandle)
{
//System config
//	DMA_ChannelAllocTableSet((uint8_t *)DmaChannelMap);//Ai
//Task & App Config
	AiPlayCt = (AiPlayContext*)osPortMalloc(sizeof(AiPlayContext));
	if(AiPlayCt == NULL)
	{
		return FALSE;
	}
	memset(AiPlayCt, 0, sizeof(AiPlayContext));
	AiPlayCt->msgHandle = MessageRegister(AI_NUM_MESSAGE_QUEUE);
	if(AiPlayCt->msgHandle == NULL)
	{
		return FALSE;
	}
//	AiPlayCt->parentMsgHandle = parentMsgHandle;
	AiPlayCt->state = TaskStateCreating;
	AiPlayCt->SampleRate = CFG_PARA_SAMPLE_RATE;

	if(!AiPlayResMalloc(mainAppCt.SamplesPreFrame))
	{
		APP_DBG("AiPlay Res Error!\n");
		return FALSE;
	}
	

	//Core Source1 para
	AiPlayCt->AudioCoreAi = (AudioCoreContext*)&AudioCore;
	//Audio init
//	//note Soure0.和sink0已经在main app中配置，不要随意配置

	//Core Soure Para
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
//	DecoderSourceNumSet(REMIND_SOURCE_NUM);
//	AiPlayCt->AudioCoreAi->AudioSource[REMIND_SOURCE_NUM].Enable = 0;
//	AiPlayCt->AudioCoreAi->AudioSource[REMIND_SOURCE_NUM].FuncDataGet = DecodedPcmDataGet;
//	AiPlayCt->AudioCoreAi->AudioSource[REMIND_SOURCE_NUM].FuncDataGetLen = NULL;
//	AiPlayCt->AudioCoreAi->AudioSource[REMIND_SOURCE_NUM].IsSreamData = FALSE;//Decoder
//	AiPlayCt->AudioCoreAi->AudioSource[REMIND_SOURCE_NUM].PcmFormat = 2; //stereo
//	AiPlayCt->AudioCoreAi->AudioSource[REMIND_SOURCE_NUM].PcmInBuf = (int16_t *)AiPlayCt->SourceDecoder;
#endif

	//Core Process
	
#ifdef CFG_FUNC_AUDIO_EFFECT_EN
    #ifdef CFG_FUNC_MIC_KARAOKE_EN
//	AiPlayCt->AudioCoreAi->AudioEffectProcess = (AudioCoreProcessFunc)AudioEffectProcess;
	#else
	AiPlayCt->AudioCoreAi->AudioEffectProcess = (AudioCoreProcessFunc)AudioMusicProcess;
	#endif
#else
	AiPlayCt->AudioCoreAi->AudioEffectProcess = (AudioCoreProcessFunc)AudioBypassProcess;
#endif

	if(ai_init() == FALSE)
	{
		return FALSE;
	}
	SoftFlagRegister(SoftFlagAiProcess);
	return TRUE;
}

//创建从属services
static void AiPlayModeCreate(void)
{
	bool NoService = TRUE;
	
	// Create service task
#if defined(CFG_FUNC_REMIND_SBC)
	DecoderServiceCreate(AiPlayCt->msgHandle, DECODER_BUF_SIZE_SBC, DECODER_FIFO_SIZE_FOR_SBC);//提示音格式决定解码器内存消耗
	NoService = FALSE;
#elif defined(CFG_FUNC_REMIND_SOUND_EN)
//	DecoderServiceCreate(AiPlayCt->msgHandle, DECODER_BUF_SIZE_MP3, DECODER_FIFO_SIZE_FOR_MP3);
//	NoService = FALSE;
#endif
	if(NoService)
	{
		AiPlayModeCreating(MSG_NONE);
	}
}

//All of services is created
//Send CREATED message to parent
static void AiPlayModeCreating(uint16_t msgId)
{
	MessageContext		msgSend;
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(msgId == MSG_DECODER_SERVICE_CREATED)
	{
		AiPlayCt->DecoderSync = TaskStateReady;
	}
	if(AiPlayCt->DecoderSync == TaskStateReady)
#endif
	{
		msgSend.msgId		= MSG_LINE_AUDIO_MODE_CREATED;
		MessageSend(AiPlayCt->parentMsgHandle, &msgSend);
		AiPlayCt->state = TaskStateReady;
	}
}

static void AiPlayModeStart(void)
{
	bool NoService = TRUE;
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	DecoderServiceStart();
	AiPlayCt->DecoderSync = TaskStateStarting;
	NoService = FALSE;
#endif
	if(NoService)
	{
		AiPlayModeStarting(MSG_NONE);
	}
	else
	{
		AiPlayCt->state = TaskStateStarting;
	}
}

static void AiPlayModeStarting(uint16_t msgId)
{
	MessageContext		msgSend;
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(msgId == MSG_DECODER_SERVICE_STARTED)
	{
		AiPlayCt->DecoderSync = TaskStateRunning;
	}
	if(AiPlayCt->DecoderSync == TaskStateRunning)
#endif
	{
		msgSend.msgId		= MSG_LINE_AUDIO_MODE_STARTED;
		MessageSend(AiPlayCt->parentMsgHandle, &msgSend);

		AiPlayCt->state = TaskStateRunning;
	}
}

static void AiPlayModeStop(void)
{
	bool NoService = TRUE;
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(AiPlayCt->DecoderSync != TaskStateNone && AiPlayCt->DecoderSync != TaskStateStopped)
	{//解码器是 随app kill
//		DecoderServiceStop();
//		AiPlayCt->DecoderSync = TaskStateStopping;
//		NoService = FALSE;
	}
#endif
	SoftFlagDeregister(SoftFlagAiProcess);//先停止，避免kill 释放引起错误
	AiPlayCt->state = TaskStateStopping;
	if(NoService)
	{
		AiPlayModeStopped();
	}
}

static void AiPlayModeStopping(uint16_t msgId)//部分子service 随用随kill
{
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(msgId == MSG_DECODER_SERVICE_STOPPED)
	{
		AiPlayCt->DecoderSync = TaskStateNone;
	}
#endif
	if((AiPlayCt->state == TaskStateStopping)
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
		&& (AiPlayCt->DecoderSync == TaskStateNone)
#endif
	)
	{
		AiPlayModeStopped();
	}
}

static void AiPlayModeStopped(void)
{
	MessageContext		msgSend;
	//Set para
	
	//clear msg
	MessageClear(AiPlayCt->msgHandle);
	
	//Set state
	AiPlayCt->state = TaskStateStopped;

	//reply
//	msgSend.msgId		= MSG_AI_MODE_STOPPED;
	MessageSend(AiPlayCt->parentMsgHandle, &msgSend);
}

/**
 * @func        AiPlayEntrance
 * @brief       模式执行主体
 * @param       void * param  
 * @Output      None
 * @return      None
 * @Others      模式建立和结束过程
 * Record
 */
static void AiPlayEntrance(void * param)
{
	MessageContext		msgRecv;
	
	APP_DBG("App\n");
	// Create services
	AiPlayModeCreate();

#ifdef CFG_FUNC_AUDIO_EFFECT_EN	
	AudioEffectModeSel(EFFECT_MODE_YuanSheng, 2);//0=init hw,1=effect,2=hw+effect
#ifdef CFG_COMMUNICATION_BY_UART
	UART1_Communication_Init((void *)(&UartRxBuf[0]), 1024, (void *)(&UartTxBuf[0]), 1024);
#endif
#endif

#if (CFG_RES_MIC_SELECT) && defined(CFG_FUNC_AUDIO_EFFECT_EN)
	AudioCoreSourceUnmute(MIC_SOURCE_NUM, TRUE, TRUE);
#endif
	while(1)
	{
		MessageRecv(AiPlayCt->msgHandle, &msgRecv, MAX_RECV_MSG_TIMEOUT);
		
		switch(msgRecv.msgId)//警告：在此段代码，禁止新增提示音插播位置。
		{	
			case MSG_DECODER_SERVICE_CREATED:
				AiPlayModeCreating(msgRecv.msgId);
				break;

			case MSG_TASK_START:
				AiPlayModeStart();
				break;

			case MSG_DECODER_SERVICE_STARTED:
				//RemindSound request		
				AiPlayModeStarting(msgRecv.msgId);
				break;

			case MSG_TASK_RESUME:
				if(AiPlayCt->state == TaskStatePaused)
				{
					AiPlayCt->state = TaskStateRunning;
				}
				break;

			case MSG_TASK_STOP:
#ifdef CFG_FUNC_REMIND_SOUND_EN
				RemindSoundServiceReset();
#endif
#if 0//CFG_COMMUNICATION_BY_USB
				NVIC_DisableIRQ(Usb_IRQn);
				OTG_DeviceDisConnect();
#endif
				AiPlayModeStop();
				break;

			case MSG_DECODER_SERVICE_STOPPED:
				AiPlayModeStopping(msgRecv.msgId);
				break;

			case MSG_APP_RES_RELEASE:
				AiPlayResRelease();
				{
					MessageContext		msgSend;
					msgSend.msgId		= MSG_APP_RES_RELEASE_SUC;
					MessageSend(GetMainMessageHandle(), &msgSend);
				}
				break;
			case MSG_APP_RES_MALLOC:
				AiPlayResMalloc(mainAppCt.SamplesPreFrame);
				{
					MessageContext		msgSend;
					msgSend.msgId		= MSG_APP_RES_MALLOC_SUC;
					MessageSend(GetMainMessageHandle(), &msgSend);
				}
				break;
			case MSG_APP_RES_INIT:
				AiPlayResInit();
				{
					MessageContext		msgSend;
					msgSend.msgId		= MSG_APP_RES_INIT_SUC;
					MessageSend(GetMainMessageHandle(), &msgSend);
				}
				break;
				
			case MSG_REMIND_SOUND_PLAY_START:
				break;
			case MSG_REMIND_SOUND_PLAY_DONE://提示音播放结束
			case MSG_REMIND_SOUND_PLAY_REQUEST_FAIL:
				break;
			default:
				AiPlayRunning(msgRecv.msgId);
				break;
		}
	}
}

static void AiPlayRunning(uint16_t msgId)
{
	if(AiPlayCt->state == TaskStateRunning)
	{
		switch(msgId)
		{
#ifdef	CFG_FUNC_POWERKEY_EN
			case MSG_TASK_POWERDOWN:
				APP_DBG("MSG receive PowerDown, Please breakpoint\n");
				SystemPowerDown();
				break;
#endif

			case MSG_DECODER_STOPPED:
#if defined(CFG_FUNC_REMIND_SOUND_EN)
				if(SoftFlagGet(SoftFlagDecoderRemind))
				{
					MessageContext		msgSend;
					msgSend.msgId = msgId;
					MessageSend(GetRemindSoundServiceMessageHandle(), &msgSend);//提示音期间转发解码器消息。
				}
#endif
				break;			

			default:
				CommonMsgProccess(msgId);
				break;
		}
	}
}

/***************************************************************************************
 *
 * APIs
 *
 */


bool AiPlayStart(void)
{
	MessageContext		msgSend;

	if(AiPlayCt == NULL)
	{
		return FALSE;
	}
	msgSend.msgId		= MSG_TASK_START;
	return MessageSend(AiPlayCt->msgHandle, &msgSend);
}

bool AiPlayPause(void)
{
	MessageContext		msgSend;
	if(AiPlayCt == NULL)
	{
		return FALSE;
	}
	msgSend.msgId		= MSG_TASK_PAUSE;
	return MessageSend(AiPlayCt->msgHandle, &msgSend);
}

bool AiPlayResume(void)
{
	MessageContext		msgSend;
	if(AiPlayCt == NULL)
	{
		return FALSE;
	}
	msgSend.msgId		= MSG_TASK_RESUME;
	return MessageSend(AiPlayCt->msgHandle, &msgSend);
}

bool AiPlayStop(void)
{
	MessageContext		msgSend;
	if(AiPlayCt == NULL)
	{
		return FALSE;
	}
	AudioCoreSourceMute(APP_SOURCE_NUM, TRUE, TRUE);
#if (CFG_RES_MIC_SELECT) && defined(CFG_FUNC_AUDIO_EFFECT_EN)
	AudioCoreSourceMute(MIC_SOURCE_NUM, TRUE, TRUE);
#endif				
	vTaskDelay(30);

	msgSend.msgId		= MSG_TASK_STOP;
	return MessageSend(AiPlayCt->msgHandle, &msgSend);
}

bool AiPlayKill(void)
{
	if(AiPlayCt == NULL)
	{
		return FALSE;
	}
	SoftFlagDeregister(SoftFlagAiProcess);
	//AiPlayCt->AudioCoreAi->AudioSource[AI_SOURCE_NUM].Enable = 0;
//	DecoderServiceStart();
	//Kill used services
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
//	AudioCoreSourceDisable(REMIND_SOURCE_NUM);
//	DecoderServiceKill();
#endif

	//注意：AudioCore父任务调整到mainApp下，此处只关闭AudioCore通道，不关闭任务
//	AudioCoreProcessConfig(NULL);

	ai_deinit();

	//task
	if(AiPlayCt->taskHandle != NULL)
	{
		vTaskDelete(AiPlayCt->taskHandle);
		AiPlayCt->taskHandle = NULL;
	}

	//Msgbox
	if(AiPlayCt->msgHandle != NULL)
	{
		MessageDeregister(AiPlayCt->msgHandle);
		AiPlayCt->msgHandle = NULL;
	}

	//PortFree
	AiPlayCt->AudioCoreAi = NULL;

#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(AiPlayCt->SourceDecoder != NULL)
	{
		osPortFree(AiPlayCt->SourceDecoder);
		AiPlayCt->SourceDecoder = NULL;
	}
#endif


	osPortFree(AiPlayCt);
	AiPlayCt = NULL;
	APP_DBG("Kill Ct\n");

#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	osTaskDelay(5);//不加延时，下面api容易死机
//	AudioEffectsDeInit();
#endif

	return TRUE;
}

MessageHandle GetAiPlayMessageHandle(void)
{
	if(AiPlayCt != NULL)
	{
		return AiPlayCt->msgHandle;
	}
	else
	{
		return NULL;
	}
}


#endif//#ifdef CFG_APP_AI_MODE_EN

#ifdef	CFG_AI_ENCODE_EN

#define XIAOAI_TASK_SIZE			5*1024//512//512//1024
#define XIAOAI_TASK_PRIO			4
#define XIAOAI_TASK_TIMEOUT			0xFFFFFFFF		/** 阻塞  **/

#define XIAOAI_TASK_QUEUE			10

typedef struct _XiaoAiTaskContext
{
	xTaskHandle			taskHandle;
	MessageHandle		msgHandle;
	MessageHandle		parentMsgHandle;

	TaskState			serviceState;
}XiaoAiTaskContext;

static XiaoAiTaskContext		XiaoAiTaskCt;

static void XiaoAiTaskEntrance(void * param);

static int32_t XiaoAiTaskInit(MessageHandle parentMsgHandle)
{
	memset(&XiaoAiTaskCt, 0, sizeof(XiaoAiTaskContext));

	/* register message handle */
	XiaoAiTaskCt.msgHandle = MessageRegister(XIAOAI_TASK_QUEUE);
	if(XiaoAiTaskCt.msgHandle == NULL)
	{
		APP_DBG("XiaoAiTask mem create fail!\n");
		return -1;
	}
	XiaoAiTaskCt.parentMsgHandle = parentMsgHandle;
	XiaoAiTaskCt.serviceState = TaskStateCreating;

	return 0;
}

void XiaoAiTaskMsgSendRun()
{
	MessageContext		msgSend;

	OpusEncodedLenSet(0);
	msgSend.msgId		= MSG_NONE;
	MessageSend(XiaoAiTaskCt.msgHandle, &msgSend);
}

void XiaoAiTaskMsgSendStart()
{
	MessageContext		msgSend;

	msgSend.msgId		= MSG_TASK_START;
	MessageSend(XiaoAiTaskCt.msgHandle, &msgSend);
}

void XiaoAiTaskMsgSendStop()
{
	MessageContext		msgSend;

	msgSend.msgId		= MSG_TASK_STOP;
	MessageSend(XiaoAiTaskCt.msgHandle, &msgSend);
}

void XiaoAiStopMsgSendtoMain()
{
	MessageContext		msgSend;

	msgSend.msgId		= MSG_BT_XM_AI_STOP;
	MessageSend(mainAppCt.msgHandle, &msgSend);
}

xTaskHandle XmAiGetTaskHandle()
{
	return XiaoAiTaskCt.taskHandle;
}


int32_t XmAiPlayCreate(MessageHandle parentMsgHandle)
{
	int32_t		ret = 0;

	XiaoAiTaskInit(parentMsgHandle);
	if(!ret)
	{
		XiaoAiTaskCt.taskHandle = NULL;
		xTaskCreate(XiaoAiTaskEntrance,
					"XiaoAiTask",
					XIAOAI_TASK_SIZE,
					NULL,
					XIAOAI_TASK_PRIO,
					&XiaoAiTaskCt.taskHandle);
		if(XiaoAiTaskCt.taskHandle == NULL)
		{
			ret = -1;
		}
	}
	if(ret)
	{
		APP_DBG("AudioCoreService create fail!\n");
	}
	return ret;
}


uint32_t XmAiPlayKill(void)
{
	if(XiaoAiTaskCt.taskHandle)
	{
		vTaskDelete(XiaoAiTaskCt.taskHandle);
		XiaoAiTaskCt.taskHandle = NULL;
	}
	if(XiaoAiTaskCt.msgHandle)
	{
		MessageDeregister(XiaoAiTaskCt.msgHandle);
		XiaoAiTaskCt.msgHandle = NULL;
	}

	return 0;
}

//按键或者其他方式调用
void ai_start(void)
{
	AiPlayCreate();

#ifdef CFG_FUNC_AI_EN
	AIXiaoQ_start();
#endif
}

bool AiPlayCreate()//(MessageHandle parentMsgHandle)
{
	bool		ret = TRUE;


#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	AudioEffectModeSel(EFFECT_MODE_YuanSheng, 2);//0=init hw,1=effect,2=hw+effect
	vTaskDelay(100);
#endif

#ifdef CFG_FUNC_AI_EN
	ret = AiPlay_Init();//(parentMsgHandle);
	if(get_ai_encoder_f())
	{
		send_data_to_phone();
		set_ai_encoder_f(0);
	}

#endif

#ifdef CFG_XIAOAI_AI_EN
	XmAiPlayCreate(mainAppCt.msgHandle);
#endif

	return ret;
}


static void XiaoAiTaskEntrance(void * param)
{
	MessageContext		msgRecv;

	aivs_speech_start();//像手机发送请求。
	while(1)
	{
		MessageRecv(XiaoAiTaskCt.msgHandle, &msgRecv, XIAOAI_TASK_TIMEOUT);

		switch(msgRecv.msgId)
		{
			case MSG_TASK_START:
				if(XiaoAiTaskCt.serviceState == TaskStateCreating)
				{

					XiaoAiTaskCt.serviceState = TaskStateRunning;
				}
				break;
			case MSG_TASK_STOP:
				XiaoAiTaskCt.serviceState = TaskStateStopping;
				XiaoAiStopMsgSendtoMain();
				break;
		}

		if(XiaoAiTaskCt.serviceState == TaskStateRunning)
			OpusEncoderTaskPro();
	}
}


#endif //CFG_AI_ENCODE_EN

