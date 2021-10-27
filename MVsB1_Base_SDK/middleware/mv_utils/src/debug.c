
#include <debug.h>
#include "type.h"
#include "FreeRTOSConfig.h"


uint8_t DBG_Global(char * str,char **fmt, ...)
{
	if('^'==**fmt)
	{
		*fmt += 1;
		return TRUE;
	}
	if(0!=(strstr(str,"device/")))
#ifdef	DEVICE_MODULE_DEBUG
		printf("[DEVICE]:");
#else
		return FALSE;
#endif
	else if(0!=(strstr(str,"services/")))
#ifdef SERVICE_MODULE_DEBUG
		printf("[SERVICE]:");
#else
		return FALSE;
#endif
	else if(0!=(strstr(str,"apps/")))
	{
		if(0!=(strstr(str,"media")))
#ifdef	MEDIA_MODULE_DEBUG
			printf("[APP_MEDIA]:");
#else
				return FALSE;
#endif
		else if(0!=(strstr(str,"bt")))
#ifdef	BT_MODULE_DEBUG
			printf("[APP_BT]:");
#else
				return FALSE;
#endif
		else if(0!=(strstr(str,"main_task")))
#ifdef	MAINTSK_MODULE_DEBUG
			printf("[APP_MAIN]:");
#else
				return FALSE;
#endif
		else if(0!=(strstr(str,"usb_audio")))
#ifdef	USBAUDIO_MODULE_DEBUG
			printf("[APP_USBAUDIO]:");
#else
				return FALSE;
#endif
		else if(0!=(strstr(str,"waiting")))
#ifdef	WAITING_MODULE_DEBUG
			printf("[APP_WAITING]:");
#else
			return FALSE;
#endif
	}

	return TRUE;
}

#ifdef OS_INT_TOGGLE
const uint8_t DbgTaskList[][configMAX_TASK_NAME_LEN] = DBG_TASK_LIST;
#define TGL_TASK_NUM	(sizeof(DbgTaskList)/(configMAX_TASK_NAME_LEN))

void DbgTaskTGL_set()
{
	int i = 0;

	for(i=0;i<TGL_TASK_NUM;i++)
	{
		if(memcmp( pcTaskGetTaskName(NULL),DbgTaskList[i],strlen(DbgTaskList[i])) == 0 )
		{
			LedOn(i);
		}
	}
}
void DbgTaskTGL_clr()
{
	int i = 0;

	for(i=0;i<TGL_TASK_NUM;i++)
	{
		if(memcmp( pcTaskGetTaskName(NULL),DbgTaskList[i],strlen(DbgTaskList[i])) == 0 )
		{
			LedOff(i);
		}
	}
}


void OS_dbg_int_in(uint32_t int_num)
{
	if(int_num == DBG_INT_ID)
	{
		LedOn(TGL_TASK_NUM);
	}
	else if(int_num == 8)
	{
		LedOn(TGL_TASK_NUM + 1);
	}
	else if(int_num != 0) //INT0 os
	{
		LedOn(TGL_TASK_NUM + 2);
	}
}

void OS_dbg_int_out(uint32_t int_num)
{
	if(int_num == DBG_INT_ID)
	{
		LedOff(TGL_TASK_NUM);
	}
	else if(int_num == 8)
	{
		LedOff(TGL_TASK_NUM + 1);
	}
	else if(int_num != 0)//INT0 os
	{
		LedOff(TGL_TASK_NUM + 2);
	}
}
#endif //OS_INT_TOGGLE

#ifdef LED_IO_TOGGLE
typedef struct _DBG_IO_ID
{
	char		PortBank;
	uint8_t		IOIndex;

} DBG_IO_ID;
const DBG_IO_ID DbgLedList[] = LED_PORT_LIST;
#define DBG_LED_NUM (sizeof(DbgLedList) / sizeof(DBG_IO_ID))


void LedPortInit(void)
{
	uint8_t i;
	APP_DBG("**********LED:%d,%d***************\n", sizeof(DbgLedList), DBG_LED_NUM);
	for(i = 0; i < DBG_LED_NUM; i++)
	{
		if((DbgLedList[i].PortBank == 'A' && DbgLedList[i].IOIndex >= 31) || (DbgLedList[i].PortBank == 'B' && DbgLedList[i].IOIndex >= 7))
		{
			APP_DBG("%d Port Error!", i);
			continue;
		}
		LedOff(i);
		GPIO_RegOneBitSet(PORT_OE_REG(DbgLedList[i].PortBank), BIT(DbgLedList[i].IOIndex));
		GPIO_RegOneBitClear(PORT_IE_REG(DbgLedList[i].PortBank), BIT(DbgLedList[i].IOIndex));
	}
}


void LedOn(uint8_t Index)
{
	if(Index >= DBG_LED_NUM)
		return;
#if !LED_ON_LEVEL
	GPIO_RegOneBitClear(PORT_OUT_REG(DbgLedList[Index].PortBank), BIT(DbgLedList[Index].IOIndex));
#else
	GPIO_RegOneBitSet(PORT_OUT_REG(DbgLedList[Index].PortBank), BIT(DbgLedList[Index].IOIndex));
#endif
}

void LedOff(uint8_t Index)
{
	if(Index >= DBG_LED_NUM)
		return;
#if LED_ON_LEVEL
	GPIO_RegOneBitClear(PORT_OUT_REG(DbgLedList[Index].PortBank), BIT(DbgLedList[Index].IOIndex));
#else
	GPIO_RegOneBitSet(PORT_OUT_REG(DbgLedList[Index].PortBank), BIT(DbgLedList[Index].IOIndex));
#endif
}

void LedToggle(uint8_t Index)
{
	if(Index >= DBG_LED_NUM)
		return;
	if(GPIO_RegOneBitGet(PORT_OUT_REG(DbgLedList[Index].PortBank), BIT(DbgLedList[Index].IOIndex)))
	{
		GPIO_RegOneBitClear(PORT_OUT_REG(DbgLedList[Index].PortBank), BIT(DbgLedList[Index].IOIndex));
	}
	else
	{
		GPIO_RegOneBitSet(PORT_OUT_REG(DbgLedList[Index].PortBank), BIT(DbgLedList[Index].IOIndex));
	}
}

void LedPortRise(uint8_t Index)
{
	if(Index >= DBG_LED_NUM)
		return;
	if(!GPIO_RegOneBitGet(PORT_OUT_REG(DbgLedList[Index].PortBank), BIT(DbgLedList[Index].IOIndex)))
	{
		GPIO_RegOneBitSet(PORT_OUT_REG(DbgLedList[Index].PortBank), BIT(DbgLedList[Index].IOIndex));
	}

}

void LedPortDown(uint8_t Index)
{
	if(Index >= DBG_LED_NUM)
		return;
	if(GPIO_RegOneBitGet(PORT_OUT_REG(DbgLedList[Index].PortBank), BIT(DbgLedList[Index].IOIndex)))
	{
		GPIO_RegOneBitClear(PORT_OUT_REG(DbgLedList[Index].PortBank), BIT(DbgLedList[Index].IOIndex));
	}
}

bool LedPortGet(uint8_t Index)
{
	if(Index >= DBG_LED_NUM)
		return FALSE;
	return GPIO_RegOneBitGet(PORT_OUT_REG(DbgLedList[Index].PortBank), BIT(DbgLedList[Index].IOIndex));
}

#endif //LED_IO_TOGGLE
