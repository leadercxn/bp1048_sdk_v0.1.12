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
#include "debug.h"
#include "delay.h"
#include "dac.h"
#include "otg_device_hcd.h"
#include "otg_device_audio.h"
#include "otg_device_standard_request.h"
#include "mcu_circular_buf.h"
#include "timer.h"
#include "resampler.h"
#include "usb_audio_api.h"
#include "otg_device_audio.h"



#define	USB_FIFO_LEN						(256 * 2 * 2 * 8)
#define USB_AUDIO_SRC_BUF_LEN				(150 * 2 * 2)


//#define USE_AUDIO_CORE
#ifdef USE_AUDIO_CORE
#else
uint32_t  usb_speaker_enable = 0;
uint32_t  usb_mic_enable = 0;
#endif

//#define USE_MALLOC
#ifdef USE_MALLOC
#else
ResamplerContext UsbAudioSpeaker_Resampler;
uint8_t UsbAudioSpeaker_PCMBuffer[USB_FIFO_LEN];
ResamplerContext UsbAudioMic_Resampler;
uint8_t UsbAudioMic_SRCOutBuf[USB_AUDIO_SRC_BUF_LEN];
uint8_t UsbAudioMic_PCMBuffer[USB_FIFO_LEN];
#endif


#ifdef CFG_APP_USB_AUDIO_MODE_EN
bool 	IsUsbAudioMode = FALSE;
extern UsbAudio UsbAudioSpeaker;
extern UsbAudio UsbAudioMic;
static uint32_t FramCount = 0;
void UsbSraInit(void);

//USB声卡模式参数配置，资源初始化
bool UsbDevicePlayInit(void)
{
	memset(&UsbAudioSpeaker,0,sizeof(UsbAudio));
	memset(&UsbAudioMic,0,sizeof(UsbAudio));
	UsbAudioSpeaker.Channel    = 2;
	UsbAudioMic.Channel        = 2;
	UsbAudioSpeaker.LeftVol    = AUDIO_MAX_VOLUME;
	UsbAudioSpeaker.RightVol   = AUDIO_MAX_VOLUME;
	UsbAudioMic.LeftVol        = AUDIO_MAX_VOLUME;
	UsbAudioMic.RightVol       = AUDIO_MAX_VOLUME;


	//转采样	
#ifdef  CFG_RES_AUDIO_USB_IN_EN

   #ifdef CFG_RES_AUDIO_USB_SRC_EN
#ifdef USE_MALLOC
	UsbAudioSpeaker.Resampler = (ResamplerContext*)osPortMalloc(sizeof(ResamplerContext));
#else
	UsbAudioSpeaker.Resampler = &UsbAudioSpeaker_Resampler;
#endif
	if(UsbAudioSpeaker.Resampler == NULL)
	{
		DBG("UsbAudioSpeaker.Resampler memory error\n");
		return FALSE;
	}
	#endif  ///end of CFG_RES_AUDIO_USB_SRC_EN
	
	//Speaker FIFO
#ifdef USE_MALLOC
	UsbAudioSpeaker.PCMBuffer = osPortMalloc(USB_FIFO_LEN);
#else
	UsbAudioSpeaker.PCMBuffer = (int16_t *)UsbAudioSpeaker_PCMBuffer;
#endif
	if(UsbAudioSpeaker.PCMBuffer == NULL)
	{
		DBG("UsbAudioSpeaker.PCMBuffer memory error\n");
		return FALSE;
	}
	memset(UsbAudioSpeaker.PCMBuffer,0,USB_FIFO_LEN);
	MCUCircular_Config(&UsbAudioSpeaker.CircularBuf, UsbAudioSpeaker.PCMBuffer, USB_FIFO_LEN);

#endif//end of CFG_RES_USB_IN_EN

	//转采样
#ifdef CFG_RES_AUDIO_USB_OUT_EN

    #ifdef CFG_RES_AUDIO_USB_SRC_EN
#ifdef USE_MALLOC
	UsbAudioMic.Resampler = (ResamplerContext*)osPortMalloc(sizeof(ResamplerContext));
#else
	UsbAudioMic.Resampler = &UsbAudioMic_Resampler;
#endif
	if(UsbAudioMic.Resampler == NULL)
	{
		DBG("UsbAudioMic.Resampler memory error\n");
		return FALSE;
	}
#ifdef USE_MALLOC
	UsbAudioMic.SRCOutBuf = osPortMalloc(USB_AUDIO_SRC_BUF_LEN);
#else
	UsbAudioMic.SRCOutBuf = (int16_t *)UsbAudioMic_SRCOutBuf;
#endif
	if(UsbAudioMic.SRCOutBuf == NULL)
	{
		DBG("UsbAudioMic.SRCOutBuf memory error\n");
		return FALSE;
	}
	#endif //end of #ifdef CFG_RES_AUDIO_USB_SRC_EN
	
	//MIC FIFO
#ifdef USE_MALLOC
	UsbAudioMic.PCMBuffer = osPortMalloc(USB_FIFO_LEN);
#else
	UsbAudioMic.PCMBuffer = (int16_t *)UsbAudioMic_PCMBuffer;
#endif
	if(UsbAudioMic.PCMBuffer == NULL)
	{
		DBG("UsbAudioMic.PCMBuffer memory error\n");
		return FALSE;
	}
	memset(UsbAudioMic.PCMBuffer,0,USB_FIFO_LEN);
	MCUCircular_Config(&UsbAudioMic.CircularBuf, UsbAudioMic.PCMBuffer, USB_FIFO_LEN);

#endif///end of CFG_REGS_AUDIO_USB_OUT_EN

	return TRUE;
}



