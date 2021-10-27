/**
 **************************************************************************************
 * @file    bt_play_mode.h
 * @brief    
 *
 * @author  kk
 * @version V1.0.0
 *
 * $Created: 2017-3-17 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __BT_PLAY_MODE_H__
#define __BT_PLAY_MODE_H__
#include "type.h"
#include "rtos_api.h"
#include "bt_play_api.h"

bool BtPlayCreate(MessageHandle parentMsgHandle);
bool BtPlayKill(void);
bool BtPlayStart(void);
bool BtPlayStop(void);
MessageHandle GetBtPlayMessageHandle(void);
void SetBtSyncVolume(uint8_t volume);

uint8_t GetBtSyncVolume(void);

void SbcDataNotify(void);

bool GetBtMuteState(void);

void SetBtMuteState(bool MuteState);

void SetBtPlayState(uint8_t state);
BT_PLAYER_STATE GetBtPlayState(void);
void BtPlayControl_PlayPause(void);
bool IsBtTaskRunning(void);

#endif /*__BT_PLAY_MODE_H__*/



