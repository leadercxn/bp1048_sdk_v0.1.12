/**
 **************************************************************************************
 * @file    audio_common.h
 * @brief
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2020-8-6 13:17:21$
 *
 * @Copyright (C) 2020, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#ifndef __AUDIO_COMM_H__
#define __AUDIO_COMM_H__

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"
#include "app_config.h"

//不少算法一次处理数据小于128，或者等于128samplers
#define MAX_FRAME_SAMPLES 128


int32_t Get_Resampler_Polyphase(uint32_t resampler);


#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__AUDIO_COMM_H__
