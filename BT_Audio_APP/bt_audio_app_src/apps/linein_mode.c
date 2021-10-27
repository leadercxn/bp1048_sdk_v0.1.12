/**
 **************************************************************************************
 * @file    linein_mode.c
 * @brief   
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2017-12-26 13:06:47$
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
#include "recorder_service.h"
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
#include "device_service.h"
void AudioEffectModeSel(uint8_t mode, uint8_t init_flag);//0=hw,1=effect,2=hw+effect ff= no init
#ifdef CFG_APP_LINEIN_MODE_EN

#define LINEIN_PLAY_TASK_STACK_SIZE		512//1024
#define LINEIN_PLAY_TASK_PRIO			3
#define LINEIN_NUM_MESSAGE_QUEUE		10

#define LINEIN_SOURCE_NUM				APP_SOURCE_NUM

typedef struct _LineInPlayContext
{
	xTaskHandle 		taskHandle;
	MessageHandle		msgHandle;
	MessageHandle		parentMsgHandle;

	TaskState			state;

	uint32_t			*ADCFIFO;			//ADC��DMAѭ��fifo
	uint16_t 			*Source1Buf_LineIn;	//ADC ȡLineIn����
	AudioCoreContext 	*AudioCoreLineIn;
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	uint16_t*			SourceDecoder;
	TaskState			DecoderSync;
#endif
	//play
	uint32_t 			SampleRate; //����ʾ��ʱ��������ز�����Ҫ������������ó�ͻ

#ifdef CFG_FUNC_RECORDER_EN
	TaskState			RecorderSync;
#endif

}LineInPlayContext;

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

static  LineInPlayContext*		LineInPlayCt;


static void LineInPlayModeCreating(uint16_t msgId);
static void LineInPlayModeStarting(uint16_t msgId);
static void LineInPlayModeStopping(uint16_t msgId);
static void LineInPlayModeStopped(void);
static void LineInPlayRunning(uint16_t msgId);

void LineInPlayResRelease(void)
{
#if (LINEIN_INPUT_CHANNEL == ANA_INPUT_CH_LINEIN3)
	AudioADC_Disable(ADC1_MODULE);
	//Reset_FunctionReset(DMAC_FUNC_SEPA);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC1_RX, DMA_DONE_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC1_RX, DMA_THRESHOLD_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC1_RX, DMA_ERROR_INT);
	DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC1_RX);
#else
	AudioADC_Disable(ADC0_MODULE);
	//Reset_FunctionReset(DMAC_FUNC_SEPA);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC0_RX, DMA_DONE_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC0_RX, DMA_THRESHOLD_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC0_RX, DMA_ERROR_INT);
	DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC0_RX);
#endif
	if(LineInPlayCt->ADCFIFO != NULL)
	{
		APP_DBG("ADCFIFO\n");
		osPortFree(LineInPlayCt->ADCFIFO);
		LineInPlayCt->ADCFIFO = NULL;
	}
	if(LineInPlayCt->Source1Buf_LineIn != NULL)
	{
		APP_DBG("Source1Buf_LineIn\n");
		osPortFree(LineInPlayCt->Source1Buf_LineIn);
		LineInPlayCt->Source1Buf_LineIn = NULL;
	}
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(LineInPlayCt->SourceDecoder != NULL)
	{
		APP_DBG("SourceDecoder\n");
		osPortFree(LineInPlayCt->SourceDecoder);
		LineInPlayCt->SourceDecoder = NULL;
	}
#endif
}

bool LineInPlayResMalloc(uint16_t SampleLen)
{
	//LineIn5  digital (DMA)
	LineInPlayCt->ADCFIFO = (uint32_t*)osPortMallocFromEnd(SampleLen * 2 * 2 * 2);
	if(LineInPlayCt->ADCFIFO == NULL)
	{
		return FALSE;
	}
	memset(LineInPlayCt->ADCFIFO, 0, SampleLen * 2 * 2 * 2);

	//InCore1 buf
	LineInPlayCt->Source1Buf_LineIn = (uint16_t*)osPortMallocFromEnd(SampleLen * 2 * 2);
	if(LineInPlayCt->Source1Buf_LineIn == NULL)
	{
		return FALSE;
	}
	memset(LineInPlayCt->Source1Buf_LineIn, 0, SampleLen * 2 * 2);
		//InCore buf
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	LineInPlayCt->SourceDecoder = (uint16_t*)osPortMallocFromEnd(SampleLen * 2 * 2);//One Frame
	if(LineInPlayCt->SourceDecoder == NULL)
	{
		return FALSE;
	}
	memset(LineInPlayCt->SourceDecoder, 0, SampleLen * 2 * 2);//2K
#endif

	return TRUE;
}

void LineInPlayResInit(void)
{
	if(LineInPlayCt->Source1Buf_LineIn != NULL)
	{
		LineInPlayCt->AudioCoreLineIn->AudioSource[LINEIN_SOURCE_NUM].PcmInBuf = (int16_t *)LineInPlayCt->Source1Buf_LineIn;
	}
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(LineInPlayCt->SourceDecoder != NULL)
	{
		LineInPlayCt->AudioCoreLineIn->AudioSource[REMIND_SOURCE_NUM].PcmInBuf = (int16_t *)LineInPlayCt->SourceDecoder;
	}
#endif
#if (LINEIN_INPUT_CHANNEL == ANA_INPUT_CH_LINEIN3)
	//DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC1_RX);
	DMA_CircularConfig(PERIPHERAL_ID_AUDIO_ADC1_RX, mainAppCt.SamplesPreFrame * 2 * 2, (void*)LineInPlayCt->ADCFIFO, mainAppCt.SamplesPreFrame * 2 * 2 * 2);
	if(AudioADC_IsOverflow(ADC1_MODULE))
	{
		AudioADC_OverflowClear(ADC1_MODULE);
	}
	AudioADC_Clear(ADC1_MODULE);
    DMA_ChannelEnable(PERIPHERAL_ID_AUDIO_ADC1_RX);
    AudioADC_LREnable(ADC1_MODULE, 1, 1);
    AudioADC_Enable(ADC1_MODULE);
#else
	//DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC0_RX);
	DMA_CircularConfig(PERIPHERAL_ID_AUDIO_ADC0_RX, mainAppCt.SamplesPreFrame * 2 * 2, (void*)LineInPlayCt->ADCFIFO, mainAppCt.SamplesPreFrame * 2 * 2 * 2);
	if(AudioADC_IsOverflow(ADC0_MODULE))
	{
		AudioADC_OverflowClear(ADC0_MODULE);
	}
	AudioADC_Clear(ADC0_MODULE);
    DMA_ChannelEnable(PERIPHERAL_ID_AUDIO_ADC0_RX);
    AudioADC_LREnable(ADC0_MODULE, 1, 1);
    AudioADC_Enable(ADC0_MODULE);
#endif
}

/**
 * @func        LineInPlay_Init
 * @brief       LineInģʽ�������ã���Դ��ʼ��
 * @param       MessageHandle parentMsgHandle
 * @Output      None
 * @return      bool
 * @Others      ����顢Adc��Dac��AudioCore����
 * @Others      ��������Adc��audiocore���к���ָ�룬audioCore��Dacͬ����audiocoreService����������
 * Record
 */
