/**
 **************************************************************************************
 * @file    waiting_mode.c
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

#define WAITING_PLAY_TASK_STACK_SIZE		512//1024
#define WAITING_PLAY_TASK_PRIO				3
#define WAITING_NUM_MESSAGE_QUEUE			5


typedef struct _WaitingPlayContext
{
	xTaskHandle 		taskHandle;
	MessageHandle		msgHandle;
	MessageHandle		parentMsgHandle;

	TaskState			state;
	AudioCoreContext 	*AudioCoreWaiting;
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	uint16_t*			Source1Decoder;
	TaskState			DecoderSync;
#endif

	//play
	uint32_t 			SampleRate; //����ʾ��ʱ��������ز�����Ҫ������������ó�ͻ

	uint32_t			WaitingTick;//WaitingPlay������ʱ�������ֵ��
}WaitingPlayContext;


/**����appconfigȱʡ����:DMA 8��ͨ������**/
/*1��cec��PERIPHERAL_ID_TIMER3*/
/*2��SD��¼����PERIPHERAL_ID_SDIO RX/TX*/
/*3�����ߴ��ڵ�����PERIPHERAL_ID_UART1 RX/TX,����ʹ��USB HID����ʡDMA��Դ*/
/*4����·������PERIPHERAL_ID_AUDIO_ADC0_RX*/
/*5��Mic������PERIPHERAL_ID_AUDIO_ADC1_RX��mode֮��ͨ������һ��*/
/*6��Dac0������PERIPHERAL_ID_AUDIO_DAC0_TX mode֮��ͨ������һ��*/
/*7��DacX�迪��PERIPHERAL_ID_AUDIO_DAC1_TX mode֮��ͨ������һ��*/
/*ע��DMA 8��ͨ�����ó�ͻ:*/
/*a��UART���ߵ�����DAC-X�г�ͻ,Ĭ�����ߵ���ʹ��USB HID*/
/*b��UART���ߵ�����HDMI/SPDIFģʽ��ͻ*/
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

static  WaitingPlayContext*		WaitingPlayCt;


static void WaitingPlayModeCreating(uint16_t msgId);
static void WaitingPlayModeStarting(uint16_t msgId);
static void WaitingPlayModeStopping(uint16_t msgId);
static void WaitingPlayModeStopped(void);
static void WaitingPlayRunning(uint16_t msgId);
uint8_t WaitingPlayDecoderSourceNum(void);

static bool WaitingPlay_Init(MessageHandle parentMsgHandle)
{
//System config
	DMA_ChannelAllocTableSet((uint8_t *)DmaChannelMap);
//Task & App Config
	WaitingPlayCt = (WaitingPlayContext*)osPortMalloc(sizeof(WaitingPlayContext));
	if(WaitingPlayCt == NULL)
	{
		return FALSE;
	}
	memset(WaitingPlayCt, 0, sizeof(WaitingPlayContext));
	WaitingPlayCt->msgHandle = MessageRegister(WAITING_NUM_MESSAGE_QUEUE);
	if(WaitingPlayCt->msgHandle == NULL)
	{
		return FALSE;
	}
	WaitingPlayCt->parentMsgHandle = parentMsgHandle;
	WaitingPlayCt->state = TaskStateCreating;
	WaitingPlayCt->SampleRate = CFG_PARA_SAMPLE_RATE;

	//Core Source1 para
	WaitingPlayCt->AudioCoreWaiting = (AudioCoreContext*)&AudioCore;
	//Audio init
//	//note Soure0.��sink0�Ѿ���main app�����ã���Ҫ��������
	//InCore1 buf
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	WaitingPlayCt->Source1Decoder = (uint16_t*)osPortMalloc(mainAppCt.SamplesPreFrame * 2 * 2);//One Frame
	if(WaitingPlayCt->Source1Decoder == NULL)
	{
		return FALSE;
	}
	memset(WaitingPlayCt->Source1Decoder, 0, mainAppCt.SamplesPreFrame * 2 * 2);//2K

	//Core Soure2 Para
	DecoderSourceNumSet(REMIND_SOURCE_NUM);
	WaitingPlayCt->AudioCoreWaiting->AudioSource[REMIND_SOURCE_NUM].Enable = 0;
	WaitingPlayCt->AudioCoreWaiting->AudioSource[REMIND_SOURCE_NUM].FuncDataGet = DecodedPcmDataGet;
	WaitingPlayCt->AudioCoreWaiting->AudioSource[REMIND_SOURCE_NUM].FuncDataGetLen = NULL;
	WaitingPlayCt->AudioCoreWaiting->AudioSource[REMIND_SOURCE_NUM].IsSreamData = FALSE;//Decoder
	WaitingPlayCt->AudioCoreWaiting->AudioSource[REMIND_SOURCE_NUM].PcmFormat = 2; //stereo
	WaitingPlayCt->AudioCoreWaiting->AudioSource[REMIND_SOURCE_NUM].PcmInBuf = (int16_t *)WaitingPlayCt->Source1Decoder;
#endif

	//Core Process

#ifdef CFG_FUNC_AUDIO_EFFECT_EN
    #ifdef CFG_FUNC_MIC_KARAOKE_EN
	WaitingPlayCt->AudioCoreWaiting->AudioEffectProcess = (AudioCoreProcessFunc)AudioEffectProcess;
	#else
	WaitingPlayCt->AudioCoreWaiting->AudioEffectProcess = (AudioCoreProcessFunc)AudioMusicProcess;
	#endif
#else
	WaitingPlayCt->AudioCoreWaiting->AudioEffectProcess = (AudioCoreProcessFunc)AudioBypassProcess;
#endif

	WaitingPlayCt->WaitingTick = GetSysTick1MsCnt();
	return TRUE;
}

