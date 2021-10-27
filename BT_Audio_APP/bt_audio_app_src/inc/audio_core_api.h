/**
 **************************************************************************************
 * @file    audio_core_api.h
 * @brief   audio core 
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2016-6-29 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __AUDIO_CORE_API_H__
#define __AUDIO_CORE_API_H__

typedef uint16_t (*AudioCoreDataGetFunc)(void* Buf, uint16_t Samples);
typedef uint16_t (*AudioCoreDataLenGetFunc)(void);
typedef uint16_t (*AudioCoreDataSetFunc)(void* Buf, uint16_t Samples);
typedef uint16_t (*AudioCoreDataSpaceLenSetFunc)(void);

typedef void (*AudioCoreProcessFunc)(void*);//音效处理主函数

#include "app_config.h"

#define AUDIO_CORE_SOURCE_MAX_MUN	4

#define MIC_SOURCE_NUM			0 //麦克风在audiocore source通道,音效预处理 必须为0
#define APP_SOURCE_NUM			1 //app主要音源通道,可配音效
#define REMIND_SOURCE_NUM		2 //提示音使用固定混音通道 无音效
#define PLAYBACK_SOURCE_NUM		3//flashfs 录音回放通道		无音效
#define USB_AUDIO_SOURCE_NUM	APP_SOURCE_NUM

enum
{
	AUDIO_DAC0_SINK_NUM,		//主音频输出在audiocore Sink中的通道，必须配置，audiocore借用此通道buf处理数据	
	#ifdef CFG_FUNC_RECORDER_EN
	AUDIO_RECORDER_SINK_NUM,	//录音专用通道		 不叠加提示音音源。	
	#endif
	#if (defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT == ENABLE))
	AUDIO_HF_SCO_SINK_NUM,	    //蓝牙sco发送数据通道
	#endif
	#ifdef CFG_RES_AUDIO_DACX_EN
	AUDIO_DACX_SINK_NUM,		//dacx通道
	#endif
	#ifdef CFG_RES_AUDIO_I2SOUT_EN
	AUDIO_I2SOUT_SINK_NUM,      //i2s_out通道
	#endif
	#ifdef CFG_APP_USB_AUDIO_MODE_EN
	USB_AUDIO_SINK_NUM,         //usb_out通道
	#endif
	AUDIO_CORE_SINK_MAX_NUM,
	
};

typedef struct _AudioCoreSource
{
	uint8_t						Index;
	uint8_t						PcmFormat;	//数据格式,sign or stereo
	bool						Enable; 	//
	bool						IsSreamData;//是否流数据，用于决定该通路数据是否要满一帧
	AudioCoreDataGetFunc		FuncDataGet;//****流数据入口函数
	AudioCoreDataLenGetFunc		FuncDataGetLen;
	int16_t						*PcmInBuf;	//缓存数据buf
	int16_t						PreGain;
	int16_t						LeftVol;	//音量
	int16_t						RightVol;	//音量
	int16_t						LeftCurVol;	//当前音量
	int16_t						RightCurVol;//当前音量
	bool						LeftMuteFlag;//静音标志
	bool						RightMuteFlag;//静音标志
}AudioCoreSource;


typedef struct _AudioCoreSink
{
	uint8_t							Index;
	uint8_t							PcmFormat;	//数据格式，1:单声道输出; 2:立体声输出
	bool							Enable;
	uint8_t							SreamDataState;//0:流数据;1：检测sinkbuf是否足够；2：
	AudioCoreDataSetFunc			FuncDataSet;//****推流入口 内buf->外缓冲搬移函数
	AudioCoreDataSpaceLenSetFunc	FuncDataSpaceLenGet;	
	int16_t							*PcmOutBuf;
	int16_t							LeftVol;	//音量
	int16_t							RightVol;	//音量
	int16_t							LeftCurVol;	//当前音量
	int16_t							RightCurVol;//当前音量
	bool							LeftMuteFlag;//静音标志
	bool							RightMuteFlag;//静音标志

}AudioCoreSink;

typedef struct _AudioCoreContext
{
	AudioCoreSource AudioSource[AUDIO_CORE_SOURCE_MAX_MUN];
	AudioCoreProcessFunc AudioEffectProcess;			//****流处理入口
	AudioCoreSink   AudioSink[AUDIO_CORE_SINK_MAX_NUM];

}AudioCoreContext;

extern AudioCoreContext		AudioCore;

//typedef void (*AudioCoreProcessFunc)(AudioCoreContext *AudioCore);
/**
 * @func        AudioCoreSourceFreqAdjustEnable
 * @brief       使能系统音频分频微调，使信道之间同步(与异步音源)
 * @param       uint8_t AsyncIndex   异步音频源混音信道编号
 * @param       uint16_t LevelLow   异步音频源混音信道低水位采样点值
 * @param       uint16_t LevelHigh   异步音频源混音信道高水位采样点值
 * @Output      None
 * @return      None
 * @Others
 * Record
 * 1.Date        : 20180518
 *   Author      : pi.wang
 *   Modification: Created function
 */
void AudioCoreSourceFreqAdjustEnable(uint8_t AsyncIndex, uint16_t LevelLow, uint16_t LevelHigh);


void AudioCoreSourceFreqAdjustDisable(void);


void AudioCoreSourcePcmFormatConfig(uint8_t Index, uint16_t Format);

void AudioCoreSourceEnable(uint8_t Index);

void AudioCoreSourceDisable(uint8_t Index);

void AudioCoreSourceMute(uint8_t Index, bool IsLeftMute, bool IsRightMute);

void AudioCoreSourceUnmute(uint8_t Index, bool IsLeftUnmute, bool IsRightUnmute);

void AudioCoreSourceVolSet(uint8_t Index, uint16_t LeftVol, uint16_t RightVol);

void AudioCoreSourceVolGet(uint8_t Index, uint16_t* LeftVol, uint16_t* RightVol);

void AudioCoreSourceConfig(uint8_t Index, AudioCoreSource* Source);

void AudioCoreSinkEnable(uint8_t Index);

void AudioCoreSinkDisable(uint8_t Index);

void AudioCoreSinkMute(uint8_t Index, bool IsLeftMute, bool IsRightMute);

void AudioCoreSinkUnmute(uint8_t Index, bool IsLeftUnmute, bool IsRightUnmute);

void AudioCoreSinkVolSet(uint8_t Index, uint16_t LeftVol, uint16_t RightVol);

void AudioCoreSinkVolGet(uint8_t Index, uint16_t* LeftVol, uint16_t* RightVol);

void AudioCoreSinkConfig(uint8_t Index, AudioCoreSink* Sink);

void AudioCoreProcessConfig(AudioCoreProcessFunc AudioEffectProcess);


bool AudioCoreInit(void);

void AudioCoreDeinit(void);

void AudioCoreRun(void);

//音量调整
void AduioCoreSourceVolSet(void);
void AduioCoreSinkVolSet(void);
void AudioCoreAppSourceVolSet(uint16_t Source,int16_t *pcm_in,uint16_t n,uint16_t Channel);

#endif //__AUDIO_CORE_API_H__
