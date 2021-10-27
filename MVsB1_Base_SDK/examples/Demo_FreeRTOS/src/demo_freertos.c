/**
 **************************************************************************************
 * @file    freertos_example.c
 * @brief   freertos example
 *
 * @author  Peter
 * @version V1.0.0
 *
 * $Created: 2019-05-30 11:30:00$
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
#include "adc.h"
#include "watchdog.h"
#include "spi_flash.h"
#include "remap.h"
#include "irqn.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "delay.h"
#include "chip_info.h"

xQueueHandle xQueue;

uint32_t SendCount = 0;
uint32_t RecvCount = 0;
uint32_t result[100];

void SendTask(void)
{
	while(1){
	DelayMs(500);
	SendCount++;
	DBG("SendMsg:0x%x\n",(unsigned int)SendCount);
	xQueueSend( xQueue, &SendCount, portMAX_DELAY );
	}
}

void RecvTask(void)
{
	uint32_t temp;
	while(1){
	 xQueueReceive( xQueue, &temp, portMAX_DELAY );
	 DBG("RecvMsg:0x%x\n\n",(unsigned int)temp);
	 result[RecvCount++] = temp;
	 if(RecvCount == 100)
		 RecvCount = 0;
	}
}
void prvInitialiseHeap(void);
int main(void)
{
	Chip_Init(1);
	WDG_Disable();

	Clock_Module1Enable(ALL_MODULE1_CLK_SWITCH);
	Clock_Module2Enable(ALL_MODULE2_CLK_SWITCH);
	Clock_Module3Enable(ALL_MODULE3_CLK_SWITCH);

	Clock_Config(1, 24000000);
	Clock_PllLock(288000);
	Clock_SysClkSelect(PLL_CLK_MODE);
	Clock_UARTClkSelect(PLL_CLK_MODE);

	GPIO_PortAModeSet(GPIOA24, 1);//Rx, A24:uart1_rxd_0
	GPIO_PortAModeSet(GPIOA25, 6);//Tx, A25:uart1_txd_0
	DbgUartInit(1, 256000, 8, 0, 1);

	Remap_InitTcm(0, 12);
	SpiFlashInit(80000000, MODE_4BIT, 0, 1);

	DBG("****************************************************************\n");
	DBG("                FreeRTOS Example                         \n");
	DBG("****************************************************************\n");

	prvInitialiseHeap();

	NVIC_EnableIRQ(SWI_IRQn);

	xQueue = xQueueCreate( 4, sizeof(uint32_t) );

	xTaskCreate( (TaskFunction_t)SendTask, "SendMsgTask", 512, NULL, 1, NULL );

	xTaskCreate( (TaskFunction_t)RecvTask, "RecvMsgTask", 512, NULL, 2, NULL );

	vTaskStartScheduler();

	while(1);
}