static bool LineInPlay_Init(MessageHandle parentMsgHandle)
{
//System config
	DMA_ChannelAllocTableSet((uint8_t *)DmaChannelMap);//lineIn
//Task & App Config
	LineInPlayCt = (LineInPlayContext*)osPortMalloc(sizeof(LineInPlayContext));
	if(LineInPlayCt == NULL)
	{
		return FALSE;
	}
	memset(LineInPlayCt, 0, sizeof(LineInPlayContext));
	LineInPlayCt->msgHandle = MessageRegister(LINEIN_NUM_MESSAGE_QUEUE);
	if(LineInPlayCt->msgHandle == NULL)
	{
		return FALSE;
	}
	LineInPlayCt->parentMsgHandle = parentMsgHandle;
	LineInPlayCt->state = TaskStateCreating;
	LineInPlayCt->SampleRate = CFG_PARA_SAMPLE_RATE;

	if(!LineInPlayResMalloc(mainAppCt.SamplesPreFrame))
	{
		APP_DBG("LineInPlay Res Error!\n");
		return FALSE;
	}
	
#if (LINEIN_INPUT_CHANNEL == ANA_INPUT_CH_LINEIN3)
	AudioADC_DigitalInit(ADC1_MODULE, LineInPlayCt->SampleRate, (void*)LineInPlayCt->ADCFIFO, mainAppCt.SamplesPreFrame * 2 * 2 * 2);
#else
	AudioADC_DigitalInit(ADC0_MODULE, LineInPlayCt->SampleRate, (void*)LineInPlayCt->ADCFIFO, mainAppCt.SamplesPreFrame * 2 * 2 * 2);
#endif
	
	//Core Source1 para
	LineInPlayCt->AudioCoreLineIn = (AudioCoreContext*)&AudioCore;
	//Audio init
//	//note Soure0.��sink0�Ѿ���main app�����ã���Ҫ��������
	//Core Soure1.Para
	LineInPlayCt->AudioCoreLineIn->AudioSource[LINEIN_SOURCE_NUM].Enable = 0;

#if (LINEIN_INPUT_CHANNEL == ANA_INPUT_CH_LINEIN3)
	LineInPlayCt->AudioCoreLineIn->AudioSource[LINEIN_SOURCE_NUM].FuncDataGet = AudioADC1DataGet;
	LineInPlayCt->AudioCoreLineIn->AudioSource[LINEIN_SOURCE_NUM].FuncDataGetLen = AudioADC1DataLenGet;
#else
	LineInPlayCt->AudioCoreLineIn->AudioSource[LINEIN_SOURCE_NUM].FuncDataGet = AudioADC0DataGet;
	LineInPlayCt->AudioCoreLineIn->AudioSource[LINEIN_SOURCE_NUM].FuncDataGetLen = AudioADC0DataLenGet;
#endif

	LineInPlayCt->AudioCoreLineIn->AudioSource[LINEIN_SOURCE_NUM].IsSreamData = TRUE;
	LineInPlayCt->AudioCoreLineIn->AudioSource[LINEIN_SOURCE_NUM].PcmFormat = 2;//lineIn stereo
	LineInPlayCt->AudioCoreLineIn->AudioSource[LINEIN_SOURCE_NUM].PcmInBuf = (int16_t *)LineInPlayCt->Source1Buf_LineIn;

	//Core Soure Para
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	DecoderSourceNumSet(REMIND_SOURCE_NUM);
	LineInPlayCt->AudioCoreLineIn->AudioSource[REMIND_SOURCE_NUM].Enable = 0;
	LineInPlayCt->AudioCoreLineIn->AudioSource[REMIND_SOURCE_NUM].FuncDataGet = DecodedPcmDataGet;
	LineInPlayCt->AudioCoreLineIn->AudioSource[REMIND_SOURCE_NUM].FuncDataGetLen = NULL;
	LineInPlayCt->AudioCoreLineIn->AudioSource[REMIND_SOURCE_NUM].IsSreamData = FALSE;//Decoder
	LineInPlayCt->AudioCoreLineIn->AudioSource[REMIND_SOURCE_NUM].PcmFormat = 2; //stereo
	LineInPlayCt->AudioCoreLineIn->AudioSource[REMIND_SOURCE_NUM].PcmInBuf = (int16_t *)LineInPlayCt->SourceDecoder;
#endif

	//Core Process	
#ifdef CFG_FUNC_AUDIO_EFFECT_EN
    #ifdef CFG_FUNC_MIC_KARAOKE_EN
	LineInPlayCt->AudioCoreLineIn->AudioEffectProcess = (AudioCoreProcessFunc)AudioEffectProcess;
	#else
	LineInPlayCt->AudioCoreLineIn->AudioEffectProcess = (AudioCoreProcessFunc)AudioMusicProcess;
	#endif
#else
	LineInPlayCt->AudioCoreLineIn->AudioEffectProcess = (AudioCoreProcessFunc)AudioBypassProcess;	
#endif

#ifdef CFG_FUNC_RECORDER_EN
	LineInPlayCt->RecorderSync = TaskStateNone;
#endif

	AudioCoreSourceMute(APP_SOURCE_NUM, TRUE, TRUE);
	return TRUE;
}

