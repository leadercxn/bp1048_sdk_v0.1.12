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
	SRAContext 			SraObj;//���΢��pcm�����������ṹ�塣
	int16_t 			SraPcmOutnBuf[(SRA_BLOCK + SRA_MAX_CHG) * 2];//sra out buff
	int16_t				SraDecIncNum;	 //���ٻ����Ӳ�����һ�Ρ� >0:��Ҫ�������<0:��Ҫɾ����.
}AudioAdjustContext;

extern AudioAdjustContext*	audioSourceAdjustCt;
extern AudioAdjustContext*	audioSinkAdjustCt;//USB ����ģʽר��

#define AUDIO_CORE_SOURCE_SOFT_ADJUST_STEP				2//10// ΢�������Լ2000samples ��Ҫ����һ��


//����Ӳ��΢��
void AudioCoreSourceFreqAdjust(void);


void AudioCoreSourceFreqAdjustEnable(uint8_t AsyncIndex, uint16_t LevelLow, uint16_t LevelHigh);

void AudioCoreSourceFreqAdjustDisable(void);

//���΢��
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
