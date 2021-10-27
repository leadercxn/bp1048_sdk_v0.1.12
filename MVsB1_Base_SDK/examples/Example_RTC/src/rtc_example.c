/**
 **************************************************************************************
 * @file    rtc_example.c
 * @brief   rtc example
 *
 * @author  Tony
 * @version V1.0.1
 *
 * $Created: 2019-08-08 11:25:00$
 *
 * @Copyright (C) 2019, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <stdlib.h>
#include <nds32_intrinsic.h>
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
#include "rtc.h"
#include "irqn.h"
#include "chip_info.h"

#define FREQ_FIXED	0x16

uint32_t SecLast = 0;
uint32_t time = 0;
uint32_t alarm =0;

bool alarmFlag = FALSE;

void SysTickInit(void);

void RC32K_Example(void)
{
	//RC_32K时钟不准确，不推荐使用
	RTC_ClockSrcSel(RC_32K);
	RTC_IntDisable();
	RTC_IntFlagClear();
	RTC_WakeupDisable();

	time = 1;
	RTC_SecSet(time);
	alarm = 10;
	RTC_SecAlarmSet(alarm);

	RTC_IntEnable();
	NVIC_DisableIRQ(Rtc_IRQn);
	while(1)
	{
		time = RTC_SecGet();
		if(SecLast != time)
		{
			SecLast = time;
			DBG("rtc:%lds\n",time);
			if(RTC_IntFlagGet() == TRUE)
			{
				DBG("there is an alarm(polling mode)\n");
				RTC_IntFlagClear();
				alarm = time+10;
				RTC_SecAlarmSet(alarm);
			}
		}
	}
}

void OSC32K_Example(void)
{
	RTC_ClockSrcSel(OSC_32K);
	RTC_IntDisable();
	RTC_IntFlagClear();
	RTC_WakeupDisable();

	time = 1;
	RTC_SecSet(time);
	alarm = 10;
	RTC_SecAlarmSet(alarm);

	RTC_IntEnable();
	NVIC_EnableIRQ(Rtc_IRQn);
	GIE_ENABLE();
	while(1)
	{
		time = RTC_SecGet();
		if(SecLast != time)
		{
			SecLast = time;
			DBG("rtc:%lds\n",time);

			if(TRUE == alarmFlag)
			{
				alarmFlag = FALSE;
				DBG("there is an alarm(int mode)\n");
				alarm = time+10;
				RTC_SecAlarmSet(alarm);
			}
		}
	}
}

void OSC24M_Example(void)
{
	RTC_ClockSrcSel(OSC_24M);
	RTC_IntDisable();
	RTC_IntFlagClear();
	RTC_WakeupDisable();

	time = 1;
	RTC_SecSet(time);
	alarm = 10;
	RTC_SecAlarmSet(alarm);

	RTC_IntEnable();
	NVIC_EnableIRQ(Rtc_IRQn);
	GIE_ENABLE();

	while(1)
	{
		time = RTC_SecGet();
		if(SecLast != time)
		{
			SecLast = time;
			DBG("rtc:%lds\n",time);

			if(TRUE == alarmFlag)
			{
				alarmFlag = FALSE;
				DBG("there is an alarm(int mode)\n");
				alarm = time+10;
				RTC_SecAlarmSet(alarm);
			}
		}
	}
}

int main(void)
{
	uint8_t  recvBuf;

	Chip_Init(1);
	WDG_Disable();

	Clock_Module1Enable(USB_UART_PLL_CLK_EN|UART1_CLK_EN|FLASH_CONTROL_PLL_CLK_EN);
	Clock_Module3Enable(UART1_REG_CLK_EN);

//	Clock_HOSCCapSet(FREQ_FIXED,FREQ_FIXED);//freq fix

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
    DBG("/-----------------------------------------------------\\\n");
    DBG("|                    RTC Example                    |\n");
    DBG("|      Mountain View Silicon Technology Co.,Ltd.      |\n");
    DBG("\\-----------------------------------------------------/\n\n");
	DBG("-------------------------------------------------------\n");
	DBG("1:  Enter RC_32K Example\n");
	DBG("2:  Enter OSC_32K Example\n");
	DBG("3:  Enter OSC_12M Example\n");
	DBG("-------------------------------------------------------\n");
    GIE_ENABLE();	//开启总中断

    while(1)
    {
		recvBuf = 0;
		UARTS_Recv(1, &recvBuf, 1,	10);

		switch(recvBuf)
		{
			case '1':
				DBG(">>>>>>>>>>>>>>>>>>RC32K_Example<<<<<<<<<<<<<<<<<<<<<<<\n\n");
				RC32K_Example();//RC_32K时钟不准确，不推荐使用
			  break;

			case '2':
				DBG(">>>>>>>>>>>>>>>>>>OSC32K_Example<<<<<<<<<<<<<<<<<<<<<<<\n\n");
				OSC32K_Example();
			  break;

			case '3':
				DBG(">>>>>>>>>>>>>>>>>>OSC12M_Example<<<<<<<<<<<<<<<<<<<<<<<\n\n");
				OSC24M_Example();
				break;

			default:
				break;
		}

    }


	return 0;
}

__attribute__((section(".driver.isr")))void RtcInterrupt(void)//RTC唤醒 并不会进入RTC中断
{
	if(RTC_IntFlagGet() == TRUE)
	{
		alarmFlag = TRUE;
		RTC_IntFlagClear();
	}
}


