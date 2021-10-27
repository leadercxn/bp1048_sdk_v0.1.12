/**
 **************************************************************************************
 * @file    usb_audio_mode.c
 * @brief
 *
 * @author  Owen
 * @version V1.0.0
 *
 * $Created: 2018-04-27 13:06:47$
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
#include "otg_device_hcd.h"
#include "otg_device_audio.h"
#include "otg_device_standard_request.h"
#include "mcu_circular_buf.h"
#include "audio_core_api.h"
#include "audio_core_service.h"
#include "decoder_service.h"
#include "remind_sound_service.h"
#include "main_task.h"
#include "timer.h"
#include "powercontroller.h"
#include "deepsleep.h"
//#include "resampler.h"
#include "backup_interface.h"
#include "breakpoint.h"
#include "ctrlvars.h"
#include "usb_audio_mode.h"
#include "recorder_service.h"
#include "audio_vol.h"
#include "ctrlvars.h"
#include "device_detect.h" 
#include "otg_device_audio.h"
#include "otg_device_stor.h"
#include "resampler_polyphase.h"
#include "audio_adjust.h"
#include "device_service.h"

#ifdef CFG_APP_USB_AUDIO_MODE_EN

#define USB_AUDIO_SRC_BUF_LEN				(150 * 2 * 2)


#define USB_DEVICE_PLAY_TASK_STACK_SIZE		(1024)
#define USB_DEVICE_PLAY_TASK_PRIO			3
#define USB_DEVICE_NUM_MESSAGE_QUEUE		10

uint32_t IsUsbAudioMode = TRUE;
extern UsbAudio UsbAudioSpeaker;
extern UsbAudio UsbAudioMic;
static uint32_t FramCount = 0;


//ResamplerContext UsbAudioMic_Resampler;
typedef struct _UsbDevicePlayContext
{
	xTaskHandle 		taskHandle;
	MessageHandle		msgHandle;
	MessageHandle		parentMsgHandle;
	TaskState			state;
	uint16_t 			*Source1Buf_USBAduio;	//ADC0 取LineIn数据
	uint16_t 			*Sink1Buf_USBAduio;	    //dac0 to pc  
	AudioCoreContext 	*AudioCoreUsb;
#ifdef CFG_FUNC_REMIND_SOUND_EN
	uint16_t*			Source2Decoder;
	TaskState			DecoderSync;
#endif

#ifdef CFG_FUNC_RECORDER_EN
	TaskState			RecorderSync;
#endif
}UsbDevicePlayContext;


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

static  UsbDevicePlayContext*		UsbDevicePlayCt;

static void UsbDevicePlayModeCreating(uint16_t msgId);
static void UsbDevicePlayModeStarting(uint16_t msgId);
static void UsbDevicePlayModeStopping(uint16_t msgId);

//pc->chip 从缓存区获取数据
uint16_t UsbAudioSpeakerDataGet(void *Buffer,uint16_t Len);
//pc->chip 获取缓存区数据长度
uint16_t UsbAudioSpeakerDataLenGet(void);
//chip->pc 保存数据到缓存区
uint16_t UsbAudioMicDataSet(void *Buffer,uint16_t Len);
//chip->pc 数据缓存区剩余空间
uint16_t UsbAudioMicSpaceLenGet(void);

static void UsbDevicePlayRunning(uint16_t msgId);

void UsbDevicePlayResRelease(void)
{
	APP_DBG("UsbDevicePlayResRelease\n");
#ifdef CFG_RES_AUDIO_USB_IN_EN	
#ifdef CFG_RES_AUDIO_USB_SRC_EN
	if(UsbAudioSpeaker.Resampler != NULL)
	{
		APP_DBG("UsbAudioSpeaker.Resampler free\n");
		osPortFree(UsbAudioSpeaker.Resampler);
		UsbAudioSpeaker.Resampler = NULL;
	}
	if(UsbAudioSpeaker.SRCOutBuf != NULL)
	{
		APP_DBG("UsbAudioSpeaker.SRCOutBuf free\n");
		osPortFree(UsbAudioSpeaker.SRCOutBuf);
		UsbAudioSpeaker.SRCOutBuf = NULL;
	}
#endif
	if(UsbAudioSpeaker.PCMBuffer != NULL)
	{
		APP_DBG("UsbAudioSpeaker.PCMBuffer free\n");
		osPortFree(UsbAudioSpeaker.PCMBuffer);
		UsbAudioSpeaker.PCMBuffer = NULL;
	}
	//采样率资源
#ifdef CFG_FUNC_SOFT_ADJUST_IN
	if(UsbAudioSpeaker.SRAFifo != NULL)
	{
		osPortFree(UsbAudioSpeaker.SRAFifo);
		UsbAudioSpeaker.SRAFifo = NULL;
	}
#endif
#endif
#ifdef CFG_RES_AUDIO_USB_OUT_EN	
#ifdef CFG_RES_AUDIO_USB_SRC_EN
	if(UsbAudioMic.Resampler != NULL)
	{
		APP_DBG("UsbAudioMic.Resampler free\n");
		osPortFree(UsbAudioMic.Resampler);
		UsbAudioMic.Resampler = NULL;
	}

	if(UsbAudioMic.SRCOutBuf != NULL)
	{
		APP_DBG("UsbAudioMic.SRCOutBuf free\n");
		osPortFree(UsbAudioMic.SRCOutBuf);
		UsbAudioMic.SRCOutBuf = NULL;
	}
#endif
	if(UsbAudioMic.PCMBuffer != NULL)
	{
		APP_DBG("UsbAudioMic.PCMBuffer free\n");
		osPortFree(UsbAudioMic.PCMBuffer);
		UsbAudioMic.PCMBuffer = NULL;
	}
#endif
#ifdef CFG_FUNC_REMIND_SOUND_EN
	if(UsbDevicePlayCt->Source2Decoder != NULL)
	{
		APP_DBG("Source2Decoder free\n");
		osPortFree(UsbDevicePlayCt->Source2Decoder);
		UsbDevicePlayCt->Source2Decoder = NULL;
	}
#endif
	if(UsbDevicePlayCt->Source1Buf_USBAduio != NULL)
	{
		APP_DBG("Source1Buf_USBAduio free\n");
		osPortFree(UsbDevicePlayCt->Source1Buf_USBAduio);
		UsbDevicePlayCt->Source1Buf_USBAduio = NULL;
	}

	if(UsbDevicePlayCt->Sink1Buf_USBAduio != NULL)
	{
		APP_DBG("Sink1Buf_USBAduio free\n");
		osPortFree(UsbDevicePlayCt->Sink1Buf_USBAduio);
		UsbDevicePlayCt->Sink1Buf_USBAduio = NULL;
	}
#ifdef CFG_FUNC_FREQ_ADJUST
	AudioCoreSourceFreqAdjustDisable();
#endif
}

bool UsbDevicePlayResMalloc(uint16_t SampleLen)
{
	APP_DBG("UsbDevicePlayResMalloc %u\n", SampleLen);
//pc->chip
#ifdef CFG_RES_AUDIO_USB_IN_EN
#ifdef CFG_RES_AUDIO_USB_SRC_EN
	//转采样
	UsbAudioSpeaker.Resampler = (ResamplerPolyphaseContext*)osPortMallocFromEnd(sizeof(ResamplerPolyphaseContext));
	if(UsbAudioSpeaker.Resampler == NULL)
	{
		APP_DBG("Resampler memory error\n");
		return FALSE;
	}
	UsbAudioSpeaker.SRCOutBuf = osPortMallocFromEnd(USB_AUDIO_SRC_BUF_LEN);
	if(UsbAudioSpeaker.SRCOutBuf == NULL)
	{
		APP_DBG("SRCOutBuf memory error\n");
		return FALSE;
	}
#endif  ///end of CFG_RES_AUDIO_USB_SRC_EN
	//Speaker FIFO
	UsbAudioSpeaker.PCMBuffer = osPortMallocFromEnd(SampleLen * 16);
	if(UsbAudioSpeaker.PCMBuffer == NULL)
	{
		APP_DBG("PCMBuffer memory error\n");
		return FALSE;
	}
	memset(UsbAudioSpeaker.PCMBuffer, 0, SampleLen * 16);
	MCUCircular_Config(&UsbAudioSpeaker.CircularBuf, UsbAudioSpeaker.PCMBuffer, SampleLen * 16);

#ifdef CFG_FUNC_SOFT_ADJUST_IN
	//采样率资源申请
	UsbAudioSpeaker.SRAFifo = osPortMallocFromEnd(SampleLen * 16);
	if(UsbAudioSpeaker.SRAFifo == NULL)
	{
		APP_DBG("SRAFifo memory error\n");
		return FALSE;
	}
	memset(UsbAudioSpeaker.SRAFifo, 0, SampleLen * 16);
	MCUCircular_Config(&UsbAudioSpeaker.CircularBufSRA, UsbAudioSpeaker.SRAFifo, SampleLen * 16);
#endif
#endif//end of CFG_RES_USB_IN_EN


#ifdef CFG_RES_AUDIO_USB_OUT_EN
#ifdef CFG_RES_AUDIO_USB_SRC_EN
	//转采样
	UsbAudioMic.Resampler = (ResamplerPolyphaseContext*)osPortMallocFromEnd(sizeof(ResamplerPolyphaseContext));
	if(UsbAudioMic.Resampler == NULL)
	{
		APP_DBG("UsbAudioMic.Resampler memory error\n");
		return FALSE;
	}
	UsbAudioMic.SRCOutBuf = osPortMallocFromEnd(USB_AUDIO_SRC_BUF_LEN);
	if(UsbAudioMic.SRCOutBuf == NULL)
	{
		APP_DBG("UsbAudioMic.SRCOutBuf memory error\n");
		return FALSE;
	}
#endif //end of #ifdef CFG_RES_AUDIO_USB_SRC_EN

	//MIC FIFO
	UsbAudioMic.PCMBuffer = osPortMallocFromEnd(SampleLen * 16);
	if(UsbAudioMic.PCMBuffer == NULL)
	{
		APP_DBG("UsbAudioMic.PCMBuffer memory error\n");
		return FALSE;
	}
	memset(UsbAudioMic.PCMBuffer, 0, SampleLen * 16);
	MCUCircular_Config(&UsbAudioMic.CircularBuf, UsbAudioMic.PCMBuffer, SampleLen * 16);
#endif///end of CFG_REGS_AUDIO_USB_OUT_EN

#ifdef CFG_FUNC_REMIND_SOUND_EN
	UsbDevicePlayCt->Source2Decoder = (uint16_t*)osPortMallocFromEnd(SampleLen * 2 * 2);//One Frame
	if(UsbDevicePlayCt->Source2Decoder == NULL)
	{
		return FALSE;
	}
	memset(UsbDevicePlayCt->Source2Decoder, 0, SampleLen * 2 * 2);//2K
#endif

	//////////////audio core///////////////
	UsbDevicePlayCt->Source1Buf_USBAduio = (uint16_t*)osPortMallocFromEnd(SampleLen * 8);//stereo one Frame
	if(UsbDevicePlayCt->Source1Buf_USBAduio == NULL)
	{
		return FALSE;
	}
	memset(UsbDevicePlayCt->Source1Buf_USBAduio, 0, SampleLen * 8);

	UsbDevicePlayCt->Sink1Buf_USBAduio = (uint16_t*)osPortMallocFromEnd(SampleLen * 8);//stereo one Frame
	if(UsbDevicePlayCt->Sink1Buf_USBAduio == NULL)
	{
		return FALSE;
	}
	memset(UsbDevicePlayCt->Sink1Buf_USBAduio, 0, SampleLen * 8);

	return TRUE;
}

void UsbDevicePlayResInit(void)
{
	APP_DBG("UsbDevicePlayResInit\n");
	if(UsbDevicePlayCt->Source1Buf_USBAduio != NULL)
	{
		UsbDevicePlayCt->AudioCoreUsb->AudioSource[USB_AUDIO_SOURCE_NUM].PcmInBuf = (int16_t *)UsbDevicePlayCt->Source1Buf_USBAduio;
	}

	if(UsbDevicePlayCt->Sink1Buf_USBAduio != NULL)
	{
		UsbDevicePlayCt->AudioCoreUsb->AudioSink[USB_AUDIO_SINK_NUM].PcmOutBuf = (int16_t *)UsbDevicePlayCt->Sink1Buf_USBAduio;
	}

#ifdef CFG_FUNC_REMIND_SOUND_EN
	if(UsbDevicePlayCt->Source2Decoder != NULL)
	{
		UsbDevicePlayCt->AudioCoreUsb->AudioSource[REMIND_SOURCE_NUM].PcmInBuf = (int16_t *)UsbDevicePlayCt->Source2Decoder;
	}
#endif

#ifdef CFG_FUNC_FREQ_ADJUST
#ifndef CFG_FUNC_SOFT_ADJUST_IN
	AudioCoreSourceFreqAdjustEnable(USB_AUDIO_SOURCE_NUM, (mainAppCt.SamplesPreFrame * 2 * 2 * 2 * 2 / 4 - mainAppCt.SamplesPreFrame) / 4,
										(mainAppCt.SamplesPreFrame * 2 * 2 * 2 * 2 / 4 - mainAppCt.SamplesPreFrame) * 3 / 4);
#else
	AudioCoreSourceFreqAdjustEnable(USB_AUDIO_SOURCE_NUM, mainAppCt.SamplesPreFrame, mainAppCt.SamplesPreFrame + 64);
#endif
#ifdef CFG_FUNC_SOFT_ADJUST_OUT_USBAUDIO
	AudioCoreSinkSRAInit();
#endif
#endif

	//转采样初始化
#ifdef CFG_RES_AUDIO_USB_IN_EN
    #ifdef CFG_RES_AUDIO_USB_SRC_EN
	if(UsbAudioSpeaker.AudioSampleRate != CFG_PARA_SAMPLE_RATE)
	{
#if (CFG_PARA_SAMPLE_RATE == 44100)
		resampler_polyphase_init(UsbAudioSpeaker.Resampler, 2, RESAMPLER_POLYPHASE_SRC_RATIO_147_160);
#elif (CFG_PARA_SAMPLE_RATE == 48000)
		resampler_polyphase_init(UsbAudioSpeaker.Resampler, 2, RESAMPLER_POLYPHASE_SRC_RATIO_160_147);
#endif
		UsbAudioSpeaker.AudioSampleRateFlag = TRUE;
	}
	#endif
#endif

	//转采样初始化
#ifdef CFG_RES_AUDIO_USB_OUT_EN
	#ifdef CFG_RES_AUDIO_USB_SRC_EN
#if (CFG_PARA_SAMPLE_RATE == 44100)
	resampler_polyphase_init(UsbAudioMic.Resampler, 2, RESAMPLER_POLYPHASE_SRC_RATIO_160_147);
#elif (CFG_PARA_SAMPLE_RATE == 48000)
	resampler_polyphase_init(UsbAudioMic.Resampler, 2, RESAMPLER_POLYPHASE_SRC_RATIO_147_160);
#endif
	#endif
#endif
}

//USB声卡模式参数配置，资源初始化
static bool UsbDevicePlay_Init(MessageHandle parentMsgHandle)
{
	memset(&UsbAudioSpeaker,0,sizeof(UsbAudioSpeaker));
	memset(&UsbAudioMic,0,sizeof(UsbAudioMic));

	UsbAudioSpeaker.Channel    = 2;
	UsbAudioSpeaker.LeftVol    = AUDIO_MAX_VOLUME;
	UsbAudioSpeaker.RightVol   = AUDIO_MAX_VOLUME;

	UsbAudioMic.Channel        = 2;
	UsbAudioMic.LeftVol        = AUDIO_MAX_VOLUME;
	UsbAudioMic.RightVol       = AUDIO_MAX_VOLUME;

//System config
	DMA_ChannelAllocTableSet((uint8_t *)DmaChannelMap);

//Task & App Config
	UsbDevicePlayCt = (UsbDevicePlayContext*)osPortMalloc(sizeof(UsbDevicePlayContext));
	memset(UsbDevicePlayCt, 0, sizeof(UsbDevicePlayContext));
	UsbDevicePlayCt->msgHandle = MessageRegister(USB_DEVICE_NUM_MESSAGE_QUEUE);
	if(UsbDevicePlayCt->msgHandle == NULL)
	{
		return FALSE;
	}
	UsbDevicePlayCt->parentMsgHandle = parentMsgHandle;
	UsbDevicePlayCt->state = TaskStateCreating;

#ifdef CFG_FUNC_SOFT_ADJUST_IN
	UsbAudioSpeaker.pBufTemp = osPortMalloc(128 * 4);
	if(UsbAudioSpeaker.pBufTemp == NULL)
	{
		APP_DBG("UsbAudioSpeaker.pBufTemp memory error\n");
		return FALSE;
	}
	memset(UsbAudioSpeaker.pBufTemp, 0, 128 * 4);
#endif

	if(!UsbDevicePlayResMalloc(mainAppCt.SamplesPreFrame))
	{
		APP_DBG("UsbDevicePlayResMalloc Res Error!\n");
		return FALSE;
	}

	//Core Source1 para
	UsbDevicePlayCt->AudioCoreUsb = (AudioCoreContext*)&AudioCore;


	UsbDevicePlayCt->AudioCoreUsb->AudioSource[USB_AUDIO_SOURCE_NUM].Enable = FALSE;//一开始为FALSE,枚举完成之后再置位,防止破音
	UsbDevicePlayCt->AudioCoreUsb->AudioSource[USB_AUDIO_SOURCE_NUM].FuncDataGet = UsbAudioSpeakerDataGet;
	UsbDevicePlayCt->AudioCoreUsb->AudioSource[USB_AUDIO_SOURCE_NUM].FuncDataGetLen = UsbAudioSpeakerDataLenGet;
	UsbDevicePlayCt->AudioCoreUsb->AudioSource[USB_AUDIO_SOURCE_NUM].IsSreamData = FALSE;
	UsbDevicePlayCt->AudioCoreUsb->AudioSource[USB_AUDIO_SOURCE_NUM].PcmFormat = 2;
	UsbDevicePlayCt->AudioCoreUsb->AudioSource[USB_AUDIO_SOURCE_NUM].PcmInBuf = (int16_t *)UsbDevicePlayCt->Source1Buf_USBAduio;
	UsbDevicePlayCt->AudioCoreUsb->AudioSource[USB_AUDIO_SOURCE_NUM].LeftMuteFlag = 0;
	UsbDevicePlayCt->AudioCoreUsb->AudioSource[USB_AUDIO_SOURCE_NUM].RightMuteFlag = 0;
	//UsbDevicePlayCt->AudioCoreUsb->AudioSource[USB_AUDIO_SOURCE_NUM].LeftVol = 4095;
	//UsbDevicePlayCt->AudioCoreUsb->AudioSource[USB_AUDIO_SOURCE_NUM].RightVol = 4095;


	UsbDevicePlayCt->AudioCoreUsb->AudioSink[USB_AUDIO_SINK_NUM].Enable = FALSE;
	UsbDevicePlayCt->AudioCoreUsb->AudioSink[USB_AUDIO_SINK_NUM].FuncDataSet = UsbAudioMicDataSet;
	UsbDevicePlayCt->AudioCoreUsb->AudioSink[USB_AUDIO_SINK_NUM].FuncDataSpaceLenGet = UsbAudioMicSpaceLenGet;
	UsbDevicePlayCt->AudioCoreUsb->AudioSink[USB_AUDIO_SINK_NUM].PcmOutBuf = (int16_t *)UsbDevicePlayCt->Sink1Buf_USBAduio;


#ifdef CFG_FUNC_REMIND_SOUND_EN
	//Core Soure2 Para
	DecoderSourceNumSet(REMIND_SOURCE_NUM);
	UsbDevicePlayCt->AudioCoreUsb->AudioSource[REMIND_SOURCE_NUM].Enable = FALSE;
	UsbDevicePlayCt->AudioCoreUsb->AudioSource[REMIND_SOURCE_NUM].FuncDataGet = DecodedPcmDataGet;
	UsbDevicePlayCt->AudioCoreUsb->AudioSource[REMIND_SOURCE_NUM].FuncDataGetLen = NULL;
	UsbDevicePlayCt->AudioCoreUsb->AudioSource[REMIND_SOURCE_NUM].IsSreamData = FALSE;//Decoder
	UsbDevicePlayCt->AudioCoreUsb->AudioSource[REMIND_SOURCE_NUM].PcmFormat = 2; //stereo
	UsbDevicePlayCt->AudioCoreUsb->AudioSource[REMIND_SOURCE_NUM].PcmInBuf = (int16_t *)UsbDevicePlayCt->Source2Decoder;
#endif

	//Core Process
#ifdef CFG_FUNC_AUDIO_EFFECT_EN
    #ifdef CFG_FUNC_MIC_KARAOKE_EN
	UsbDevicePlayCt->AudioCoreUsb->AudioEffectProcess = (AudioCoreProcessFunc)AudioEffectProcess;
	#else
	UsbDevicePlayCt->AudioCoreUsb->AudioEffectProcess = (AudioCoreProcessFunc)AudioMusicProcess;
	#endif
#else
	UsbDevicePlayCt->AudioCoreUsb->AudioEffectProcess = (AudioCoreProcessFunc)AudioBypassProcess;
#endif
#ifdef CFG_FUNC_RECORDER_EN
	UsbDevicePlayCt->RecorderSync = TaskStateNone;
#endif

#ifdef CFG_FUNC_SOFT_ADJUST_IN
	AudioCoreSourceSRAResMalloc();
#ifdef CFG_FUNC_SOFT_ADJUST_OUT_USBAUDIO
	AudioCoreSinkSRAResMalloc();
#endif
	SoftFlagRegister(SoftFlagBtSra);
#endif
	return TRUE;
}

//创建从属services
static void UsbDevicePlayModeCreate(void)
{
	bool NoService = TRUE;
	// Create service task
#if defined(CFG_FUNC_REMIND_SBC)
	DecoderServiceCreate(UsbDevicePlayCt->msgHandle, DECODER_BUF_SIZE_SBC, DECODER_FIFO_SIZE_FOR_SBC);//提示音格式决定解码器内存消耗
	NoService = FALSE;
#elif defined(CFG_FUNC_REMIND_SOUND_EN)
	DecoderServiceCreate(UsbDevicePlayCt->msgHandle, DECODER_BUF_SIZE_MP3, DECODER_FIFO_SIZE_FOR_MP3);
	NoService = FALSE;
#endif
	if(NoService)
	{
		UsbDevicePlayModeCreating(MSG_NONE);
	}
}

//All of services is created, Send CREATED message to parent
static void UsbDevicePlayModeCreating(uint16_t msgId)
{
	MessageContext		msgSend;
#ifdef CFG_FUNC_REMIND_SOUND_EN
	if(msgId == MSG_DECODER_SERVICE_CREATED)
	{
		UsbDevicePlayCt->DecoderSync = TaskStateReady;
	}
	if(UsbDevicePlayCt->DecoderSync == TaskStateReady)
#endif
	{
		msgSend.msgId		= MSG_USB_DEVICE_MODE_CREATED;
		MessageSend(UsbDevicePlayCt->parentMsgHandle, &msgSend);
		UsbDevicePlayCt->state = TaskStateReady;
	}
}

static void UsbDevicePlayModeStart(void)
{
	bool NoService = TRUE;
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	DecoderServiceStart();
	UsbDevicePlayCt->DecoderSync = TaskStateStarting;
	NoService = FALSE;
#endif
	if(NoService)
	{
		UsbDevicePlayModeStarting(MSG_NONE);
	}
	else
	{
		UsbDevicePlayCt->state = TaskStateStarting;
	}
}

static void UsbDevicePlayModeStarting(uint16_t msgId)
{
	MessageContext		msgSend;

	APP_DBG("UsbDevicePlayModeStarting\n");

#ifdef CFG_FUNC_REMIND_SOUND_EN
	if(msgId == MSG_DECODER_SERVICE_STARTED)
	{
		UsbDevicePlayCt->DecoderSync = TaskStateRunning;
	}
	if(UsbDevicePlayCt->DecoderSync == TaskStateRunning)
#endif
	{
		msgSend.msgId		= MSG_USB_DEVICE_MODE_STARTED;
		MessageSend(UsbDevicePlayCt->parentMsgHandle, &msgSend);
		UsbDevicePlayCt->state = TaskStateRunning;

#ifdef CFG_FUNC_REMIND_SOUND_EN
		if(!SoftFlagGet(SoftFlagRemindMask))
		{
			if(!RemindSoundServiceItemRequest(SOUND_REMIND_SHENGKAM, TRUE)) //插播提示音:模式启动
			{
				AudioCoreSourceUnmute(APP_SOURCE_NUM, TRUE, TRUE);
				APP_DBG("usb device Unmute\n");
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
		}
#else
#ifdef CFG_FUNC_FREQ_ADJUST
#ifndef CFG_FUNC_SOFT_ADJUST_IN
		AudioCoreSourceFreqAdjustEnable(USB_AUDIO_SOURCE_NUM, (mainAppCt.SamplesPreFrame * 2 * 2 * 2 * 2 / 4 - mainAppCt.SamplesPreFrame) / 4,
										(mainAppCt.SamplesPreFrame * 2 * 2 * 2 * 2 / 4 - mainAppCt.SamplesPreFrame) * 3 / 4);
#else
		AudioCoreSourceFreqAdjustEnable(USB_AUDIO_SOURCE_NUM, (mainAppCt.SamplesPreFrame, mainAppCt.SamplesPreFrame + 64);
#endif
#ifdef CFG_FUNC_SOFT_ADJUST_OUT_USBAUDIO
		AudioCoreSinkSRAInit();
#endif
#endif
#endif
	}
}

static void UsbDevicePlayModeStop(void)
{
	bool NoService = TRUE;
	
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	DecoderServiceStop();
	NoService = FALSE;
#endif
#ifdef CFG_FUNC_RECORDER_EN
	if(UsbDevicePlayCt->RecorderSync != TaskStateNone)
	{//此service 随用随Kill
		MediaRecorderServiceStop();
		UsbDevicePlayCt->RecorderSync = TaskStateStopping;
		NoService = FALSE;
	}
#endif
	UsbDevicePlayCt->state = TaskStateStopping;
	if(NoService)
	{
		UsbDevicePlayModeStopping(MSG_NONE);
	}
}

static void UsbDevicePlayModeStopping(uint16_t msgId)
{
	MessageContext		msgSend;
	//Set para
#ifdef CFG_FUNC_RECORDER_EN
	if(msgId == MSG_MEDIA_RECORDER_SERVICE_STOPPED)
	{
		UsbDevicePlayCt->RecorderSync = TaskStateNone;
		APP_DBG("UsbDevice:RecorderKill");
		MediaRecorderServiceKill();
	}
#endif
#ifdef CFG_FUNC_REMIND_SOUND_EN
	if(msgId == MSG_DECODER_SERVICE_STOPPED)
	{
		UsbDevicePlayCt->DecoderSync = TaskStateNone;
	}
#endif

	if((UsbDevicePlayCt->state == TaskStateStopping)
#ifdef CFG_FUNC_RECORDER_EN
		&& (UsbDevicePlayCt->RecorderSync == TaskStateNone)
#endif
#ifdef CFG_FUNC_REMIND_SOUND_EN
		&& (UsbDevicePlayCt->DecoderSync == TaskStateNone)
#endif
		)
	{
		NVIC_DisableIRQ(Usb_IRQn);
		OTG_DeviceDisConnect();
		IsUsbAudioMode = FALSE;

		//clear msg
		MessageClear(UsbDevicePlayCt->msgHandle);

		//Set state
		UsbDevicePlayCt->state = TaskStateStopped;

		//reply
		msgSend.msgId = MSG_USB_DEVICE_MODE_STOPPED;
		MessageSend(UsbDevicePlayCt->parentMsgHandle, &msgSend);
		vTaskDelay(0xFFFFFFFF);//wait task killed
	}
}

static void UsbDevicePlayEntrance(void * param)
{
	uint32_t mode = 0;
	//uint32_t sd_link = 0;
	MessageContext		msgRecv;

//#if (defined(CFG_APP_USB_AUDIO_MODE_EN)) && (defined(CFG_COMMUNICATION_BY_USB))
//	SetDeviceInitState(TRUE);
//#endif

	APP_DBG("UsbDevice:App\n");
	UsbDevicePlayModeCreate();
	
#ifdef CFG_FUNC_AUDIO_EFFECT_EN	
	AudioEffectModeSel(mainAppCt.EffectMode, 2);//0=init hw,1=effect,2=hw+effect
#ifdef CFG_COMMUNICATION_BY_UART
	UART1_Communication_Init((void *)(&UartRxBuf[0]), 1024, (void *)(&UartTxBuf[0]), 1024);
#endif
#endif

	IsUsbAudioMode = TRUE;
	OTG_DeviceModeSel(CFG_PARA_USB_MODE, USB_VID, USBPID(CFG_PARA_USB_MODE));//
	mode = CFG_PARA_USB_MODE;
	if((mode == READER) || (mode == AUDIO_READER) || (mode == MIC_READER) || (mode == AUDIO_MIC_READER))
	{
		if(ResourceValue(AppResourceCard))
		{
			CardPortInit(CFG_RES_CARD_GPIO);
			if(SDCard_Init() == NONE_ERR)
			{
				APP_DBG("SD INIT OK\n");
				//sd_link = 1;
			}
		}
#if 0
		if(sd_link == 0)
		{
			if(mode == READER)
			{
				APP_DBG("mode error\n");
			}
			else if(mode == AUDIO_READER)
			{
				OTG_DeviceModeSel(AUDIO_ONLY,0x0000,0x1234);
			}
			else if(mode == MIC_READER)
			{
				OTG_DeviceModeSel(MIC_ONLY,0x0000,0x1234);
			}
			else if(mode == AUDIO_MIC_READER)
			{
				OTG_DeviceModeSel(AUDIO_MIC,0x0000,0x1234);
			}
		}
#endif
	}
	OTG_DeviceStorInit();
	OTG_DeviceInit();
	NVIC_EnableIRQ(Usb_IRQn);

#ifdef CFG_FUNC_BREAKPOINT_EN
	BackupInfoUpdata(BACKUP_SYS_INFO);
#endif

#if (CFG_RES_MIC_SELECT) && defined(CFG_FUNC_AUDIO_EFFECT_EN)
	AudioCoreSourceUnmute(MIC_SOURCE_NUM, TRUE, TRUE);
#endif
#ifdef CFG_COMMUNICATION_BY_USB
    sDevice_Inserted_Flag = 1;
#endif
	while(1)
	{
		MessageRecv(UsbDevicePlayCt->msgHandle, &msgRecv, 10);
		
		switch(msgRecv.msgId)
		{
			case MSG_DECODER_SERVICE_CREATED:
				UsbDevicePlayModeCreating(msgRecv.msgId);
				break;

			case MSG_TASK_START:
				APP_DBG("TASK_START\n");
				UsbDevicePlayModeStart();
				break;

			case MSG_DECODER_SERVICE_STARTED:
				UsbDevicePlayModeStarting(msgRecv.msgId);
#ifdef CFG_FUNC_FREQ_ADJUST	//此处需要加声卡数据确认
#ifndef CFG_FUNC_SOFT_ADJUST_IN
				AudioCoreSourceFreqAdjustEnable(USB_AUDIO_SOURCE_NUM, (mainAppCt.SamplesPreFrame * 2 * 2 * 2 * 2 / 4 - mainAppCt.SamplesPreFrame) / 4,
													(mainAppCt.SamplesPreFrame * 2 * 2 * 2 * 2 / 4 - mainAppCt.SamplesPreFrame) * 3 / 4);
#else
				AudioCoreSourceFreqAdjustEnable(USB_AUDIO_SOURCE_NUM, mainAppCt.SamplesPreFrame, mainAppCt.SamplesPreFrame + 64);
#endif
#ifdef CFG_FUNC_SOFT_ADJUST_OUT_USBAUDIO
				AudioCoreSinkSRAInit();
#endif
#endif
				break;

			case MSG_TASK_STOP:
#ifdef CFG_FUNC_REMIND_SOUND_EN
				RemindSoundServiceReset();
#endif
				UsbDevicePlayModeStop();
				break;

			case MSG_MEDIA_RECORDER_SERVICE_STOPPED:
			case MSG_DECODER_SERVICE_STOPPED:
				UsbDevicePlayModeStopping(msgRecv.msgId);
				break;
			
			case MSG_APP_RES_RELEASE:
				UsbDevicePlayResRelease();
				UsbAudioSpeaker.AudioSampleRateFlag = FALSE;
				{
					MessageContext		msgSend;
					msgSend.msgId		= MSG_APP_RES_RELEASE_SUC;
					MessageSend(GetMainMessageHandle(), &msgSend);
				}
				break;
			case MSG_APP_RES_MALLOC:
				UsbDevicePlayResMalloc(mainAppCt.SamplesPreFrame);
				{
					MessageContext		msgSend;
					msgSend.msgId		= MSG_APP_RES_MALLOC_SUC;
					MessageSend(GetMainMessageHandle(), &msgSend);
				}
				break;
			case MSG_APP_RES_INIT:
				UsbDevicePlayResInit();
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
			case MSG_REMIND_SOUND_PLAY_REQUEST_FAIL:
				APP_DBG("MSG_REMIND_SOUND_PLAY_DONE: \n");
				AudioCoreSourceUnmute(APP_SOURCE_NUM, TRUE, TRUE);
				break;
#if 1
			case MSG_DEVICE_SERVICE_CARD_IN:
				CardPortInit(CFG_RES_CARD_GPIO);
				if(SDCard_Init() == 0)
				{
					APP_DBG("SD INIT OK\n");
					//OTG_DeviceStorInit();
				} else {
					APP_DBG("SD INIT FAILED\n");
				}
				break;
#endif
			default:
				if(UsbDevicePlayCt->state == TaskStateRunning)
				{
					UsbDevicePlayRunning(msgRecv.msgId);
				}
				break;
		}
		OTG_DeviceRequestProcess();
		OTG_DeviceStorProcess();
	}
}

void PCAudioPP(void);
void PCAudioNext(void);
void PCAudioPrev(void);
void PCAudioStop(void);
static void UsbDevicePlayRunning(uint16_t msgId)
{
	switch(msgId)
	{
#ifdef	CFG_FUNC_POWERKEY_EN
		case MSG_TASK_POWERDOWN:
			APP_DBG("PowerDown MSG, Please breakpoint\n");
			SystemPowerDown();
			break;
#endif
		case MSG_DECODER_STOPPED:
#ifdef CFG_FUNC_REMIND_SOUND_EN
		{
			MessageContext		msgSend;
			msgSend.msgId = msgId;
			MessageSend(GetRemindSoundServiceMessageHandle(), &msgSend);//提示音期间转发解码器消息
		}
#endif
			break;

#ifdef CFG_FUNC_RECORDER_EN
		case MSG_REC:
			if(ResourceValue(AppResourceCard) || ResourceValue(AppResourceUDisk))
			{
				if(UsbDevicePlayCt->RecorderSync == TaskStateNone)
				{
					if(!MediaRecordHeapEnough())
					{
						break;
					}
					MediaRecorderServiceCreate(UsbDevicePlayCt->msgHandle);
					UsbDevicePlayCt->RecorderSync = TaskStateCreating;
				}
				else if(UsbDevicePlayCt->RecorderSync == TaskStateRunning)//再按录音键停止
				{
					MediaRecorderStop();
					MediaRecorderServiceStop();
					UsbDevicePlayCt->RecorderSync = TaskStateStopping;
				}
			}
			else
			{//flashfs录音 待处理
				APP_DBG("Usbdevice:error, no disk!!!\n");
			}
			break;

		case MSG_MEDIA_RECORDER_SERVICE_CREATED:
			UsbDevicePlayCt->RecorderSync = TaskStateStarting;
			MediaRecorderServiceStart();
			break;

		case MSG_MEDIA_RECORDER_SERVICE_STARTED:
			MediaRecorderRun();
			UsbDevicePlayCt->RecorderSync = TaskStateRunning;
			break;

		case MSG_MEDIA_RECORDER_STOPPED:
			MediaRecorderServiceStop();
			UsbDevicePlayCt->RecorderSync = TaskStateStopping;
			break;

		case MSG_MEDIA_RECORDER_ERROR:
			if(UsbDevicePlayCt->RecorderSync == TaskStateRunning)
			{
				MediaRecorderStop();
				MediaRecorderServiceStop();
				UsbDevicePlayCt->RecorderSync = TaskStateStopping;
			}
			else
			{
				PCAudioStop();
			}
			break;

		case MSG_PLAY_PAUSE:
			APP_DBG("Play Pause\n");
#ifdef CFG_FUNC_RECORDER_EN	
			if(GetMediaRecorderMessageHandle() !=  NULL)
			{
				EncoderServicePause();
				break;
			}
#endif						
			PCAudioPP();
			break;

		case MSG_PRE:
			APP_DBG("PRE Song\n");
			PCAudioPrev();
			break;

		case MSG_NEXT:
			APP_DBG("next Song\n");
			PCAudioNext();
			break;
#endif //录音

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
bool UsbDevicePlayCreate(MessageHandle parentMsgHandle)
{
	bool	ret;

	ret = UsbDevicePlay_Init(parentMsgHandle);
	if(ret)
	{
		UsbDevicePlayCt->taskHandle = NULL;
		xTaskCreate(UsbDevicePlayEntrance,
					"UsbDevicePlay",
					USB_DEVICE_PLAY_TASK_STACK_SIZE,
					NULL,
					USB_DEVICE_PLAY_TASK_PRIO,
					&UsbDevicePlayCt->taskHandle);
		if(UsbDevicePlayCt->taskHandle == NULL)
		{
			ret = FALSE;
		}
	}
	if(!ret)
	{
		APP_DBG("create fail!\n");
	}
	return ret;
}

bool UsbDevicePlayStart(void)
{
	MessageContext		msgSend;
	if(UsbDevicePlayCt == NULL)
	{
		return FALSE;
	}
	APP_DBG("PlayStart\n");
	msgSend.msgId		= MSG_TASK_START;
	return MessageSend(UsbDevicePlayCt->msgHandle, &msgSend);
}

bool UsbDevicePlayPause(void)
{
	MessageContext		msgSend;
	if(UsbDevicePlayCt == NULL)
	{
		return FALSE;
	}
	msgSend.msgId		= MSG_TASK_PAUSE;
	return MessageSend(UsbDevicePlayCt->msgHandle, &msgSend);
}

bool UsbDevicePlayResume(void)
{
	MessageContext		msgSend;

	msgSend.msgId		= MSG_TASK_RESUME;
	MessageSend(UsbDevicePlayCt->msgHandle, &msgSend);

	return 0;
}

bool UsbDevicePlayStop(void)
{
	MessageContext		msgSend;
	if(UsbDevicePlayCt == NULL)
	{
		return FALSE;
	}
	AudioCoreSourceMute(APP_SOURCE_NUM, TRUE, TRUE);
#if (CFG_RES_MIC_SELECT) && defined(CFG_FUNC_AUDIO_EFFECT_EN)
	AudioCoreSourceMute(MIC_SOURCE_NUM, TRUE, TRUE);
#endif
#ifdef CFG_FUNC_REMIND_SOUND_EN
	AudioCoreSourceMute(REMIND_SOURCE_NUM, TRUE, TRUE);
#endif
	vTaskDelay(30);

	msgSend.msgId		= MSG_TASK_STOP;
	return MessageSend(UsbDevicePlayCt->msgHandle, &msgSend);
}

bool UsbDevicePlayKill(void)
{
	if(UsbDevicePlayCt == NULL)
	{
		return FALSE;
	}
	//Kill used services
#ifdef CFG_FUNC_REMIND_SOUND_EN
	AudioCoreSourceDisable(REMIND_SOURCE_NUM);
	DecoderServiceKill();
#endif
#ifdef CFG_FUNC_RECORDER_EN
	if(UsbDevicePlayCt->RecorderSync != TaskStateNone)//当录音创建失败时，需要强行回收
	{
		MediaRecorderServiceKill();
		UsbDevicePlayCt->RecorderSync = TaskStateNone;
	}
#endif
#ifdef CFG_FUNC_FREQ_ADJUST
	AudioCoreSourceFreqAdjustDisable();
#ifdef CFG_FUNC_SOFT_ADJUST_IN
	SoftFlagDeregister(SoftFlagBtSra);
	AudioCoreSourceSRAResRelease();
#ifdef CFG_FUNC_SOFT_ADJUST_OUT_USBAUDIO
	AudioCoreSinkSRAResRelease();
#endif
#endif
#endif

	//注意：AudioCore父任务调整到mainApp下，此处只关闭AudioCore通道，不关闭任务
	AudioCoreProcessConfig((void*)AudioNoAppProcess);
	AudioCoreSourceDisable(USB_AUDIO_SOURCE_NUM);

#ifdef CFG_RES_AUDIO_USB_OUT_EN
	AudioCoreSinkDisable(USB_AUDIO_SINK_NUM);
#endif

	OTG_DeviceDisConnect();
	NVIC_DisableIRQ(Usb_IRQn);
	vTaskDelay(2);

	//task
	if(UsbDevicePlayCt->taskHandle != NULL)
	{
		vTaskDelete(UsbDevicePlayCt->taskHandle);
		UsbDevicePlayCt->taskHandle = NULL;
	}

	//Msgbox
	if(UsbDevicePlayCt->msgHandle != NULL)
	{
		MessageDeregister(UsbDevicePlayCt->msgHandle);
		UsbDevicePlayCt->msgHandle = NULL;
	}

	UsbDevicePlayResRelease();

#ifdef CFG_FUNC_SOFT_ADJUST_IN
	if(UsbAudioSpeaker.pBufTemp != NULL)
	{
		osPortFree(UsbAudioSpeaker.pBufTemp);
		UsbAudioSpeaker.pBufTemp = NULL;
	}
#endif

	osPortFree(UsbDevicePlayCt);
	UsbDevicePlayCt = NULL;

#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	AudioEffectsDeInit();
#endif

#if (defined(CFG_APP_USB_AUDIO_MODE_EN)) && (defined(CFG_COMMUNICATION_BY_USB))
	SetUSBAudioExitFlag(TRUE);
#endif

	return TRUE;
}

MessageHandle GetUsbDeviceMessageHandle(void)
{
	if(UsbDevicePlayCt == NULL)
	{
		return NULL;
	}
	return UsbDevicePlayCt->msgHandle;
}

void UsbAudioMicSampleRateChange(uint32_t SampleRate)
{
#ifdef CFG_RES_AUDIO_USB_OUT_EN
	if(SampleRate == 48000)
	{
#if CFG_PARA_SAMPLE_RATE == 44100
		resampler_polyphase_init(UsbAudioMic.Resampler, 2, RESAMPLER_POLYPHASE_SRC_RATIO_160_147);
#endif
	}
	else if(SampleRate == 44100)
	{
#if CFG_PARA_SAMPLE_RATE == 48000
		resampler_polyphase_init(UsbAudioMic.Resampler, 2, RESAMPLER_POLYPHASE_SRC_RATIO_147_160);
#endif
	}
#endif
	MCUCircular_Config(&UsbAudioMic.CircularBuf, UsbAudioMic.PCMBuffer, mainAppCt.SamplesPreFrame*16);
}

void UsbAudioSpeakerSampleRateChange(uint32_t SampleRate)
{
#ifdef CFG_RES_AUDIO_USB_IN_EN
	if(SampleRate != CFG_PARA_SAMPLE_RATE)
	{
#if CFG_PARA_SAMPLE_RATE == 44100
		resampler_polyphase_init(UsbAudioSpeaker.Resampler, 2, RESAMPLER_POLYPHASE_SRC_RATIO_147_160);
#elif CFG_PARA_SAMPLE_RATE == 48000
		resampler_polyphase_init(UsbAudioSpeaker.Resampler, 2, RESAMPLER_POLYPHASE_SRC_RATIO_160_147);
#endif
		UsbAudioSpeaker.AudioSampleRateFlag = TRUE;
	}
#endif
	MCUCircular_Config(&UsbAudioSpeaker.CircularBuf, UsbAudioSpeaker.PCMBuffer, mainAppCt.SamplesPreFrame*16);
}

//////////////////////////////////////////////////audio core api/////////////////////////////////////////////////////
//pc->chip 从缓存区获取数据
uint16_t UsbAudioSpeakerDataGet(void *Buffer, uint16_t Len)
{
	uint16_t Length = 0;

#ifdef CFG_FUNC_SOFT_ADJUST_IN //软件微调
	if(!UsbAudioSpeaker.SRAFifo)
	{
		return 0;
	}
	Length = Len * 4;
	Length = MCUCircular_GetData(&UsbAudioSpeaker.CircularBufSRA, Buffer, Length);
#else
	if(!UsbAudioSpeaker.PCMBuffer)
	{
		return 0;
	}
	Length = Len * 4;
	Length = MCUCircular_GetData(&UsbAudioSpeaker.CircularBuf, Buffer, Length);
#endif
	return Length / 4;
}

//pc->chip 获取缓存区数据长度
uint16_t UsbAudioSpeakerDataLenGet(void)
{
	uint16_t Len;
#ifdef CFG_FUNC_SOFT_ADJUST_IN //软件微调
	if(!UsbAudioSpeaker.SRAFifo)
	{
		return 0;
	}
	Len = MCUCircular_GetDataLen(&UsbAudioSpeaker.CircularBufSRA) / 4 + MCUCircular_GetDataLen(&UsbAudioSpeaker.CircularBuf) / 4;
#else
	if(!UsbAudioSpeaker.PCMBuffer)
	{
		return 0;
	}
	Len = MCUCircular_GetDataLen(&UsbAudioSpeaker.CircularBuf);
	Len = Len / 4;
#endif
	return Len;
}

//chip->pc 保存数据到缓存区
uint16_t UsbAudioMicDataSet(void *Buffer, uint16_t Len)
{
#ifdef CFG_RES_AUDIO_USB_OUT_EN
	if(!UsbAudioMic.PCMBuffer)
	{
		return 0;
	}
#ifdef CFG_FUNC_SOFT_ADJUST_OUT_USBAUDIO //软件微调
	if((UsbAudioMic.SRAFifo == NULL) && (!SoftFlagGet(SoftFlagBtSra)))
	{
		return 0;
	}
#endif

#ifndef CFG_FUNC_SOFT_ADJUST_OUT_USBAUDIO //软件微调
	#ifdef CFG_RES_AUDIO_USB_SRC_EN
	if(UsbAudioMic.AudioSampleRate != CFG_PARA_SAMPLE_RATE)
	{
		uint32_t i;
		uint32_t temp0 = Len / 128;
		uint32_t temp1 = Len % 128;
		int32_t SRCDoneLen = 0;
		int16_t *p = (int16_t *)Buffer;
		//帧长有大于128的情况，SRC只能128一次
		for(i = 0; i < temp0; i++)
		{
			SRCDoneLen = 0;
			//SRCDoneLen = resampler_apply(UsbAudioMic.Resampler, p + i * 128 * 2, UsbAudioMic.SRCOutBuf, 128);
			SRCDoneLen = resampler_polyphase_apply(UsbAudioMic.Resampler, p + i * 128 * 2, UsbAudioMic.SRCOutBuf, 128);
			MCUCircular_PutData(&UsbAudioMic.CircularBuf, UsbAudioMic.SRCOutBuf, SRCDoneLen * 4);
		}
		if(temp1)
		{
			SRCDoneLen = 0;
			//SRCDoneLen = resampler_apply(UsbAudioMic.Resampler, p + temp0 * 128 * 2, UsbAudioMic.SRCOutBuf, temp1);
			SRCDoneLen = resampler_polyphase_apply(UsbAudioMic.Resampler, p + temp0 * 128 * 2, UsbAudioMic.SRCOutBuf, temp1);
			MCUCircular_PutData(&UsbAudioMic.CircularBuf, UsbAudioMic.SRCOutBuf, SRCDoneLen * 4);
		}
	}
	else //if(UsbAudioMic.AudioSampleRate == 44100)
	#endif ///end of #ifdef CFG_RES_AUDIO_USB_SRC_EN
	{
		MCUCircular_PutData(&UsbAudioMic.CircularBuf, Buffer, Len * 4);
	}
#else
	uint32_t i;
	uint32_t temp0 = Len / 128;
	int32_t SRCDoneLen = 0;
	int32_t SRADoneLen = 0;
	int16_t *p = (int16_t *)Buffer;
	for(i = 0; i < temp0; i++)
	{
		SRADoneLen = AudioSinkSRAProcess(p + i * 128 * 2, 128);
		if(UsbAudioMic.AudioSampleRate != CFG_PARA_SAMPLE_RATE)
		{
			SRCDoneLen = resampler_polyphase_apply(UsbAudioMic.Resampler, (uint8_t*)audioSinkAdjustCt->SraPcmOutnBuf, UsbAudioMic.SRCOutBuf, SRADoneLen);
			MCUCircular_PutData(&UsbAudioMic.CircularBuf, UsbAudioMic.SRCOutBuf, SRCDoneLen * 4);
		}
		else
		{
			MCUCircular_PutData(&UsbAudioMic.CircularBuf, (uint8_t*)audioSinkAdjustCt->SraPcmOutnBuf, SRADoneLen * 4);
		}
	}
#endif
#endif
	return Len;
}

//chip->pc 数据缓存区剩余空间
uint16_t UsbAudioMicSpaceLenGet(void)
{
	uint16_t Len;
	
	if(!UsbAudioMic.PCMBuffer)
	{
		return 0;
	}	
	Len = MCUCircular_GetSpaceLen(&UsbAudioMic.CircularBuf);
	Len = Len / 4;
	return Len;
}


void UsbAudioTimer1msProcess(void)
{
	if(GetSystemMode() != AppModeUsbDevicePlay)
	{
		return;
	}
	FramCount++;
	if(FramCount % 2)//2ms
	{
		return;
	}
#ifdef CFG_RES_AUDIO_USB_IN_EN
	if(UsbAudioSpeaker.AltSet)//open stream
	{
		if(UsbAudioSpeaker.FramCount)//正在传数据 1-2帧数据
		{
			if(UsbAudioSpeaker.FramCount != UsbAudioSpeaker.TempFramCount)
			{
				UsbAudioSpeaker.TempFramCount = UsbAudioSpeaker.FramCount;
				if(AudioCore.AudioSource[USB_AUDIO_SOURCE_NUM].Enable == FALSE)
				{
					AudioCoreSourceEnable(USB_AUDIO_SOURCE_NUM);
				}
			}
			else
			{
				AudioCoreSourceDisable(USB_AUDIO_SOURCE_NUM);
			}
		}
	}
	else
	{
		UsbAudioSpeaker.FramCount = 0;
		UsbAudioSpeaker.TempFramCount = 0;
		AudioCoreSourceDisable(USB_AUDIO_SOURCE_NUM);
	}
#endif

#ifdef CFG_RES_AUDIO_USB_IN_EN
	if(UsbAudioMic.AltSet)//open stream
	{
		if(UsbAudioMic.FramCount)//正在传数据 切传输了1-2帧数据
		{
			if(UsbAudioMic.FramCount != UsbAudioMic.TempFramCount)
			{
				UsbAudioMic.TempFramCount = UsbAudioMic.FramCount;
				if(AudioCore.AudioSink[USB_AUDIO_SINK_NUM].Enable == FALSE)
				{
					AudioCoreSinkEnable(USB_AUDIO_SINK_NUM);
				}
			}
		}
	}
	else
	{
		UsbAudioMic.FramCount = 0;
		UsbAudioMic.TempFramCount = 0;
		AudioCoreSinkDisable(USB_AUDIO_SINK_NUM);
	}
#endif
}

#endif //CFG_APP_USB_AUDIO_MODE_EN

