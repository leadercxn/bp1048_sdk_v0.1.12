/**
 **************************************************************************************
 * @file    backup_example.c
 * @brief   backup example
 *
 * @author  Tony
 * @version V1.0.0
 *
 * $Created: 2019-10-30 13:25:00$
 *
 * @Copyright (C) 2019, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <stdlib.h>
#include <nds32_intrinsic.h>
//#include <string.h>
#include "gpio.h"
#include "uarts.h"
#include "uarts_interface.h"
#include "type.h"
#include "debug.h"
#include "timeout.h"
#include "clk.h"
#include "dma.h"
#include "timer.h"
#include "watchdog.h"
#include "spi_flash.h"
#include "remap.h"
#include "backup.h"
#include "chip_info.h"
#include "rtc.h"
#include "irqn.h"
#include "backup_interface.h"

//软开关状态下，可以使用RTC作为wakeup唤醒源
//demo中启用如下宏时，系统PowerDown之后5S会自动唤醒
#define WAKEUP_BY_RTC
#ifdef	WAKEUP_BY_RTC
#define SET_POWERUP_TIMESEC		5
#endif

extern void SysTickInit(void);

#ifdef	WAKEUP_BY_RTC

uint32_t time = 0;

void BACKUP_RTCSet(uint32_t rtc_time_set,uint32_t wakeup_s)
{
	RTC_ClockSrcSel(OSC_32K);
	RTC_IntDisable();
	RTC_IntFlagClear();
	RTC_WakeupDisable();

	time = rtc_time_set;
	RTC_SecSet(time);
	time += wakeup_s;
	RTC_SecAlarmSet(time);

	RTC_WakeupEnable();
	RTC_IntEnable();
}

#endif


void NvmTestExample(void)
{
	uint8_t Nvm_Set_Data[16] = {0};
	uint8_t Nvm_Get_Data[17] = {0};

	int i = 0;

	for(i=0;i<16;i++)
	{
		Nvm_Set_Data[i] = 0x30 + i;
	}

	BACKUP_NVMInit();
	BACKUP_NvmWrite(4,Nvm_Set_Data,10);
	BACKUP_NvmRead(5,Nvm_Get_Data,11);

	printf("data:");
	for(i=0;i<16;i++)
	{
		printf("0x%02x,",Nvm_Get_Data[i]);
	}
	printf("\r\n");
}


int main(void)
{
	Chip_Init(1);
	WDG_Disable();

	Clock_Module1Enable(USB_UART_PLL_CLK_EN|UART1_CLK_EN|FLASH_CONTROL_PLL_CLK_EN);
	Clock_Module3Enable(UART1_REG_CLK_EN);

	Clock_Config(1, 24000000);
	Clock_PllLock(288000);
	Clock_APllLock(240000);

	Clock_SysClkSelect(PLL_CLK_MODE);
	Clock_UARTClkSelect(PLL_CLK_MODE);

	Remap_InitTcm(FLASH_ADDR, TCM_SIZE);
	SpiFlashInit(80000000, MODE_4BIT, 0, 1);


	GPIO_PortAModeSet(GPIOA24, 1);//Rx, A24:uart1_rxd_0
	GPIO_PortAModeSet(GPIOA25, 6);//Tx, A25:uart1_txd_0
	DbgUartInit(1, 256000, 8, 0, 1);

	SysTickInit();
	DBG("\n");
    DBG( "/-----------------------------------------------------\\\n");
    DBG( "|                    Backup Example                   |\n");
    DBG( "|      Mountain View Silicon Technology Co.,Ltd.      |\n");
    DBG("\\-----------------------------------------------------/\n");
    DBG("\n");

	
	Backup_Clock(BACKUP_CLK_32K_RC32);	//default RC_32K & OSC_32K is enable.
										//you can use this function open HOSC ---- Clock_Config(1, X000000);

	while(!BACKUP_IsOscClkToggle());	//wait OSCclk ready (clk TGL).
	while(BACKUP_IsOscClkToggle());

	DBG("backup clk isc ok!/\n");

	BACKUP_VBK22KMode();		//if use adc key please use this function.

	SystemPowerKeyInit(POWERKEY_MODE_PUSH_BUTTON, POWERKEY_CNT);
	PowerKeyModeGet();

	NvmTestExample();

	while(1)
	{
		if(SystemPowerKeyDetect())
		{
#ifdef	WAKEUP_BY_RTC
			if(BACKUP_GetConfig() == POWERKEY_MODE_PUSH_BUTTON)
			{
				BACKUP_RTCSet(0,SET_POWERUP_TIMESEC);
			}
#endif
			SystemPowerDown();
		}
		//DBG("*");
	}
}

