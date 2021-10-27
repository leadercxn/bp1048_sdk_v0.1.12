/**
 **************************************************************************************
 * @file    remind_sound_service.h
 * @brief   
 *
 * @author  pi
 * @version V1.0.0
 *
 * $Created: 2017-2-26 13:06:47$
 *
 * @Copyright (C) 2018, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __REMIND_SOUND_SERVICE_H__
#define __REMIND_SOUND_SERVICE_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"
#include "decoder_service.h"

#include "remind_sound_item.h"


#define REMIND_SOUND_SERVICE_AUDIO_DECODER_IN_BUF_SIZE	1024 * 19

MessageHandle GetRemindSoundServiceMessageHandle(void);

int32_t RemindSoundServiceCreate(MessageHandle parentMsgHandle);

void RemindSoundServiceStart(void);

void RemindSoundServicePause(void);

void RemindSoundServiceResume(void);


void RemindSoundServiceStop(void);

void RemindSoundServiceKill(void);
//提示音请求，记录条目字符串。
//BlockPlay 指播放不被打断，复位除外。
bool RemindSoundServiceItemRequest(char *SoundItem, bool IsBlock);
#define NO_REMIND			(char*)""//空提示音 。
//TRUE 可申请提示音播放 FALSE：忙
bool RemindSoundServiceRequestStatus(void);

void RemindSoundServicePlay(void);

void RemindSoundServiceReset(void);

void RemindSoundServiceEnd(void);

void RemindSoundServicePlayEnd(void);


bool sound_clips_all_crc(void);



#ifdef __cplusplus
}
#endif//__cplusplus

#endif /* __REMIND_SOUND_SERVICE_H__ */

