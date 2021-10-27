/**
 **************************************************************************************
 * @file    display_service.h
 * @brief
 *
 * @author  davidlin
 * @version V1.0.0
 *
 * $Created: 2019-6-10 10:09:00$
 *
 * @Copyright (C) 2018, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
**************************************************************************************
*/
#ifndef __DISPLAY_SERVICE_H__
#define __DISPLAY_SERVICE_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "rtos_api.h"

int32_t DisplayServiceCreate(MessageHandle parentMsgHandle);

MessageHandle GetDisplayMessageHandle(void);

int32_t DisplayServiceStart(void);

void DisplayServicePause(void);

void DisplayServiceResume(void);


#ifdef __cplusplus
}
#endif//__cplusplus

#endif //__DISPLAY_SERVICE_H__