//��������services
static void WaitingPlayModeCreate(void)
{
	bool NoService = TRUE;
	
	// Create service task
#if defined(CFG_FUNC_REMIND_SBC)
	DecoderServiceCreate(WaitingPlayCt->msgHandle, DECODER_BUF_SIZE_SBC, DECODER_FIFO_SIZE_FOR_SBC);//��ʾ����ʽ�����������ڴ�����
	NoService = FALSE;
#elif defined(CFG_FUNC_REMIND_SOUND_EN)
	DecoderServiceCreate(WaitingPlayCt->msgHandle, DECODER_BUF_SIZE_MP3, DECODER_FIFO_SIZE_FOR_MP3);
	NoService = FALSE;
#endif
	if(NoService)
	{
		WaitingPlayModeCreating(MSG_NONE);
	}
}

//All of services is created
//Send CREATED message to parent
static void WaitingPlayModeCreating(uint16_t msgId)
{
	MessageContext		msgSend;
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(msgId == MSG_DECODER_SERVICE_CREATED)
	{
		APP_DBG("Decoder created\n");
		WaitingPlayCt->DecoderSync = TaskStateReady;
	}
	if(WaitingPlayCt->DecoderSync == TaskStateReady)
#endif
	{
		msgSend.msgId		= MSG_WAITING_PLAY_MODE_CREATED;
		MessageSend(WaitingPlayCt->parentMsgHandle, &msgSend);
		WaitingPlayCt->state = TaskStateReady;
	}
}

static void WaitingPlayModeStart(void)
{
	bool NoService = TRUE;
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	DecoderServiceStart();
	WaitingPlayCt->DecoderSync = TaskStateStarting;
	NoService = FALSE;
#endif
	if(NoService)
	{
		WaitingPlayModeStarting(MSG_NONE);
	}
	else
	{
		WaitingPlayCt->state = TaskStateStarting;
	}
}

static void WaitingPlayModeStarting(uint16_t msgId)
{
	MessageContext		msgSend;
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(msgId == MSG_DECODER_SERVICE_STARTED)
	{
		APP_DBG("Decoder startted\n");
		WaitingPlayCt->DecoderSync = TaskStateRunning;
	}
	if(WaitingPlayCt->DecoderSync == TaskStateRunning)
#endif
	{
		msgSend.msgId		= MSG_WAITING_PLAY_MODE_STARTED;
		MessageSend(WaitingPlayCt->parentMsgHandle, &msgSend);
		WaitingPlayCt->state = TaskStateRunning;
		
#ifdef CFG_FUNC_REMIND_SOUND_EN
		if(SoftFlagGet(SoftFlagWaitSysRemind))
		{
			//������ʾ���ܣ������ſ�����ʾ��ʱ�������������д���
			RemindSoundServiceItemRequest(mainAppCt.SysRemind, FALSE); //�岥ϵͳ��ʾ��
		}
		else
		{
			//������Դʱ��δ����ʾ��
		}
#endif
	}
}

static void WaitingPlayModeStop(void)
{
	bool NoService = TRUE;
#if	defined(CFG_FUNC_REMIND_SOUND_EN) 
	if(WaitingPlayCt->DecoderSync != TaskStateNone && WaitingPlayCt->DecoderSync != TaskStateStopped)
	{//�������� ��app kill
		DecoderServiceStop();
		WaitingPlayCt->DecoderSync = TaskStateStopping;
		NoService = FALSE;
	}
#endif
	WaitingPlayCt->state = TaskStateStopping;
	if(NoService)
	{
		WaitingPlayModeStopped();
	}
}

