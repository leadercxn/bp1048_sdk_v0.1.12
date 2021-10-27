/**
 **************************************************************************************
 * @file    radio_mode.h
 * @brief   radio
 *
 * @author  pi
 * @version V1.0.0
 *
 * $Created: 2018-1-5 11:40:00$
 *
 * @Copyright (C) 2018, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __RADIO_MODE_H__
#define __RADIO_MODE_H__


#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

bool RadioPlayCreate(MessageHandle parentMsgHandle);

bool RadioPlayStart(void);

bool RadioPlayPause(void);

bool RadioPlayResume(void);

bool RadioPlayStop(void);

bool RadioPlayKill(void);

MessageHandle GetRadioPlayMessageHandle(void);

uint8_t RadioPlayDecoderSourceNum(void);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __RADIO_MODE_H__