void UsbAudioMicSampleRateChange(uint32_t SampleRate)
{
#ifdef CFG_RES_AUDIO_USB_OUT_EN
	if(SampleRate == 48000)
	{
		resampler_init(UsbAudioMic.Resampler, 2,CFG_PARA_SAMPLE_RATE, 48000, 0, 0);
	}
#endif
	MCUCircular_Config(&UsbAudioMic.CircularBuf, UsbAudioMic.PCMBuffer, USB_FIFO_LEN);
}

void UsbAudioSpeakerSampleRateChange(uint32_t SampleRate)
{
#ifdef CFG_RES_AUDIO_USB_IN_EN
	if(SampleRate == 48000)
	{
		resampler_init(UsbAudioSpeaker.Resampler, 2, 48000, CFG_PARA_SAMPLE_RATE, 0, 0);
	}
#endif
	MCUCircular_Config(&UsbAudioSpeaker.CircularBuf, UsbAudioSpeaker.PCMBuffer, USB_FIFO_LEN);
}


//////////////////////////////////////////////////audio core api/////////////////////////////////////////////////////
//pc->chip 从缓存区获取数据
uint16_t UsbAudioSpeakerDataGet(void *Buffer,uint16_t Len)
{
	uint16_t Length = 0;
	if(!UsbAudioSpeaker.PCMBuffer) return 0;
	Length = Len*4;
	Length = MCUCircular_GetData(&UsbAudioSpeaker.CircularBuf,Buffer,Length);
	return Length/4;
}

//pc->chip 获取缓存区数据长度
uint16_t UsbAudioSpeakerDataLenGet(void)
{
	uint16_t Len;
	if(!UsbAudioSpeaker.PCMBuffer) return 0;
	Len = MCUCircular_GetDataLen(&UsbAudioSpeaker.CircularBuf);
	Len = Len/4;
	return Len;
}

//chip->pc 保存数据到缓存区
uint16_t UsbAudioMicDataSet(void *Buffer,uint16_t Len)
{
#ifdef CFG_RES_AUDIO_USB_OUT_EN
	if(!UsbAudioMic.PCMBuffer)
		return 0;

    #ifdef CFG_RES_AUDIO_USB_SRC_EN
	if(UsbAudioMic.AudioSampleRate == 48000)
	{
		uint32_t i;
		uint32_t temp0 = Len/128;
		uint32_t temp1 = Len%128;
		int32_t SRCDoneLen;
		int16_t *p = Buffer;
		for(i=0;i<temp0;i++)
		{
			SRCDoneLen = resampler_apply(UsbAudioMic.Resampler, (int16_t*)(p+(i*128*2)), UsbAudioMic.SRCOutBuf, 128);
			MCUCircular_PutData(&UsbAudioMic.CircularBuf, UsbAudioMic.SRCOutBuf, SRCDoneLen * 4);
		}
		if(temp1)
		{
			SRCDoneLen = resampler_apply(UsbAudioMic.Resampler, (int16_t*)(p+(temp0*128*2)), UsbAudioMic.SRCOutBuf, temp1);
			MCUCircular_PutData(&UsbAudioMic.CircularBuf, UsbAudioMic.SRCOutBuf, SRCDoneLen * 4);
		}
	}
	else //if(UsbAudioMic.AudioSampleRate == 44100)
	#endif ///end of #ifdef CFG_RES_AUDIO_USB_SRC_EN
	{
		MCUCircular_PutData(&UsbAudioMic.CircularBuf, Buffer, Len*4);
	}
#endif
	return Len;
}


