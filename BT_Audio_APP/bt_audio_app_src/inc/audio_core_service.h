/**
 **************************************************************************************
 * @file    audio_core_service.h
 * @brief   
 *
 * @author  halley
 * @version V1.0.0
 *
 * $Created: 2016-6-29 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __AUDIO_CORE_SERVICE_H__
#define __AUDIO_CORE_SERVICE_H__

#include "type.h"
#include "rtos_api.h"

MessageHandle GetAudioCoreServiceMsgHandle(void);

TaskState GetAudioCoreServiceState(void);

int32_t AudioCoreServiceCreate(MessageHandle parentMsgHandle);


/**
 * @brief	Start audio core service.
 * @param	NONE
 * @return  
 */
int32_t AudioCoreServiceStart(void);

int32_t AudioCoreServicePause(void);

int32_t AudioCoreServiceResume(void);

/**
 * @brief	Exit audio core service.
 * @param	NONE
 * @return  
 */
void AudioCoreServiceStop(void);

int32_t AudioCoreServiceKill(void);


#endif //__AUDIO_CORE_SERVICE_H__

