/**
 **************************************************************************************
 * @file    media_audio_play.h
 * @brief   uDisk and TF Card 
 *
 * @author  kk
 * @version V1.0.0
 *
 * $Created: 2017-3-17 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __MEDIA_AUDIO_PLAY_H__
#define __MEDIA_AUDIO_PLAY_H__

#include "type.h"
//#include "rtos_api.h"
//#include "audio_utility.h"
//#include "mcu_circular_buf.h"

MessageHandle GetMediaPlayMessageHandle(void);

bool MediaPlayCreate(MessageHandle parentMsgHandle);

bool MediaPlayKill(void);

bool MediaPlayStart(void);

bool MediaPlayPause(void);

bool MediaPlayResume(void);

bool MediaPlayStop(void);


#endif /*__MEDIA_AUDIO_PLAY_H__*/
