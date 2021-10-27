/*
 * RTOS_API.c
 *
 *  Created on: Aug 30, 2016
 *      Author: peter
 */
#include <stdint.h>
#include <stddef.h>
#include "type.h"
#include <string.h>
#include "rtos_api.h"
#include "remap.h"
#include "task.h"
#ifdef CFG_APP_CONFIG
#include "app_config.h"
#endif
#include "debug.h"
#include <nds32_intrinsic.h>
#include "timeout.h"

extern void vPortDefineHeapRegions( const HeapRegion_t * const pxHeapRegions ); //defined by heap_5s.c add for warning by pi
uint32_t GetIPSR( void );
/* Determine whether we are in thread mode or handler mode. */
uint32_t inHandlerMode (void)
{
	//return 0;
	return GetIPSR();
}

bool MessageSend(MessageHandle msgHandle,  MessageContext * msgContext)
{
	if(msgHandle == NULL)
	{
		return  FALSE;
	}

	if (xQueueSend(msgHandle, msgContext, 0) != pdTRUE)
	{
		return FALSE;
	}

	return TRUE;
}

bool MessageSendFromISR(MessageHandle msgHandle,  MessageContext * msgContext)
{
	portBASE_TYPE taskWoken = pdFALSE;

	if (xQueueSendFromISR(msgHandle, msgContext, &taskWoken) != pdTRUE)
	{
		return FALSE;
	}
	portEND_SWITCHING_ISR(taskWoken);
	
	return TRUE;
}

void MessageSendx(MessageHandle msgHandle,  MessageContext * msgContext)
{
	if(msgHandle == NULL)
	{
		return ;
	}

	xQueueSend(msgHandle, msgContext, 0xFFFFFFFF);
}


bool MessageRecv(MessageHandle msgHandle, MessageContext * msgContext, uint32_t millisec)
{
	bool			ret = FALSE;

	if (msgHandle == NULL)
	{
		return FALSE;
	}

	msgContext->msgId = MSG_INVAILD;


	if (xQueueReceive(msgHandle, msgContext, millisec) == pdTRUE)
	{
		/* We have mail */
		ret = TRUE;
	}
	else
	{
		ret = FALSE;
	}


	return ret;
}

//系统使用的ram结束地址和em的使用大小有关系
#ifdef CFG_APP_CONFIG
static uint32_t gB1xSramEndAddr = BB_MPU_START_ADDR;
#endif
void prvInitialiseHeap(void)
{
	extern char _end;
	HeapRegion_t xHeapRegions[2];

	xHeapRegions[0].pucStartAddress = (uint8_t*)&_end;

#ifdef CFG_APP_CONFIG
	xHeapRegions[0].xSizeInBytes = gB1xSramEndAddr-(uint32_t)&_end;
#else
	xHeapRegions[0].xSizeInBytes = gSramEndAddr-(uint32_t)&_end;
#endif

	xHeapRegions[1].pucStartAddress = NULL;
	xHeapRegions[1].xSizeInBytes = 0;

	vPortDefineHeapRegions( (HeapRegion_t *)xHeapRegions );
}

//note: 改函数必须在prvInitialiseHeap()函数调用之后
void osSemaphoreMutexCreate(void)
{
    if(UART0Mutex == NULL)
    {
    	UART0Mutex = xSemaphoreCreateMutex();
    }
	if(UART1Mutex == NULL)
	{
		UART1Mutex = xSemaphoreCreateMutex();
	}
#ifdef	CFG_FLASH_MUTEX_USE
	if(FlashMutex == NULL)
	{
		FlashMutex = xSemaphoreCreateMutex();
	}
#endif
#ifdef	CFG_RES_CARD_USE
	if(SDIOMutex == NULL)
	{
		SDIOMutex = osMutexCreate();
	}
#endif
#ifdef CFG_RES_UDISK_USE
	if(UDiskMutex == NULL)
	{
		UDiskMutex = osMutexCreate();
	}
#endif
#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	if(AudioEffectMutex == NULL)
	{
		AudioEffectMutex = osMutexCreate();
	}
	if(LoadAudioParamMutex == NULL)
	{
		LoadAudioParamMutex = osMutexCreate();
	}
#endif

#if (defined(CFG_APP_BT_MODE_EN) && defined(BT_TWS_SUPPORT))
	if(SbcDecoderMutex == NULL)
	{
		SbcDecoderMutex = osMutexCreate();
	}
#endif
}

/**
* @brief Allocate a memory block from a memory pool
* @param  osWantedSize Allocate memory word(1word=4byte) size
* @retval  address of the allocated memory block or NULL in case of no memory available.
*/
void *osPortMalloc_1(uint16_t osWantedSize)
{
	void *ospvReturn = NULL;
	vPortEnterCritical();
	ospvReturn=pvPortMalloc(osWantedSize);
	vPortExitCritical();
#ifdef OS_MEM_DEBUG
	DBG("Malloc:%08x %d %d ",ospvReturn, (int)osWantedSize, (int)xPortGetFreeHeapSize());
#endif
	return ospvReturn;
}
void *pvPortMallocFromEnd( size_t xWantedSize );
/**
* @brief Allocate a memory block from a memory pool
* @param  osWantedSize Allocate memory word(1word=4byte) size
* @retval  address of the allocated memory block or NULL in case of no memory available.
*/
void *osPortMallocFromEnd_1(uint32_t osWantedSize)
{
	void *ospvReturn = NULL;

	//DBG("\nMalloc22:%d, %d\n", xPortGetFreeHeapSize(), osWantedSize);
	ospvReturn = pvPortMallocFromEnd(osWantedSize);
	//DBG("Malloc33:%d->%d, Add=%x\n", (int)osWantedSize, (int)xPortGetFreeHeapSize(), ospvReturn);
#ifdef OS_MEM_DEBUG
	DBG("Malloc:%08x %d %d ",ospvReturn, (int)osWantedSize, (int)xPortGetFreeHeapSize());
#endif
	return ospvReturn;
}

