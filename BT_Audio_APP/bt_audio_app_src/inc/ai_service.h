/*
 * ai_service.h
 *
 *  Created on: 2020-03-03
 *      Author: tony
 */

#ifndef _AT_SERVICE_H_
#define _AT_SERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include <stdio.h>
#include "type.h"
#include "rtos_api.h"

/* Typedef -------------------------------------------------------------------*/

/* Define --------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* API -----------------------------------------------------------------------*/

void ai_start(void);

void XiaoAiTaskMsgSendRun();
void XiaoAiTaskMsgSendStart();
void XiaoAiTaskMsgSendStop();
void XiaoAiStopMsgSendtoMain();
xTaskHandle XmAiGetTaskHandle();
int32_t XmAiPlayCreate(MessageHandle parentMsgHandle);
uint32_t XmAiPlayKill(void);


#ifdef __cplusplus
}
#endif




#endif /* _AT_SERVICE_H_ */
