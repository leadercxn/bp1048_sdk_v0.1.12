/**
 **************************************************************************************
 * @file    waiting_play.h
 * @brief   waiting
 *
 * @author  pi
 * @version V1.0.0
 *
 * $Created: 2018-10-22 11:40:00$
 *
 * @Copyright (C) 2018, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __WAITING_MODE_H__
#define __WAITING_MODE_H__



#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 



MessageHandle GetWaitingPlayMessageHandle(void);

bool WaitingPlayCreate(MessageHandle parentMsgHandle);

bool WaitingPlayKill(void);

bool WaitingPlayStart(void);

bool WaitingPlayPause(void);

bool WaitingPlayResume(void);

bool WaitingPlayStop(void);

#ifdef __cplusplus
}
#endif // __cplusplus 

#endif // __WAITING_MODE_H__
