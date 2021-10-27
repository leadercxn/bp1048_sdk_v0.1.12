/*
 * RTOS_API.c
 *
 *  Created on: Aug 30, 2016
 *      Author: peter
 */
#include <stdint.h>
#include <stddef.h>
#include "type.h"
#include "rtos_api.h"
#include "remap.h"
#include "task.h"
#ifdef CFG_APP_CONFIG
#include "app_config.h"
#endif
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
	portBASE_TYPE taskWoken = pdFALSE;

	if(msgHandle == NULL)
	{
		return  FALSE;
	}
	if (inHandlerMode())
	{
		if (xQueueSendFromISR(msgHandle, msgContext, &taskWoken) != pdTRUE)
		{
			return FALSE;
		}
		portEND_SWITCHING_ISR(taskWoken);
	}
	else
	{
		if (xQueueSend(msgHandle, msgContext, 0) != pdTRUE)
		{
			return FALSE;
		}
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
	portBASE_TYPE	taskWoken;
	bool			ret = FALSE;

	if (msgHandle == NULL)
	{
		return FALSE;
	}

	taskWoken = pdFALSE;
	msgContext->msgId = MSG_INVAILD;

	if (inHandlerMode())
	{
		if (xQueueReceiveFromISR(msgHandle, msgContext, &taskWoken) == pdTRUE)
		{
			/* We have mail */
			ret = TRUE;
		}
		else
		{
			ret = FALSE;
		}
		portEND_SWITCHING_ISR(taskWoken);
	}
	else 
	{
		if (xQueueReceive(msgHandle, msgContext, millisec) == pdTRUE)
		{
			/* We have mail */
			ret = TRUE;
		}
		else 
		{
			ret = FALSE;
		}
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
#endif
}

/**
* @brief Allocate a memory block from a memory pool
* @param  osWantedSize Allocate memory word(1word=4byte) size
* @retval  address of the allocated memory block or NULL in case of no memory available.
*/
void *osPortMalloc(uint16_t osWantedSize)
{
	void *ospvReturn = NULL;
	uint32_t *lp_p,lp_v;


	lp_p = 0x20003020;
	__asm("pushm $r0,$r7");
	__asm("sethi $r0,#0x20003");
	__asm("swi $lp,[$r0+#0x20]");  //0x20003020
	__asm("popm $r0,$r7");

	lp_v = *lp_p;


	vPortEnterCritical();
	ospvReturn=pvPortMalloc(osWantedSize);
	vPortExitCritical();
	//DBG("Malloc11:%d->%d, Add=%x text:%08lx\n", (int)osWantedSize, (int)xPortGetFreeHeapSize(), ospvReturn,lp_v);
	return ospvReturn;
}
void *pvPortMallocFromEnd( size_t xWantedSize );
/**
* @brief Allocate a memory block from a memory pool
* @param  osWantedSize Allocate memory word(1word=4byte) size
* @retval  address of the allocated memory block or NULL in case of no memory available.
*/
void *osPortMallocFromEnd(uint32_t osWantedSize)
{
	void *ospvReturn = NULL;
	uint32_t *lp_p,lp_v;


	lp_p = 0x20003020;
	__asm("pushm $r0,$r7");
	__asm("sethi $r0,#0x20003");
	__asm("swi $lp,[$r0+#0x20]");  //0x20003020
	__asm("popm $r0,$r7");

	lp_v = *lp_p;

	//DBG("\nMalloc22:%d, %d\n", xPortGetFreeHeapSize(), osWantedSize);
	ospvReturn = pvPortMallocFromEnd(osWantedSize);
	DBG("Malloc33:%d->%d, Add=%x ,text:%08x\n", (int)osWantedSize, (int)xPortGetFreeHeapSize(), ospvReturn,lp_v);
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
	DBG("free111=%x\n", ospv);
	vPortEnterCritical();
	vPortFree(ospv);
	vPortExitCritical();
}

void osTaskDelay(uint32_t Cnt)
{
	vTaskDelay(Cnt);
}

//PRIVILEGED_DATA static volatile UBaseType_t uxCurrentNumberOfTasks 	= ( UBaseType_t ) 0U;

uint32_t SysemMipsPercent;
static uint32_t UlRunTimeCounterOffset=0;
static uint32_t UlTotalTimeOffset=0;
#define StatsPeriod (60*1000)//ms
void vApplicationIdleHook(void)
{
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
typedef struct _TASK_COUT_LOG
{
	uint32_t cout;
	uint8_t name[8];
} TASK_COUT_LOG;

#define TASK_COUT_LOG_NUM  100

TASK_COUT_LOG TaskCountLog[TASK_COUT_LOG_NUM];
uint32_t LogCont=0;

void trace_TASK_SWITCHED_IN(void)
{
	__nds32__mtsr(0, NDS32_SR_PFMC0);
	__nds32__mtsr(1, NDS32_SR_PFM_CTL);
}

void trace_TASK_SWITCHED_OUT(void)
{
	if(LogCont >= TASK_COUT_LOG_NUM)
		LogCont = 0;

	__nds32__mtsr(0, NDS32_SR_PFM_CTL);
	TaskCountLog[LogCont].cout = __nds32__mfsr(NDS32_SR_PFMC0);
	memcpy(&TaskCountLog[LogCont].name[0], (void *)pcTaskGetTaskName(NULL), 7);
	TaskCountLog[LogCont].name[7] = 0;
	LogCont++;
}

void DisplayMcpsInfo(void)
{
	uint32_t i;
	for(i=0;  i<TASK_COUT_LOG_NUM; i++)
	{
		GIE_DISABLE();
		printf("%08s  %d\n", &TaskCountLog[i].name[0], TaskCountLog[i].cout);
		GIE_ENABLE();
	}
}
uint32_t check_btint_stack_size();

typedef struct debug_malloc_t
{
	uint32_t debug_addr;
	uint32_t debug_size;
	void* lp_p;
}debug_malloc_t;

debug_malloc_t *get_debug_info;
uint8_t task_list[1024];
void get_debug_intfo()
{
	int i = 0;
	extern char _end;

	get_debug_info = (debug_malloc_t*)getdebug_p();
	printf("********free:%ld, %ld,eram:%08lx***********\r\n",xPortGetFreeHeapSize(),head_szie(),&_end);
	for(i=0; i < getdebug_cnt() ;i++)
	{

		if(get_debug_info)
		{

			printf("malloc:%ld,%08lx, %ld , %08lx\r\n",
					get_debug_info->debug_addr,get_debug_info->debug_addr,get_debug_info->debug_size,get_debug_info->lp_p);

			get_debug_info++;

		}
	}
	printf("**********************\r\n");
	printf("----------------------\r\n");
	vTaskList(task_list);
	printf("%s\r\n",task_list);
	printf("**********************\r\n");
}

uint32_t getstart  = 0;


#define INT_SP_ADDR 	0x20003904
#define SIZE_SP			512


void RTOS_RAM_PRO()
{
	GPIOAX_setclr(1,2);
	GPIOAX_setclr(0,2);
	if( (GetSysTick1MsCnt() - getstart) > 5000)
	{
		getstart = GetSysTick1MsCnt();
		get_debug_intfo();

		//配合test_TTTT_init
		{
			uint32_t addr_1 = check_btint_stack_size();
			printf("intsize:addr[0x%08lx],size:%ld\n\n",addr_1,((INT_SP_ADDR + 512 - 4) - addr_1));
		}
	}
}

void test_TTTT_init()
{
	uint8_t* addr = NULL;

	addr = (uint8_t*)INT_SP_ADDR;
	memset(addr,0xaa,(SIZE_SP) );
}

void test_TTTT_in()
{
	__asm("pushm $r0,$r7");
	__asm("sethi $r0,#0x20003");
	__asm("addi $r0,$r0,#0x904");//0x20003904

	__asm("swi $sp,[$r0]");  //0x20003904
	__asm("sethi $sp,#0x20003");  //0x20003B04
	__asm("addi $sp,$sp,#0xB00");


}

void test_TTTT_out()
{
	__asm("sethi $r0,#0x20003");
	__asm("addi $r0,$r0,#0x904");
	__asm("lwi $sp,[$r0]");  //0x20003020
	__asm("popm $r0,$r7");
}

uint32_t check_btint_stack_size()//retrun addr
{
	uint32_t *addr;
	uint32_t i = 0;

	addr = (uint32_t*)INT_SP_ADDR+4;
	for(i=0;i<(SIZE_SP/4);i++)
	{
		if(*addr == 0xAAAAAAAA)
		{
			addr += 1;
		}
		else
		{
			return (uint32_t)addr;
		}
	}

	return 0;
}

void GPIOAX_setclr(uint8_t set,uint8_t GPIOX)
{
	if(set)
	{
		GPIO_PortAModeSet(GPIOA0 << GPIOX, 0);
		GPIO_RegOneBitSet(GPIO_A_OE,GPIOA0 << GPIOX);
		GPIO_RegOneBitClear(GPIO_A_IE,GPIOA0 << GPIOX);
		GPIO_RegOneBitSet(GPIO_A_OUT,GPIOA0 << GPIOX);
	}
	else
	{
		GPIO_PortAModeSet(GPIOA0 << GPIOX, 0);
		GPIO_RegOneBitSet(GPIO_A_OE,GPIOA0 << GPIOX);
		GPIO_RegOneBitClear(GPIO_A_IE,GPIOA0 << GPIOX);
		GPIO_RegOneBitClear(GPIO_A_OUT,GPIOA0 << GPIOX);
	}
}
#define TASK_LIST_SIZE	4
uint8_t tasklist[TASK_LIST_SIZE][20] = {
		{"MainApp"},
		{"dump_task"},
		{"BtStackServiceStart"},
		{"IDLE"},
};

void tdbgTaskTGL_set()
{
	int i = 0;

	for(i=0;i<TASK_LIST_SIZE;i++)
	{
		if(memcmp( pcTaskGetTaskName(NULL),tasklist[i],strlen(tasklist[i])) == 0 )
		{
			GPIOAX_setclr(1,i);
		}
	}
}
void tdbgTaskTGL_clr()
{
	int i = 0;

	for(i=0;i<TASK_LIST_SIZE;i++)
	{
		if(memcmp( pcTaskGetTaskName(NULL),tasklist[i],strlen(tasklist[i])) == 0 )
		{
			GPIOAX_setclr(0,i);
		}
	}
}

void OS_dbg_int_in(uint32_t int_num)
{
	if(int_num == 18)
	{
		GPIOAX_setclr(1,9);//bt int
	}
	else if(int_num != 0)
	{
		GPIOAX_setclr(1,10);//other int
	}
}

void OS_dbg_int_out(uint32_t int_num)
{
	if(int_num == 18)
	{

		GPIOAX_setclr(0,9);
	}
	else if(int_num != 0)
	{
		GPIOAX_setclr(0,10);
	}
}


void OS_int5int(void)
{
	GPIOAX_setclr(1,12);
}

void OS_int5out(void)
{
	GPIOAX_setclr(0,12);
}

#endif
