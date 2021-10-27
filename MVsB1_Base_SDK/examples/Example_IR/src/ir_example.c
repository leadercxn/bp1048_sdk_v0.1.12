/**
 **************************************************************************************
 * @file    ir_example.c
 * @brief   ir example
 *
 * @author  Taowen
 * @version V1.0.0
 *
 * $Created: 2019-06-03 19:17:00$
 *
 * @Copyright (C) 2019, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include <stdlib.h>
#include <nds32_intrinsic.h>
#include <string.h>
#include "gpio.h"
#include "uarts.h"
#include "uarts_interface.h"
#include "type.h"
#include "debug.h"
#include "timeout.h"
#include "clk.h"
#include "dma.h"
#include "timer.h"
#include "spi_flash.h"
#include "remap.h"
#include "irqn.h"
#include "sys.h"
#include "chip_info.h"
#include "watchdog.h"
#include "reset.h"
#include <string.h>
#include "type.h"
#include "irqn.h"
#include "gpio.h"
#include "debug.h"
#include "timer.h"
#include "dma.h"
#include "uarts_interface.h"
#include "clk.h"
#include "watchdog.h"
#include "spi_flash.h"
#include "remap.h"
#include "chip_info.h"
#include "ir.h"
#include "delay.h"
#include "powercontroller.h"

//注意使用的UART端口
#define UART_TX_IO	1
#define UART_RX_IO  1

#define IR_NEC_KEY (0xba45ff00)//0xED127F80
#define IR_SONY_KEY (0x00000095)

void NEC_IR_RecieveCmdExample(void)
{
	bool CmdOK = FALSE;
	uint32_t Cmd = 0;
	uint8_t RepeatCount = 0;
	uint8_t Last_RepeatCount = 0xff;

	DBG("enter NEC IR---A29 Recieve Command Example\n\n");

	GPIO_RegOneBitSet(GPIO_A_IE, GPIO_INDEX29);
	GPIO_RegOneBitClear(GPIO_A_OE, GPIO_INDEX29);

	IR_Config(IR_MODE_NEC, IR_GPIOA29, IR_NEC_32BITS);
	IR_Enable();

	while(1)
	{
		CmdOK = IR_CommandFlagGet();
		if(CmdOK == TRUE)
		{
			DBG("\n查询方式\n");
			Cmd = IR_CommandDataGet();
			DBG("\n---IR_Cmd:%08lx\n",Cmd);
			IR_IntFlagClear();

			RepeatCount = IR_RepeatCountGet();
			DBG("\n---RepeatCount:%0x\n",RepeatCount);
			while(1)
			{
				DelayMs(120);//NEC IR repeat code is transmited every 110mS

				Last_RepeatCount = RepeatCount;

				RepeatCount = IR_RepeatCountGet();
				DBG("\n*********RepeatCount:%0x\n",RepeatCount);

				if( (Last_RepeatCount - RepeatCount) > 0 )
				{
					Cmd = IR_CommandDataGet();
					DBG("\n--------There is a New IR_Command:%08lx------\n",Cmd);
					DBG("\n*****************IR_Cmd:%08lx\n",Cmd);
					IR_IntFlagClear();
				}

				if(RepeatCount == Last_RepeatCount)
				{
					IR_CommandFlagClear();
					break;
				}
			}
		}
	}
}

void NEC_IR_InterruptExample(void)
{
	DBG("enter NEC IR---A29 Interrupt Example\n\n");

	GPIO_RegOneBitSet(GPIO_A_IE, GPIO_INDEX29);
	GPIO_RegOneBitClear(GPIO_A_OE, GPIO_INDEX29);
	IR_Config(IR_MODE_NEC, IR_GPIOA29, IR_NEC_32BITS);

	NVIC_EnableIRQ(IR_IRQn);
	NVIC_SetPriority(IR_IRQn, 1);
	GIE_ENABLE();

	IR_Enable();//开启IR功能
	IR_InterruptEnable();//开启IR中断
}

//#define SW_PORTB  1
uint32_t PllFreqGet  = 0;
uint32_t APllFreqGet  = 0;
CLK_MODE SystemClockSel = PLL_CLK_MODE;
uint32_t SystemClockDivGet = 0;
void SystermAfterWackup(void)
{
	Chip_Init(1);
	WDG_Disable();
 	Power_PowerModeConfigTest(FALSE, FALSE, FALSE);//B1X的HPM模式和LPM模式在功耗上没啥区别
	Clock_Module1Enable(ALL_MODULE1_CLK_SWITCH);
	Clock_Module2Enable(ALL_MODULE2_CLK_SWITCH);
	Clock_Module3Enable(ALL_MODULE3_CLK_SWITCH);

	Clock_Config(1, 24000000);
	Clock_PllLock(PllFreqGet/1000);
	Clock_DeepSleepSysClkSelect(PLL_CLK_MODE, FSHC_PLL_CLK_MODE, 0);
	Clock_SysClkSelect(SystemClockSel);
	Clock_SysClkDivSet(SystemClockDivGet);
	Clock_UARTClkSelect(RC_CLK_MODE);

	GPIO_PortAModeSet(GPIOA9, 1);//Rx, A9:uart1_rxd_0
	GPIO_PortAModeSet(GPIOA10, 3);//Tx,A10:uart1_txd_0
	DbgUartInit(1, 115200, 8, 0, 1);

	SysTickInit();

	SpiFlashInit(80000000, MODE_4BIT, 0, 1);

	#ifdef SW_PORTB  //若定义B口味SW下载口，就设置A口

		GPIO_PortBModeSet(GPIOB0, 0x1);////若前面关闭了GPIOB0、B1复用为SW下载调试口复用模式，请在此处开启 ;
		GPIO_PortBModeSet(GPIOB1, 0x1);
	 #else
	 	GPIO_PortAModeSet(GPIOA30, 0x5);//若前面关闭了GPIOA30、A31复用为SW下载调试口复用模式，请在此处开启 ;
	 	GPIO_PortAModeSet(GPIOA31, 0x4);

	 #endif

	    DBG("-------Wakeup from DeepSleep-------\n");
}
void DeepSleepPowerConsumption_Config(void);
void SystermIRWakeupConfig(IR_MODE_SEL ModeSel, IR_IO_SEL GpioSel, IR_CMD_LEN_SEL CMDLenSel);
void WakeupSourceGet(void);
void NEC_IR_WackupFromDeepSleepExample(void)
{
	 uint8_t Temp = 0;
	 uint32_t Cmd = 0;

	DBG("\n Input 'e' to enter SystemDeepSleepPowerConsumption_Example again\n");
	DBG("\n Input 'x' to quit SystemDeepSleepPowerConsumption_Example \n");

	 while(1)
	 {
		 UARTS_Recv(UART_RX_IO, &Temp, 1, 1000);

		 if(Temp == 'e')
		 {
				DBG("Wait 3s then enter DeepSleep\n");

				DelayMs(3000);

			 	//DeepSleepPowerConsumption Config
			 	DeepSleepPowerConsumption_Config();

			 	//Select Wakeup Source
			 	SystermIRWakeupConfig(IR_MODE_NEC, IR_GPIOB6, IR_NEC_32BITS);

			 	//Enter Deepsleep
			 	Power_GotoDeepSleep();

				//Wakeup from DeepSleep
				SystermAfterWackup();

				//Wakeup Source Get
				WakeupSourceGet();

				while(1)
				{
					//获取IR键值
					if((IR_CommandFlagGet() == TRUE))
					{
						Cmd = IR_CommandDataGet();
						DBG("\n---IR_Cmd:%08lx\n",Cmd);
						IR_IntFlagClear();
						IR_CommandFlagClear();

						if(IR_NEC_KEY == Cmd)
						{
							break;
						}
						else
						{
							DBG("IR_KEY  is wrong!!!,enter DeepSleep again!!!\n");
							//DeepSleepPowerConsumption Config
							DeepSleepPowerConsumption_Config();

							//Select Wakeup Source
							SystermIRWakeupConfig(IR_MODE_NEC, IR_GPIOB6, IR_NEC_32BITS);

							//Enter Deepsleep
							Power_GotoDeepSleep();

							//Wakeup from DeepSleep
							SystermAfterWackup();

							//Wakeup Source Get
							WakeupSourceGet();
						}
					}
				}

				DBG("\n Input 'e' to enter SystemDeepSleepPowerConsumption_Example again\n");
				DBG("\n Input 'x' to quit SystemDeepSleepPowerConsumption_Example \n");
		 }
		 else if(Temp == 'x')
		 {
			 DBG("\n  Quit SystemDeepSleepPowerConsumption_Example \n");
			 break;
		 }
	 }
}

void SONY_IR_RecieveCmdExample(void)
{
	bool CmdOK = FALSE;
	uint32_t Cmd = 0;
	uint8_t RepeatCount = 0;
	uint8_t Last_RepeatCount = 0xff;

	DBG("enter NEC IR---A29 Recieve Command Example\n\n");

	GPIO_RegOneBitSet(GPIO_A_IE, GPIO_INDEX29);
	GPIO_RegOneBitClear(GPIO_A_OE, GPIO_INDEX29);

	IR_Config(IR_MODE_SONY, IR_GPIOA29, IR_SONY_12BITS);
	IR_Enable();

	while(1)
	{
		CmdOK = IR_CommandFlagGet();
		if(CmdOK == TRUE)
		{
			DBG("\n查询方式\n");
			Cmd = IR_CommandDataGet();
			DBG("\n---IR_Cmd:%08lx\n",Cmd);
			IR_IntFlagClear();

			RepeatCount = IR_RepeatCountGet();
			DBG("\n---RepeatCount:%0x\n",RepeatCount);
			while(1)
			{
				DelayMs(45);//NEC IR repeat code is transmited every 110mS

				Last_RepeatCount = RepeatCount;

				RepeatCount = IR_RepeatCountGet();
				DBG("\n*********RepeatCount:%0x\n",RepeatCount);

				if( (Last_RepeatCount - RepeatCount) > 0 )
				{
					Cmd = IR_CommandDataGet();
					DBG("\n--------There is a New IR_Command:%08lx------\n",Cmd);
					DBG("\n*****************IR_Cmd:%08lx\n",Cmd);
					IR_IntFlagClear();
				}

				if(RepeatCount == Last_RepeatCount)
				{
					IR_CommandFlagClear();
					break;
				}
			}
		}
	}
}

void SONY_IR_InterruptExample(void)
{
	DBG("enter NEC IR---A29 Interrupt Example\n\n");

	GPIO_RegOneBitSet(GPIO_A_IE, GPIO_INDEX29);
	GPIO_RegOneBitClear(GPIO_A_OE, GPIO_INDEX29);

	NVIC_EnableIRQ(IR_IRQn);
	NVIC_SetPriority(IR_IRQn, 1);
	GIE_ENABLE();

	IR_Config(IR_MODE_SONY, IR_GPIOA29, IR_SONY_12BITS);
	IR_Enable();//开启IR功能
	IR_InterruptEnable();//开启IR中断
}

void SONY_IR_WackupFromDeepSleepExample(void)
{
	 uint8_t Temp = 0;
	 uint32_t Cmd = 0;

	DBG("\n Input 'e' to enter SystemDeepSleepPowerConsumption_Example again\n");
	DBG("\n Input 'x' to quit SystemDeepSleepPowerConsumption_Example \n");

	 while(1)
	 {
		 UARTS_Recv(UART_RX_IO, &Temp, 1, 1000);

		 if(Temp == 'e')
		 {
				DBG("Wait 3s then enter DeepSleep\n");

				DelayMs(3000);

			 	//DeepSleepPowerConsumption Config
			 	DeepSleepPowerConsumption_Config();

			 	//Select Wakeup Source
			 	SystermIRWakeupConfig(IR_MODE_SONY, IR_GPIOB6, IR_SONY_12BITS);

			 	//Enter Deepsleep
			 	Power_GotoDeepSleep();

				//Wakeup from DeepSleep
				SystermAfterWackup();

				//Wakeup Source Get
				WakeupSourceGet();

				while(1)
				{
					//获取IR键值
					if((IR_CommandFlagGet() == TRUE))
					{
						Cmd = IR_CommandDataGet();
						DBG("\n---IR_Cmd:%08lx\n",Cmd);
						IR_IntFlagClear();
						IR_CommandFlagClear();

						if(IR_SONY_KEY == Cmd)
						{
							break;
						}
						else
						{
							DBG("IR_KEY  is wrong!!!,enter DeepSleep again!!!\n");
							//DeepSleepPowerConsumption Config
							DeepSleepPowerConsumption_Config();

							//Select Wakeup Source
							SystermIRWakeupConfig(IR_MODE_SONY, IR_GPIOB6, IR_SONY_12BITS);

							//Enter Deepsleep
							Power_GotoDeepSleep();

							//Wakeup from DeepSleep
							SystermAfterWackup();

							//Wakeup Source Get
							WakeupSourceGet();
						}
					}
				}

				DBG("\n Input 'e' to enter SystemDeepSleepPowerConsumption_Example again\n");
				DBG("\n Input 'x' to quit SystemDeepSleepPowerConsumption_Example \n");
		 }
		 else if(Temp == 'x')
		 {
			 DBG("\n  Quit SystemDeepSleepPowerConsumption_Example \n");
			 break;
		 }
	 }
}

int main(void)
{
	uint32_t recvBuf = 0;

	Chip_Init(1);
	WDG_Disable();

	Clock_Module1Enable(ALL_MODULE1_CLK_SWITCH);
	Clock_Module2Enable(ALL_MODULE2_CLK_SWITCH);
	Clock_Module3Enable(ALL_MODULE3_CLK_SWITCH);

	Clock_Config(1, 24000000);
	Clock_PllLock(288000);
	Clock_SysClkSelect(PLL_CLK_MODE);
	Clock_UARTClkSelect(PLL_CLK_MODE);

	GPIO_PortAModeSet(GPIOA24, 1);//Rx,A24:uart1_rxd_0
	GPIO_PortAModeSet(GPIOA25, 6);//Tx,A25:uart1_txd_0
	DbgUartInit(1, 256000, 8, 0, 1);

	SysTickInit();

	Remap_InitTcm(0, 12);
	SpiFlashInit(80000000, MODE_4BIT, 0, 1);

	PllFreqGet = Clock_PllFreqGet();
	APllFreqGet = Clock_APllFreqGet();
	SystemClockDivGet = Clock_SysClkDivGet();


	DBG("\n");
    DBG("/-----------------------------------------------------\\\n");
    DBG("|                     IR Example                     |\n");
    DBG("|      Mountain View Silicon Technology Co.,Ltd.      |\n");
    DBG("\\-----------------------------------------------------/\n");
    DBG("\n");

	DBG("-------------------------------------------------------\n");
	DBG("Input '1' to enter NEC IR Recieve Cmd example\n\n");
	DBG("Input '2' to enter NEC IR Interrupt example\n\n");
	DBG("Input '3' to enter NEC IR WakeupFromDeepSleep example\n\n");
	DBG("Input '4' to enter SONY IR Recieve Cmd example\n\n");
	DBG("Input '5' to enter SONY IR Interrupt example\n\n");
	DBG("Input '6' to enter SONY IR WakeupFromDeepSleep example\n\n");
	DBG("-------------------------------------------------------\n");

    	while(1)
    	{
    		recvBuf = 0;
    		UARTS_Recv(UART_RX_IO, (uint8_t *)&recvBuf, 1,	1000);

    		if(recvBuf == '1')
    		{
    			//NEC  IR RecieveCmdExample
    			NEC_IR_RecieveCmdExample();
    		}
    		else if(recvBuf == '2')
    		{
    			//NEC IR Interrupt example
    			NEC_IR_InterruptExample();
    		}
    		else if(recvBuf == '3')
    		{
    			//NEC IR WakeupFromDeepSleep example
    			NEC_IR_WackupFromDeepSleepExample();
    		}
    		else if(recvBuf == '4')
    		{
    			//SONY IR RecieveCmdExample
    			SONY_IR_RecieveCmdExample();
    		}
    		else if(recvBuf == '5')
    		{
    			//SONY IR Interrupt example
    			SONY_IR_InterruptExample();
    		}
    		else if(recvBuf == '6')
    		{
    			// SONY IR WakeupFromDeepSleep example
    			SONY_IR_WackupFromDeepSleepExample();
    		}
    	}

    	return 0;
}

static bool INT_Flag = FALSE;
__attribute__((section(".driver.isr"))) void IRInterrupt(void)
{
	uint32_t Cmd = 0;

	if( IR_IntFlagGet() == TRUE )
	{
		DBG("\n**********中断方式**********\n");
		INT_Flag = FALSE;

		Cmd = IR_CommandDataGet();
		DBG("\n---IR_Cmd:%08lx\n",Cmd);
	}
	IR_IntFlagClear();
//	IR_CommandFlagClear();
}
