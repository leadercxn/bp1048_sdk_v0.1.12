/**
 **************************************************************************************
 * @file    display_service.c
 * @brief   
 *
 * @author  david
 * @version V1.0.0
 *
 * $Created: 2019-6-10 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include <string.h>
#include "type.h"
#include "app_config.h"//for BOARD
#include "gpio.h" //for BOARD
#include "debug.h"
#include "rtos_api.h"
#include "main_task.h"
#include "app_message.h"
#include "display_service.h"
#include "mode_switch_api.h"
#include "backup_interface.h"
#include "breakpoint.h"
#include "timeout.h"
#include "media_play_api.h"
#include "radio_api.h"
//#ifdef CFG_FUNC_DISPLAY_EN
#include "display.h"
//#endif
#ifdef CFG_FUNC_POWER_MONITOR_EN
#include "power_monitor.h"
#endif

#define DISPLAY_SERVICE_SIZE				512//1024
#define DISPLAY_SERVICE_PRIO				3
#define DISPLAY_SERVICE_TIMEOUT			    100	/* 10 ms */

#define NUM_MESSAGE_QUEUE				4

typedef struct _DisplayServiceContext
{
	xTaskHandle		taskHandle;
	MessageHandle	msgHandle;
	MessageHandle	parentMsgHandle;
	TaskState		serviceState;
}DisplayServiceContext;

static DisplayServiceContext			displayServiceCt;

/**
 * @brief	Display servcie init
 * @param	MessageHandle parentMsgHandle
 * @return	0 for success
 */
static int32_t DisplayServiceInit(MessageHandle parentMsgHandle)
{
	memset(&displayServiceCt, 0, sizeof(DisplayServiceContext));
	/* register message handle */
	displayServiceCt.msgHandle = MessageRegister(NUM_MESSAGE_QUEUE);
	if(displayServiceCt.msgHandle == NULL)
	{
		return -1;
	}
	displayServiceCt.parentMsgHandle = parentMsgHandle;
	displayServiceCt.serviceState = TaskStateNone;
	//DispInit();
	return 0;
}

static void DisplayServiceDeinit(void)
{
	MessageContext		msgSend;

	displayServiceCt.serviceState = TaskStateNone;

	/* Send message to main app */
	msgSend.msgId		= MSG_DISPLAY_SERVICE_STOPPED;
	MessageSend(displayServiceCt.parentMsgHandle, &msgSend);
}

//sleep«∞display ‘›Õ££¨
static void DisplayServiceEntrance(void * param)
{
	MessageContext		msgRecv;
	MessageContext		msgSend;

	/* Send message to main app */
	msgSend.msgId		= MSG_DISPLAY_SERVICE_CREATED;
	MessageSend(displayServiceCt.parentMsgHandle, &msgSend);
	displayServiceCt.serviceState = TaskStateReady;

	while(1)
	{

		MessageRecv(displayServiceCt.msgHandle, &msgRecv, DISPLAY_SERVICE_TIMEOUT);
		//APP_DBG("Display service run\n");
		switch(msgRecv.msgId)
		{
			case MSG_TASK_START:
				if(displayServiceCt.serviceState == TaskStateReady)
				{
					displayServiceCt.serviceState = TaskStateRunning;

					msgSend.msgId		= MSG_DISPLAY_SERVICE_STARTED;
					MessageSend(displayServiceCt.parentMsgHandle, &msgSend);
				}
				break;

			case MSG_APP_EXIT:
				break;//default
				

			case MSG_TASK_PAUSE:
				displayServiceCt.serviceState = TaskStatePaused;
				msgSend.msgId		= MSG_DISPLAY_SERVICE_PAUSED;
				MessageSend(displayServiceCt.parentMsgHandle, &msgSend);
				DispDeInit();
				break;
			case MSG_TASK_RESUME:
#if defined(CFG_FUNC_DISPLAY_EN)
				/* Init display*/
				//DispInit(0);
#endif

				displayServiceCt.serviceState = TaskStateRunning;
				break;
			
			case MSG_TASK_STOP:
				DisplayServiceDeinit();
				/*Wait for kill*/
				break;
				
			default:
				break;
		}

		if(displayServiceCt.serviceState == TaskStateRunning)
		{
			Display(msgRecv.msgId);
		}
	}
}

/**
 * @brief	Start display service.
 * @param	MessageHandle parentMsgHandle
 * @return
 */
int32_t DisplayServiceCreate(MessageHandle parentMsgHandle)
{
	int32_t		ret = 0;

	ret = DisplayServiceInit(parentMsgHandle);
	if(!ret)
	{
		displayServiceCt.taskHandle = NULL;
		xTaskCreate(DisplayServiceEntrance,
				"DisplayService",
				DISPLAY_SERVICE_SIZE,
				NULL, DISPLAY_SERVICE_PRIO,
				&displayServiceCt.taskHandle);
		if(displayServiceCt.taskHandle == NULL)
		{
			ret = -1;
		}
	}
	if(ret)
	{
		APP_DBG("Display Service create fail!\n");
	}
	return ret;
}


/***************************************************************************************
 *
 * APIs
 *
 */

/**
 * @brief	Get message receive handle of audio core manager
 * @param	NONE
 * @return	MessageHandle
 */
MessageHandle GetDisplayMessageHandle(void)
{
	return displayServiceCt.msgHandle;
}

/**
 * @brief	Start display service.
 * @param	NONE
 * @return  
 */
int32_t DisplayServiceStart(void)
{
	MessageContext		msgSend;

	msgSend.msgId		= MSG_TASK_START;
	MessageSend(displayServiceCt.msgHandle, &msgSend);

	return 0;
}

/**
 * @brief	Pause display service.
 * @param	NONE
 * @return  
 */
void DisplayServicePause(void)
{
	MessageContext		msgSend;

	msgSend.msgId		= MSG_TASK_PAUSE;
	MessageSend(displayServiceCt.msgHandle, &msgSend);
	return ;
}

/**
 * @brief	Resume display service.
 * @param	NONE
 * @return  
 */
void DisplayServiceResume(void)
{
	MessageContext		msgSend;

	msgSend.msgId		= MSG_TASK_RESUME;
	MessageSend(displayServiceCt.msgHandle, &msgSend);
	return ;
}

/**
 * @brief	Stop display service.
 * @param	NONE
 * @return  
 */
void DisplayServiceStop(void)
{
	return ;
}

/**
 * @brief	Stop device service.
 * @param	NONE
 * @return  
 */
void DisplayServiceKill(void)
{
	//task
	if(displayServiceCt.taskHandle != NULL)
	{
		vTaskDelete(displayServiceCt.taskHandle);
		displayServiceCt.taskHandle = NULL;
	}

	//Msgbox
	if(displayServiceCt.msgHandle != NULL)
	{
		MessageDeregister(displayServiceCt.msgHandle);
		displayServiceCt.msgHandle = NULL;
	}
	//PortFree
}