static void WaitingPlayModeStopping(uint16_t msgId)//������service ������kill
{
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(msgId == MSG_DECODER_SERVICE_STOPPED)
	{
		WaitingPlayCt->DecoderSync = TaskStateNone;
	}
#endif
	if((WaitingPlayCt->state == TaskStateStopping)
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
		&& (WaitingPlayCt->DecoderSync == TaskStateNone)
#endif
	)
	{
		WaitingPlayModeStopped();
	}
}

static void WaitingPlayModeStopped(void)
{
	MessageContext		msgSend;
	//Set para
	
	//clear msg
	MessageClear(WaitingPlayCt->msgHandle);
	
	//Set state
	WaitingPlayCt->state = TaskStateStopped;

	//reply
	msgSend.msgId		= MSG_WAITING_PLAY_MODE_STOPPED;
	MessageSend(WaitingPlayCt->parentMsgHandle, &msgSend);
}

bool WaitingPlayPermissionMode(void)
{
	MessageContext		msgSend;
	if(WaitingPlayCt == NULL)
	{
		return FALSE;
	}
	msgSend.msgId		= MSG_WAITING_PERMISSION_MODE;
	return MessageSend(WaitingPlayCt->parentMsgHandle, &msgSend);
}


static void WaitingPlayEntrance(void * param)
{
	MessageContext		msgRecv;
	
	APP_DBG("App\n");
	// Create services
	WaitingPlayModeCreate();
#ifdef CFG_FUNC_AUDIO_EFFECT_EN	
	AudioEffectModeSel(mainAppCt.EffectMode, 2);//0=init hw,1=effect,2=hw+effect
#ifdef CFG_COMMUNICATION_BY_UART
	UART1_Communication_Init((void *)(&UartRxBuf[0]), 1024, (void *)(&UartTxBuf[0]), 1024);
#endif
#endif

	while(1)
	{
		MessageRecv(WaitingPlayCt->msgHandle, &msgRecv, 50);//û��Ϣʱ50ms��һ�Ρ�
		if(WaitingPlayCt->state != TaskStateRunning)
		{
			switch(msgRecv.msgId)//���棺�ڴ˶δ��룬��ֹ������ʾ���岥λ�á�
			{	
				case MSG_DECODER_SERVICE_CREATED:
					WaitingPlayModeCreating(msgRecv.msgId);
					break;
		
				case MSG_TASK_START:
					WaitingPlayModeStart();
					break;
				case MSG_DECODER_SERVICE_STARTED:
					//RemindSound request		
					WaitingPlayModeStarting(msgRecv.msgId);
					break;

				case MSG_TASK_STOP: //Created ʱstop
#ifdef CFG_COMMUNICATION_BY_USB
					NVIC_DisableIRQ(Usb_IRQn);
					OTG_DeviceDisConnect();
#endif
					WaitingPlayModeStop();
					break;

				case MSG_DECODER_SERVICE_STOPPED:
					WaitingPlayModeStopping(msgRecv.msgId);
					break;
			}
		}
		else //WaitingPlayCt->state != TaskStateRunning
		{
			WaitingPlayRunning(msgRecv.msgId);
		}
		if(SoftFlagGet(SoftFlagWaitDetect) && GetSysTick1MsCnt() - WaitingPlayCt->WaitingTick > MODE_WAIT_DEVICE_TIME)
		{
			SoftFlagDeregister (SoftFlagWaitDetect);
			if(!SoftFlagGet(SoftFlagWaitModeMask))
			{
				WaitingPlayPermissionMode();
			}
		}
	}
}

