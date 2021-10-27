/**
 **************************************************************************************
 * @file    device_service.h
 * @brief   linein 
 *
 * @author  pi
 * @version V1.0.0
 *
 * $Created: 2018-1-9 15:40:00$
 *
 * @Copyright (C) 2018, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
**************************************************************************************
*/
#ifndef __DEVICE_SERVICE_H__
#define __DEVICE_SERVICE_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "rtos_api.h"
extern osMutexId SDIOMutex;

int32_t DeviceServiceCreate(MessageHandle parentMsgHandle);

MessageHandle GetDeviceMessageHandle(void);

int32_t DeviceServiceStart(void);

void DeviceServicePause(void);

void DeviceServiceResume(void);

void SetRecState(uint32_t state);

void EncoderServicePause(void);

#ifdef __cplusplus
}
#endif//__cplusplus

#endif //__DEVICE_SERVICE_H__