//��������services
static void LineInPlayModeCreate(void)
{
	bool NoService = TRUE;
	
	// Create service task
#if defined(CFG_FUNC_REMIND_SBC)
	DecoderServiceCreate(LineInPlayCt->msgHandle, DECODER_BUF_SIZE_SBC, DECODER_FIFO_SIZE_FOR_SBC);//��ʾ����ʽ�����������ڴ�����
	NoService = FALSE;
#elif defined(CFG_FUNC_REMIND_SOUND_EN)
	DecoderServiceCreate(LineInPlayCt->msgHandle, DECODER_BUF_SIZE_MP3, DECODER_FIFO_SIZE_FOR_MP3);
	NoService = FALSE;
#endif
	if(NoService)
	{
		LineInPlayModeCreating(MSG_NONE);
	}
}

//All of services is created
//Send CREATED message to parent
static void LineInPlayModeCreating(uint16_t msgId)
{
	MessageContext		msgSend;
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(msgId == MSG_DECODER_SERVICE_CREATED)
	{
		LineInPlayCt->DecoderSync = TaskStateReady;
	}
	if(LineInPlayCt->DecoderSync == TaskStateReady)
#endif
	{
		msgSend.msgId		= MSG_LINE_AUDIO_MODE_CREATED;
		MessageSend(LineInPlayCt->parentMsgHandle, &msgSend);
		LineInPlayCt->state = TaskStateReady;
	}
}