int osPortRemainMem(void)
{
	return (int)xPortGetFreeHeapSize();
}

/**
* @brief Free a memory block
* @param address of the allocated memory block.
*/
void osPortFree(void *ospv)
{
	//DBG("Add=%x\n", ospv);
	vPortEnterCritical();
	vPortFree(ospv);
	vPortExitCritical();
}

void osTaskDelay(uint32_t Cnt)
{
	vTaskDelay(Cnt);
}


uint32_t SysemMipsPercent;
#ifndef use_MCPS_ANALYSIS
static uint32_t UlRunTimeCounterOffset=0;
static uint32_t UlTotalTimeOffset=0;
#define StatsPeriod (60*1000)//ms
#endif
void vApplicationIdleHook(void)
{
#ifndef use_MCPS_ANALYSIS
	TaskStatus_t *pxTaskStatusArray;
	volatile UBaseType_t uxArraySize, x;
	uint32_t ulTotalTime, ulStatsAsPercentage;

	uxArraySize = uxTaskGetNumberOfTasks();

	pxTaskStatusArray = pvPortMalloc( uxTaskGetNumberOfTasks() * sizeof( TaskStatus_t ) );

	if( pxTaskStatusArray != NULL )
	{
		uxArraySize = uxTaskGetSystemState( pxTaskStatusArray, uxArraySize, &ulTotalTime );

		ulTotalTime /= 100UL;

		if( ulTotalTime > 0 )
		{
			for( x = 0; x < uxArraySize; x++ )
			{
				ulStatsAsPercentage = pxTaskStatusArray[ x ].ulRunTimeCounter / ulTotalTime;
				if(( ulStatsAsPercentage > 0UL ))// && (pxTaskStatusArray[ x ].pcTaskName == 'IDLE'))
				{
					char *P_T = (char *)pxTaskStatusArray[ x ].pcTaskName;
					if((P_T[0] == 'I') && (P_T[1] == 'D') && (P_T[2] == 'L') && (P_T[3] == 'E'))
					{
						if(SysemMipsPercent==0)
						{
							SysemMipsPercent = ulStatsAsPercentage;
						}

						if(ulTotalTime-UlTotalTimeOffset>0)
						{
							ulStatsAsPercentage = (pxTaskStatusArray[ x ].ulRunTimeCounter-UlRunTimeCounterOffset) / (ulTotalTime-UlTotalTimeOffset);
							SysemMipsPercent = ulStatsAsPercentage;
							if(ulTotalTime-UlTotalTimeOffset>(StatsPeriod/100))
							{
								UlRunTimeCounterOffset=pxTaskStatusArray[ x ].ulRunTimeCounter;
								UlTotalTimeOffset=ulTotalTime;
							}
						}
						break;
					}
				}
			}
		}
	}

	vPortFree( pxTaskStatusArray );
#endif
	
#ifdef CFG_GOTO_SLEEP_USE
	//#include <nds32_intrinsic.h>
		if(__nds32__mfsr(NDS32_SR_INT_PEND2))
		{
			__nds32__mtsr(__nds32__mfsr(NDS32_SR_INT_PEND2), NDS32_SR_INT_PEND2);
		}
		__nds32__standby_no_wake_grant();
#endif
}


#ifdef use_MCPS_ANALYSIS
//#include <nds32_intrinsic.h>
#if 0
typedef struct _TASK_COUT_LOG
{
	uint32_t cout;
	uint8_t name[8];
} TASK_COUT_LOG;

#define TASK_COUT_LOG_NUM  100

TASK_COUT_LOG TaskCountLog[TASK_COUT_LOG_NUM];
uint32_t LogCont=0;
#endif

#define StatsTime 1 //不能超过10
static uint32_t IdleTaskCycle=0, AllTaskCycle=0;
void trace_TASK_SWITCHED_IN(void)
{
	__nds32__mtsr(0, NDS32_SR_PFMC0);
	__nds32__mtsr(1, NDS32_SR_PFM_CTL);
#ifdef OS_INT_TOGGLE
	DbgTaskTGL_set();
#endif
}

void trace_TASK_SWITCHED_OUT(void)
{
	uint32_t temp;
#if 0
	if(LogCont >= TASK_COUT_LOG_NUM)
		LogCont = 0;

	__nds32__mtsr(0, NDS32_SR_PFM_CTL);
	TaskCountLog[LogCont].cout = __nds32__mfsr(NDS32_SR_PFMC0);
	memcpy(&TaskCountLog[LogCont].name[0], (void *)pcTaskGetTaskName(NULL), 7);
	TaskCountLog[LogCont].name[7] = 0;
	LogCont++;
#else
	__nds32__mtsr(0, NDS32_SR_PFM_CTL);
	temp = __nds32__mfsr(NDS32_SR_PFMC0);
	if(memcmp(pcTaskGetTaskName(NULL), "IDLE", 4) == 0)
	{
		IdleTaskCycle += temp;
	}
	AllTaskCycle += temp;
	if(AllTaskCycle>300*1000*1000*StatsTime)
	{
		SysemMipsPercent = (unsigned)((float)IdleTaskCycle*10000.0f/(float)AllTaskCycle);
		IdleTaskCycle=0;
		AllTaskCycle=0;
	}
#endif
#ifdef OS_INT_TOGGLE
	DbgTaskTGL_clr();
#endif
}
#endif