static void WaitingPlayRunning(uint16_t msgId)
{
	switch(msgId)
	{
		case MSG_TASK_STOP:
#ifdef CFG_FUNC_REMIND_SOUND_EN 
			RemindSoundServiceReset();
#endif
#ifdef CFG_COMMUNICATION_BY_USB
			NVIC_DisableIRQ(Usb_IRQn);
			OTG_DeviceDisConnect();
#endif
			WaitingPlayModeStop();
			break;

#ifdef	CFG_FUNC_POWERKEY_EN
		case MSG_TASK_POWERDOWN:
			APP_DBG("MSG receive PowerDown, Please breakpoint\n");
			SystemPowerDown();
			break;
#endif

#ifdef CFG_FUNC_REMIND_SOUND_EN
		case MSG_DECODER_STOPPED:
			{
				MessageContext		msgSend;
				msgSend.msgId = msgId;
				MessageSend(GetRemindSoundServiceMessageHandle(), &msgSend);//��ʾ���ڼ�ת����������Ϣ��
			}
			break;

		case MSG_REMIND_SOUND_PLAY_REQUEST_FAIL:
		case MSG_DECODER_RESET://���������úͻ��գ��ٽ���Ϣ,��ʾ��������ǡ�
			//�״�����ģʽʱ & �ȴ�������ʾ������ & device��ʱ����
			if(SoftFlagGet(SoftFlagWaitSysRemind))
			{
				SoftFlagDeregister (SoftFlagWaitSysRemind);
				if(!SoftFlagGet(SoftFlagWaitModeMask))
				{
					WaitingPlayPermissionMode();
				}
			}
			break;
#endif
		default:
			CommonMsgProccess(msgId);
			break;
	}
}

/***************************************************************************************
 *
 * APIs
 *
 */
bool WaitingPlayCreate(MessageHandle parentMsgHandle)
{
	bool		ret = TRUE;

	ret = WaitingPlay_Init(parentMsgHandle);
	if(ret)
	{
		WaitingPlayCt->taskHandle = NULL;
		xTaskCreate(WaitingPlayEntrance,
					"WaitingPlay",
					WAITING_PLAY_TASK_STACK_SIZE,
					NULL, WAITING_PLAY_TASK_PRIO,
					&WaitingPlayCt->taskHandle);
		if(WaitingPlayCt->taskHandle == NULL)
		{
			ret = FALSE;
		}
	}
	else
	{
		APP_DBG("app create fail!\n");
	}
	return ret;
}

bool WaitingPlayStart(void)
{
	MessageContext		msgSend;

	if(WaitingPlayCt == NULL)
	{
		return FALSE;
	}
	msgSend.msgId		= MSG_TASK_START;
	return MessageSend(WaitingPlayCt->msgHandle, &msgSend);
}

bool WaitingPlayPause(void)
{
	MessageContext		msgSend;
	if(WaitingPlayCt == NULL)
	{
		return FALSE;
	}
	msgSend.msgId		= MSG_TASK_PAUSE;
	return MessageSend(WaitingPlayCt->msgHandle, &msgSend);
}

bool WaitingPlayResume(void)
{
	MessageContext		msgSend;
	if(WaitingPlayCt == NULL)
	{
		return FALSE;
	}
	msgSend.msgId		= MSG_TASK_RESUME;
	return MessageSend(WaitingPlayCt->msgHandle, &msgSend);
}

bool WaitingPlayStop(void)
{
	MessageContext		msgSend;
	if(WaitingPlayCt == NULL)
	{
		return FALSE;
	}
	msgSend.msgId		= MSG_TASK_STOP;
	return MessageSend(WaitingPlayCt->msgHandle, &msgSend);
}

bool WaitingPlayKill(void)
{
	if(WaitingPlayCt == NULL)
	{
		return FALSE;
	}

	//Kill used services
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	AudioCoreSourceDisable(REMIND_SOURCE_NUM);
	DecoderServiceKill();
#endif

	//ע�⣺AudioCore�����������mainApp�£��˴�ֻ�ر�AudioCoreͨ�������ر�����
	AudioCoreProcessConfig((void*)AudioNoAppProcess);

	//task
	if(WaitingPlayCt->taskHandle != NULL)
	{
		vTaskDelete(WaitingPlayCt->taskHandle);
		WaitingPlayCt->taskHandle = NULL;
	}

	//Msgbox
	if(WaitingPlayCt->msgHandle != NULL)
	{
		MessageDeregister(WaitingPlayCt->msgHandle);
		WaitingPlayCt->msgHandle = NULL;
	}

	//PortFree
	WaitingPlayCt->AudioCoreWaiting = NULL;

#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(WaitingPlayCt->Source1Decoder != NULL)
	{
		osPortFree(WaitingPlayCt->Source1Decoder);
		WaitingPlayCt->Source1Decoder = NULL;
	}
#endif

	osPortFree(WaitingPlayCt);
	WaitingPlayCt = NULL;
	APP_DBG("Kill Ct\n");
#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	AudioEffectsDeInit();
#endif

	return TRUE;
}

MessageHandle GetWaitingPlayMessageHandle(void)
{
	if(WaitingPlayCt != NULL)
	{
		return WaitingPlayCt->msgHandle;
	}
	else
	{
		return NULL;
	}
}
