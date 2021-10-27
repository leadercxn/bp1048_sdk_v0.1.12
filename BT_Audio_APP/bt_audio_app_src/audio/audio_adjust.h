/**
 **************************************************************************************
 * @file    audio_adjust.h
 * @brief
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2020-3-2 11:17:21$
 *
 * @Copyright (C) 2020, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#ifndef __AUDIO_ADJUST_H__
#define __AUDIO_ADJUST_H__

#include "sra.h"

typedef struct _AudioAdjustContext
{
	SRAContext 			SraObj;//软件微调pcm采样点数，结构体。
	int16_t 			SraPcmOutnBuf[(SRA_BLOCK + SRA_MAX_CHG) * 2];//sra out buff
	int16_t				SraDecIncNum;	 //减少或增加采样点一次。 >0:需要插点数；<0:需要删点数.
}AudioAdjustContext;

extern AudioAdjustContext*	audioSourceAdjustCt;
extern AudioAdjustContext*	audioSinkAdjustCt;//USB 声卡模式专用

#define AUDIO_CORE_SOURCE_SOFT_ADJUST_STEP				2//10// 微调间隔大约2000samples 需要调整一次


//收入硬件微调
void AudioCoreSourceFreqAdjust(void);


void AudioCoreSourceFreqAdjustEnable(uint8_t AsyncIndex, uint16_t LevelLow, uint16_t LevelHigh);

void AudioCoreSourceFreqAdjustDisable(void);

//软件微调
uint16_t AudioSourceSRAProcess(int16_t *InBuf, uint16_t InLen);

void AudioCoreSourceSRAStepCnt(void);

void AudioCoreSourceSRA(void);

void AudioCoreSourceSRAResMalloc(void);

void AudioCoreSourceSRAResRelease(void);

void AudioCoreSourceSRAInit(void);

void AudioCoreSourceSRADeinit(void);

//sink
uint16_t AudioSinkSRAProcess(int16_t *InBuf, uint16_t InLen);

void AudioCoreSinkSRAStepCnt(void);

void AudioCoreSinkSRA(void);

void AudioCoreSinkSRAResMalloc(void);

void AudioCoreSinkSRAResRelease(void);

void AudioCoreSinkSRAInit(void);

void AudioCoreSinkSRADeinit(void);

#endif//__AUDIO_ADJUST_H__