static void LineInPlayModeStart(void)
{
	bool NoService = TRUE;
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	DecoderServiceStart();
	LineInPlayCt->DecoderSync = TaskStateStarting;
	NoService = FALSE;
#endif
	if(NoService)
	{
		LineInPlayModeStarting(MSG_NONE);
	}
	else
	{
		LineInPlayCt->state = TaskStateStarting;
	}
}

static void LineInPlayModeStarting(uint16_t msgId)
{
	MessageContext		msgSend;
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(msgId == MSG_DECODER_SERVICE_STARTED)
	{
		LineInPlayCt->DecoderSync = TaskStateRunning;
	}
	if(LineInPlayCt->DecoderSync == TaskStateRunning)
#endif
	{
		msgSend.msgId		= MSG_LINE_AUDIO_MODE_STARTED;
		MessageSend(LineInPlayCt->parentMsgHandle, &msgSend);

		AudioCoreSourceMute(APP_SOURCE_NUM, TRUE, TRUE);
		vTaskDelay(20);
		AudioCoreSourceEnable(LINEIN_SOURCE_NUM);
		LineInPlayCt->state = TaskStateRunning;

#ifndef CFG_FUNC_REMIND_SOUND_EN			
		AudioCoreSourceUnmute(APP_SOURCE_NUM, TRUE, TRUE);	
#else
		if(!SoftFlagGet(SoftFlagRemindMask))
		{
			if(!RemindSoundServiceItemRequest(SOUND_REMIND_XIANLUMO,FALSE)) //�岥��ʾ����ģʽline����
			{
				APP_DBG("Line In No Sound\n");
				AudioCoreSourceUnmute(APP_SOURCE_NUM, TRUE, TRUE);			
			}
		}
		else		
		{			
			SoftFlagDeregister(SoftFlagRemindMask);
			if(SoftFlagGet(SoftFlagDiscDelayMask))
			{
				SoftFlagDeregister(SoftFlagDiscDelayMask);
				RemindSoundServiceItemRequest(SOUND_REMIND_DISCONNE, FALSE);
			}
			else
				AudioCoreSourceUnmute(APP_SOURCE_NUM, TRUE, TRUE);
		}
#endif
	}
}