//chip->pc 数据缓存区剩余空间
uint16_t UsbAudioMicSpaceLenGet(void)
{
	uint16_t Len;
	if(!UsbAudioMic.PCMBuffer) return 0;
	Len = MCUCircular_GetSpaceLen(&UsbAudioMic.CircularBuf);
	Len = Len/4;
	return Len;
}

//usb设备使能
void UsbDeviceEnable(void)
{
	DBG("UsbDevice:App enable\n");
#ifdef CFG_RES_AUDIO_USB_IN_EN
    #ifdef CFG_RES_AUDIO_USB_SRC_EN
	resampler_init(UsbAudioSpeaker.Resampler, 2, 48000, CFG_PARA_SAMPLE_RATE, 0, 0);
	#endif
#endif

#ifdef CFG_RES_AUDIO_USB_OUT_EN 
       #ifdef CFG_RES_AUDIO_USB_SRC_EN
	   resampler_init(UsbAudioMic.Resampler, 2,CFG_PARA_SAMPLE_RATE, 48000, 0, 0);
	   #endif
#endif

	IsUsbAudioMode = TRUE;
	OTG_DeviceInit();
	NVIC_EnableIRQ(Usb_IRQn);
	NVIC_SetPriority(Usb_IRQn,0);
	NVIC_SetPriority(BT_IRQn,1);
	NVIC_SetPriority(BLE_IRQn,1);
	UsbSraInit();
}


//usb设备关闭
void UsbDeviceDisable(void)
{
	DBG("UsbDevice:Appn disable\n");
	NVIC_DisableIRQ(Usb_IRQn);
	OTG_DeviceDisConnect();
	IsUsbAudioMode = FALSE;
#ifdef CFG_RES_AUDIO_USB_IN_EN
#ifdef USE_AUDIO_CORE
	AudioCoreSourceDisable(USB_AUDIO_SOURCE_NUM);
#else
	usb_speaker_enable = 0;
#endif
#endif
#ifdef CFG_RES_AUDIO_USB_OUT_EN
#ifdef USE_AUDIO_CORE
	AudioCoreSinkDisable(AUDIO_USB_OUT_SINK_NUM);
#else
	usb_mic_enable = 0;
#endif
#endif
}


//usb 声卡状态监控
void UsbAudioTimer1msProcess(void)
{
	if(IsUsbAudioMode == FALSE)
	{
#ifdef USE_AUDIO_CORE
		AudioCoreSourceDisable(USB_AUDIO_SOURCE_NUM);
		AudioCoreSinkDisable(AUDIO_USB_OUT_SINK_NUM);
#else
		usb_speaker_enable = 0;
		usb_mic_enable = 0;
#endif
		return;
	}
	FramCount++;
	if(FramCount%2)//2ms
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
#ifdef USE_AUDIO_CORE
				AudioCoreSourceEnable(USB_AUDIO_SOURCE_NUM);
#else
				usb_speaker_enable = 1;
#endif
			}
			else
			{
#ifdef USE_AUDIO_CORE
				AudioCoreSourceDisable(USB_AUDIO_SOURCE_NUM);
#else
				usb_speaker_enable = 0;
#endif
			}
		}
	}
	else
	{
		UsbAudioSpeaker.FramCount = 0;
		UsbAudioSpeaker.TempFramCount = 0;
#ifdef USE_AUDIO_CORE
		AudioCoreSourceDisable(USB_AUDIO_SOURCE_NUM);
#else
		usb_speaker_enable = 0;
#endif
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
#ifdef USE_AUDIO_CORE
				if(AudioCore.AudioSink[AUDIO_USB_OUT_SINK_NUM].Enable == FALSE)
				{
					AudioCoreSinkEnable(AUDIO_USB_OUT_SINK_NUM);
				}
#else
				if(usb_mic_enable==0)
				{
					usb_mic_enable = 1;
				}
#endif
			}
		}
	}
	else
	{
		UsbAudioMic.FramCount = 0;
		UsbAudioMic.TempFramCount = 0;
#ifdef USE_AUDIO_CORE
		AudioCoreSinkDisable(AUDIO_USB_OUT_SINK_NUM);
#else
		usb_mic_enable = 0;
#endif
	}
#endif
}

#endif

