/**
 **************************************************************************************
 * @file    rest_play.h
 * @brief   rest
 *
 * @author  pi
 * @version V1.0.0
 *
 * $Created: 2018-10-22 11:40:00$
 *
 * @Copyright (C) 2018, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __REST_MODE_H__
#define __REST_MODE_H__



#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 



MessageHandle GetRestPlayMessageHandle(void);

bool RestPlayCreate(MessageHandle parentMsgHandle);

bool RestPlayKill(void);

bool RestPlayStart(void);

bool RestPlayPause(void);

bool RestPlayResume(void);

bool RestPlayStop(void);

#ifdef __cplusplus
}
#endif // __cplusplus 

#endif // __REST_MODE_H__