static void LineInPlayModeStop(void)
{
	bool NoService = TRUE;
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(LineInPlayCt->DecoderSync != TaskStateNone && LineInPlayCt->DecoderSync != TaskStateStopped)
	{//�������� ��app kill
		DecoderServiceStop();
		LineInPlayCt->DecoderSync = TaskStateStopping;
		NoService = FALSE;
	}
#endif
#ifdef CFG_FUNC_RECORDER_EN
	if(LineInPlayCt->RecorderSync != TaskStateNone)
	{//��service ������Kill
		MediaRecorderServiceStop();
		LineInPlayCt->RecorderSync = TaskStateStopping;
		NoService = FALSE;
	}

#endif
	LineInPlayCt->state = TaskStateStopping;
	if(NoService)
	{
		LineInPlayModeStopped();
	}
}

static void LineInPlayModeStopping(uint16_t msgId)//������service ������kill
{
#ifdef CFG_FUNC_RECORDER_EN
	if(msgId == MSG_MEDIA_RECORDER_SERVICE_STOPPED)
	{
		LineInPlayCt->RecorderSync = TaskStateNone;
		APP_DBG("Line:RecorderKill");
		MediaRecorderServiceKill();
	}
#endif	
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(msgId == MSG_DECODER_SERVICE_STOPPED)
	{
		LineInPlayCt->DecoderSync = TaskStateNone;
	}
#endif
	if((LineInPlayCt->state == TaskStateStopping)
#ifdef CFG_FUNC_RECORDER_EN
		&& (LineInPlayCt->RecorderSync == TaskStateNone)
#endif
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
		&& (LineInPlayCt->DecoderSync == TaskStateNone)
#endif
	)
	{
		LineInPlayModeStopped();
	}
}

static void LineInPlayModeStopped(void)
{
	MessageContext		msgSend;
	//Set para
	
	//clear msg
	MessageClear(LineInPlayCt->msgHandle);
	
	//Set state
	LineInPlayCt->state = TaskStateStopped;

	//reply
	msgSend.msgId		= MSG_LINE_AUDIO_MODE_STOPPED;
	MessageSend(LineInPlayCt->parentMsgHandle, &msgSend);
}

/**
 * @func        LineInPlayEntrance
 * @brief       ģʽִ������
 * @param       void * param  
 * @Output      None
 * @return      None
 * @Others      ģʽ�����ͽ�������
 * Record
 */
