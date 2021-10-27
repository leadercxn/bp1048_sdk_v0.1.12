/**
 **************************************************************************************
 * @file    bt_stack_service.h
 * @brief   
 *
 * @author  kk
 * @version V1.0.0
 *
 * $Created: 2016-6-29 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __BT_STACK_SERVICE_H__
#define __BT_STACK_SERVICE_H__

#include "type.h"
//#include "rtos_api.h"

uint8_t GetBtStackCt(void);

//MessageHandle GetBtStackServiceMsgHandle(void);

TaskState GetBtStackServiceState(void);

/**
 * @brief	Start bluetooth stack service.
 * @param	NONE
 * @return  
 */
void BtStackServiceStart(void);


/**
 * @brief	Kill bluetooth stack service.
 * @param	NONE
 * @return  
 */
bool BtStackServiceKill(void);


void BtBbStart(void);

void BT_IntDisable(void);

void BT_ModuleClose(void);

void BtStackServiceWaitResume(void);

void BtFastPowerOff(void);
void BtFastPowerOn(void);

#endif //__AUDIO_CORE_SERVICE_H__

