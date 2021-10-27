/**
 **************************************************************************************
 * @file    clock_example.c
 * @brief   clock example
 *
 * @author  Sam
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
#include "pwm.h"
#include "chip_info.h"


//ʱ����ʾ���̣���Ҫ��ʾϵͳʱ��ѡ��ο�Դ
//	1: 24M��������DPLL����288M���ں�ʱ��ΪPLLʱ�ӣ�ϵͳʱ��PLL 2��ƵΪ144M��
//	2: 24M��������APLL����240M���ں�ʱ��ΪPLLʱ�ӣ�ϵͳʱ��PLL 2��ƵΪ120M��
//	3��RC12M��ϵͳ������RCʱ����
//��������ʾ���ⲿ��������ʱ��Ҫ����ؾ���
//��PWM 10��Ƶ�� GPIOA0��ʾ�����۲�
//ϵͳĬ��ʹ��24M������DPLL288M,APLL240M
int main(void)
{
	uint8_t Key = 0;
	PWM_StructInit PWMParam;
	//Chip_Init();
	WDG_Disable();

	Clock_Module1Enable(ALL_MODULE1_CLK_SWITCH);
	Clock_Module2Enable(ALL_MODULE2_CLK_SWITCH);
	Clock_Module3Enable(ALL_MODULE3_CLK_SWITCH);

	Clock_Config(1, 24000000);
	Clock_PllLock(288000);
	Clock_APllLock(240000);

	Clock_SysClkSelect(PLL_CLK_MODE);
	Clock_UARTClkSelect(PLL_CLK_MODE);
	Clock_Timer3ClkSelect(SYSTEM_CLK_MODE);

	Remap_InitTcm(FLASH_ADDR, TCM_SIZE);
	SpiFlashInit(80000000, MODE_4BIT, 0, 1);

	//GPIO_PortAModeSet(GPIOA9, 1);//Rx, A9:uart1_rxd_0
	//GPIO_PortAModeSet(GPIOA10, 3);//Tx,A10:uart1_txd_0
	GPIO_PortAModeSet(GPIOA24, 1);	//Rx, A24:uart1_rxd_0
	GPIO_PortAModeSet(GPIOA25, 6);	//Tx, A25:uart1_txd_0
	DbgUartInit(1, 256000, 8, 0, 1);

	DBG("\n");
    DBG("/-----------------------------------------------------\\\n");
    DBG("|                    Clock Example                    |\n");
    DBG("|      Mountain View Silicon Technology Co.,Ltd.      |\n");
    DBG("\\-----------------------------------------------------/\n");
    DBG("\n");

    DBG("Driver Version: %s\n", GetLibVersionDriver());

	//PWM ������ڹ۲⣬
	PWM_GpioConfig(TIMER3_PWM_A0_A8_A22_A27, 0, PWM_IO_MODE_OUT);
	DBG("PWM Init OUTPUT: A0\n");

	//PWM��������
	PWMParam.CounterMode   = PWM_COUNTER_MODE_DOWN;
	PWMParam.OutputType    = PWM_OUTPUT_SINGLE_1;
	PWMParam.FreqDiv       = 100;//PWMFreqTable[FreqIndex];
	PWMParam.Duty          = 50;//PWMDutyTable[DutyIndex];
	PWMParam.DMAReqEnable  = 0;
	PWM_Config(TIMER3, &PWMParam);
	PWM_Enable(TIMER3);

    while(1)
    {
    	if(UARTS_Recv(1, &Key, 1,100) > 0)
		{
			switch(Key)
			{
			case 'A':
			case 'a':
				Clock_Config(1, 24000000);
				Clock_PllLock(288000);
				Clock_SysClkSelect(PLL_CLK_MODE);
				Clock_UARTClkSelect(PLL_CLK_MODE);
				GPIO_PortAModeSet(GPIOA9, 1);//Rx, A9:uart1_rxd_0
				GPIO_PortAModeSet(GPIOA10, 3);//Tx,A10:uart1_txd_0
				DbgUartInit(1, 256000, 8, 0, 1);
				DBG("system work on DPLL 288M, Soc 24M\n");
				break;
			case 'B':
			case 'b':
				Clock_Config(1, 24000000);
				Clock_PllLock(240000);
				Clock_SysClkSelect(APLL_CLK_MODE);
				Clock_UARTClkSelect(APLL_CLK_MODE);
				GPIO_PortAModeSet(GPIOA9, 1);//Rx, A9:uart1_rxd_0
				GPIO_PortAModeSet(GPIOA10, 3);//Tx,A10:uart1_txd_0
				DbgUartInit(1, 256000, 8, 0, 1);
				DBG("system work on APLL 240M, Soc 24M\n");
				break;
			case 'C':
			case 'c':
				Clock_SysClkSelect(RC_CLK_MODE);
				Clock_UARTClkSelect(RC_CLK_MODE);
				GPIO_PortAModeSet(GPIOA9, 1);//Rx, A9:uart1_rxd_0
				GPIO_PortAModeSet(GPIOA10, 3);//Tx,A10:uart1_txd_0
				DbgUartInit(1, 256000, 8, 0, 1);
				DBG("system work on RC 12M\n");
				break;
			default:
				break;
			}
		}
    }
    DBG("Err!\n");
	while(1);
}

__attribute__((section(".driver.isr"))) void BT_Interrupt(void)
{
}

__attribute__((section(".driver.isr"))) void BLE_Interrupt(void)
{
}

__attribute__((section(".driver.isr"))) void IRInterrupt(void)
{
}