static void LineInPlayEntrance(void * param)
{
	MessageContext		msgRecv;
	
	APP_DBG("LineIn:App\n");
	// Create services
	LineInPlayModeCreate();

#ifdef CFG_FUNC_AUDIO_EFFECT_EN	
	AudioEffectModeSel(mainAppCt.EffectMode, 2);//0=init hw,1=effect,2=hw+effect
#ifdef CFG_COMMUNICATION_BY_UART
	UART1_Communication_Init((void *)(&UartRxBuf[0]), 1024, (void *)(&UartTxBuf[0]), 1024);
#endif
#endif

	AudioAnaChannelSet(LINEIN_INPUT_CHANNEL);

#ifdef CFG_FUNC_BREAKPOINT_EN
	BackupInfoUpdata(BACKUP_SYS_INFO);
#endif

//	//�������ģʽAudioCoreΪ����״̬����unmute
//	if(IsAudioPlayerMute() == TRUE)
//	{
//		AudioPlayerMute();
//	}
#if (CFG_RES_MIC_SELECT) && defined(CFG_FUNC_AUDIO_EFFECT_EN)
	AudioCoreSourceUnmute(MIC_SOURCE_NUM, TRUE, TRUE);
#endif
	while(1)
	{
		MessageRecv(LineInPlayCt->msgHandle, &msgRecv, 100);
		
		switch(msgRecv.msgId)//���棺�ڴ˶δ��룬��ֹ������ʾ���岥λ�á�
		{	
			case MSG_DECODER_SERVICE_CREATED:
				LineInPlayModeCreating(msgRecv.msgId);
				break;

			case MSG_TASK_START:
				LineInPlayModeStart();
				break;
			case MSG_DECODER_SERVICE_STARTED:
				//RemindSound request		
				LineInPlayModeStarting(msgRecv.msgId);
				break;

			case MSG_TASK_RESUME:
				if(LineInPlayCt->state == TaskStatePaused)
				{
					LineInPlayCt->state = TaskStateRunning;
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
				LineInPlayModeStop();
				break;

			case MSG_DECODER_SERVICE_STOPPED:
			case MSG_MEDIA_RECORDER_SERVICE_STOPPED:
				LineInPlayModeStopping(msgRecv.msgId);
				break;

			case MSG_APP_RES_RELEASE:
				LineInPlayResRelease();
				{
					MessageContext		msgSend;
					msgSend.msgId		= MSG_APP_RES_RELEASE_SUC;
					MessageSend(GetMainMessageHandle(), &msgSend);
				}
				break;
			case MSG_APP_RES_MALLOC:
				LineInPlayResMalloc(mainAppCt.SamplesPreFrame);
				{
					MessageContext		msgSend;
					msgSend.msgId		= MSG_APP_RES_MALLOC_SUC;
					MessageSend(GetMainMessageHandle(), &msgSend);
				}
				break;
			case MSG_APP_RES_INIT:
				LineInPlayResInit();
				{
					MessageContext		msgSend;
					msgSend.msgId		= MSG_APP_RES_INIT_SUC;
					MessageSend(GetMainMessageHandle(), &msgSend);
				}
				break;
				
			case MSG_REMIND_SOUND_PLAY_START:
				break;
			case MSG_REMIND_SOUND_PLAY_DONE://��ʾ�����Ž���
			case MSG_REMIND_SOUND_PLAY_REQUEST_FAIL:					
				{
					AudioCoreSourceUnmute(APP_SOURCE_NUM, TRUE, TRUE);
				}
				break;
			default:
				LineInPlayRunning(msgRecv.msgId);
				break;
		}
	}
}

static void LineInPlayRunning(uint16_t msgId)
{
	if(LineInPlayCt->state == TaskStateRunning)
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
					MessageSend(GetRemindSoundServiceMessageHandle(), &msgSend);//��ʾ���ڼ�ת����������Ϣ��
				}
#endif
				break;			

#ifdef CFG_FUNC_RECORDER_EN	
			case MSG_REC:
#ifdef	CFG_FUNC_RECORD_SD_UDISK
				if(ResourceValue(AppResourceCard) || ResourceValue(AppResourceUDisk))
				{
					if(LineInPlayCt->RecorderSync == TaskStateNone )
					{
						if(!MediaRecordHeapEnough())
						{
							break;
						}
						MediaRecorderServiceCreate(LineInPlayCt->msgHandle);
						LineInPlayCt->RecorderSync = TaskStateCreating;

					}
					else if(LineInPlayCt->RecorderSync == TaskStateRunning)//�ٰ�¼���� ֹͣ
					{
							MediaRecorderStop();
							MediaRecorderServiceStop();
							LineInPlayCt->RecorderSync = TaskStateStopping;
					}
				}
				else
				{
					APP_DBG("error, no disk!!!\n");
				}
#elif defined(CFG_FUNC_RECORD_FLASHFS)
				if(LineInPlayCt->RecorderSync == TaskStateNone)
				{
					MediaRecorderServiceCreate(LineInPlayCt->msgHandle);
					LineInPlayCt->RecorderSync = TaskStateCreating;
					break;
				} 
				else if(LineInPlayCt->RecorderSync == TaskStateRunning)//�ٰ�¼���� ֹͣ
				{
					MediaRecorderStop();
					MediaRecorderServiceStop();
					LineInPlayCt->RecorderSync = TaskStateStopping;
				}
	//				APP_DBG("Recorder service error.\n");

#endif
				break;
			
			case MSG_MEDIA_RECORDER_SERVICE_CREATED:
#ifdef CFG_FUNC_REMIND_SOUND_EN
				//RemindSound request
				//¼���¼���ʾ�������¼���ļ�Я������ʾ����ʹ��������ʱ
				RemindSoundServiceItemRequest(SOUND_REMIND_LUYIN, FALSE);
				osTaskDelay(350);//����¼������ʾ������ʱ��
#endif
				LineInPlayCt->RecorderSync = TaskStateStarting;
				MediaRecorderServiceStart();
				break;

			case MSG_MEDIA_RECORDER_SERVICE_STARTED:
				MediaRecorderRun();
				LineInPlayCt->RecorderSync = TaskStateRunning;
				break;

			case MSG_MEDIA_RECORDER_STOPPED:
#ifdef CFG_FUNC_REMIND_SOUND_EN
				RemindSoundServiceItemRequest(SOUND_REMIND_STOPREC, FALSE);
#endif				
				MediaRecorderServiceStop();
				LineInPlayCt->RecorderSync = TaskStateStopping;
				break;

			case MSG_MEDIA_RECORDER_ERROR:
				if(LineInPlayCt->RecorderSync == TaskStateRunning)
				{
					MediaRecorderStop();
					MediaRecorderServiceStop();
					LineInPlayCt->RecorderSync = TaskStateStopping;
				}
				break;
				
			case MSG_PLAY_PAUSE:
				APP_DBG("Play Pause\n");
				if(GetMediaRecorderMessageHandle() !=  NULL)
				{
					EncoderServicePause();				
				}
				break;
#endif //¼��

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
bool LineInPlayCreate(MessageHandle parentMsgHandle)
{
	bool		ret = TRUE;

	ret = LineInPlay_Init(parentMsgHandle);
	if(ret)
	{
		LineInPlayCt->taskHandle = NULL;
		xTaskCreate(LineInPlayEntrance,
					"lineInPlay",
					LINEIN_PLAY_TASK_STACK_SIZE,
					NULL, LINEIN_PLAY_TASK_PRIO,
					&LineInPlayCt->taskHandle);
		if(LineInPlayCt->taskHandle == NULL)
		{
			ret = FALSE;
		}
	}
	else
	{
		APP_DBG("lineInPlay app create fail!\n");
	}
	return ret;
}

bool LineInPlayStart(void)
{
	MessageContext		msgSend;

	if(LineInPlayCt == NULL)
	{
		return FALSE;
	}
	msgSend.msgId		= MSG_TASK_START;
	return MessageSend(LineInPlayCt->msgHandle, &msgSend);
}

bool LineInPlayPause(void)
{
	MessageContext		msgSend;
	if(LineInPlayCt == NULL)
	{
		return FALSE;
	}
	msgSend.msgId		= MSG_TASK_PAUSE;
	return MessageSend(LineInPlayCt->msgHandle, &msgSend);
}

bool LineInPlayResume(void)
{
	MessageContext		msgSend;
	if(LineInPlayCt == NULL)
	{
		return FALSE;
	}
	msgSend.msgId		= MSG_TASK_RESUME;
	return MessageSend(LineInPlayCt->msgHandle, &msgSend);
}

bool LineInPlayStop(void)
{
	MessageContext		msgSend;
	if(LineInPlayCt == NULL)
	{
		return FALSE;
	}
	AudioCoreSourceMute(APP_SOURCE_NUM, TRUE, TRUE);
#if (CFG_RES_MIC_SELECT) && defined(CFG_FUNC_AUDIO_EFFECT_EN)
	AudioCoreSourceMute(MIC_SOURCE_NUM, TRUE, TRUE);
#endif	
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	AudioCoreSourceMute(REMIND_SOURCE_NUM, TRUE, TRUE);
#endif
	vTaskDelay(30);

	msgSend.msgId		= MSG_TASK_STOP;
	return MessageSend(LineInPlayCt->msgHandle, &msgSend);
}

bool LineInPlayKill(void)
{
	if(LineInPlayCt == NULL)
	{
		return FALSE;
	}	
	//Kill used services
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	AudioCoreSourceDisable(REMIND_SOURCE_NUM);
	DecoderServiceKill();
#endif
#ifdef CFG_FUNC_RECORDER_EN
	if(LineInPlayCt->RecorderSync != TaskStateNone)//��¼������ʧ��ʱ����Ҫǿ�л���
	{
		MediaRecorderServiceKill();
		LineInPlayCt->RecorderSync = TaskStateNone;
	}
#endif
	//ע�⣺AudioCore�����������mainApp�£��˴�ֻ�ر�AudioCoreͨ�������ر�����
	AudioCoreProcessConfig((void*)AudioNoAppProcess);
	AudioCoreSourceDisable(LINEIN_SOURCE_NUM);

#if (LINEIN_INPUT_CHANNEL == ANA_INPUT_CH_LINEIN3)
	AudioADC_Disable(ADC1_MODULE);
	AudioADC_DeInit(ADC1_MODULE);
#else
	AudioADC_Disable(ADC0_MODULE);
	AudioADC_DeInit(ADC0_MODULE);
#endif

	AudioAnaChannelSet(ANA_INPUT_CH_NONE);
	//AudioADC_DeInit(ADC0_MODULE);

	//task
	if(LineInPlayCt->taskHandle != NULL)
	{
		vTaskDelete(LineInPlayCt->taskHandle);
		LineInPlayCt->taskHandle = NULL;
	}

	//Msgbox
	if(LineInPlayCt->msgHandle != NULL)
	{
		MessageDeregister(LineInPlayCt->msgHandle);
		LineInPlayCt->msgHandle = NULL;
	}

	//PortFree
	LineInPlayCt->AudioCoreLineIn = NULL;
	if(LineInPlayCt->Source1Buf_LineIn != NULL)
	{
		osPortFree(LineInPlayCt->Source1Buf_LineIn);
		LineInPlayCt->Source1Buf_LineIn = NULL;
	}
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	if(LineInPlayCt->SourceDecoder != NULL)
	{
		osPortFree(LineInPlayCt->SourceDecoder);
		LineInPlayCt->SourceDecoder = NULL;
	}
#endif

	if(LineInPlayCt->ADCFIFO != NULL)
	{
		osPortFree(LineInPlayCt->ADCFIFO);
		LineInPlayCt->ADCFIFO = NULL;
	}

	osPortFree(LineInPlayCt);
	LineInPlayCt = NULL;
	APP_DBG("Line:Kill Ct\n");

#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	AudioEffectsDeInit();
#endif

	return TRUE;
}

MessageHandle GetLineInMessageHandle(void)
{
	if(LineInPlayCt != NULL)
	{
		return LineInPlayCt->msgHandle;
	}
	else
	{
		return NULL;
	}
}


#endif//#ifdef CFG_APP_LINEIN_MODE_EN

