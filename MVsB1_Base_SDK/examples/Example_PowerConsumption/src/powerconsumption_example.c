/**
 **************************************************************************************
 * @file    power_waste_example.c
 * @brief   power_waste example
 *
 * @author  Taowen
 * @version V1.0.0
 *
 * $Created: 2019-05-31 19:17:00$
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
#include "powercontroller.h"
#include "dac.h"
#include "rtc.h"
#include "ir.h"
#include "pwc.h"
#include "backup.h"
#include "delay.h"


//ע��ʹ�õ�UART�˿�
#define UART_TX_IO	1
#define UART_RX_IO  1


#define SW_PORTB  1


uint32_t PllFreqGet  = 0;
uint32_t APllFreqGet  = 0;
CLK_MODE SystemClockSel = PLL_CLK_MODE;
uint32_t SystemClockDivGet = 0;

void UARTLogIOConfig(void)
{
	//GPIO_PortAModeSet(GPIOA9, 1);//Rx, A9:uart1_rxd_0
	//GPIO_PortAModeSet(GPIOA10, 3);//Tx,A10:uart1_txd_0
	GPIO_PortAModeSet(GPIOA24, 1);//Rx,A24:uart1_rxd_0
	GPIO_PortAModeSet(GPIOA25, 6);//Tx,A25:uart1_txd_0
	DbgUartInit(1, 256000, 8, 0, 1);
}

void SystermAfterWackup(void)
{
	Chip_Init(1);
	WDG_Disable();
 	Power_PowerModeConfigTest(FALSE, FALSE, FALSE);//B1X��HPMģʽ��LPMģʽ�ڹ�����ûɶ����
	Clock_Module1Enable(ALL_MODULE1_CLK_SWITCH);
	Clock_Module2Enable(ALL_MODULE2_CLK_SWITCH);
	Clock_Module3Enable(ALL_MODULE3_CLK_SWITCH);

	Clock_Config(1, 24000000);
	Clock_PllLock(PllFreqGet/1000);
	Clock_DeepSleepSysClkSelect(PLL_CLK_MODE, FSHC_PLL_CLK_MODE, 0);
	Clock_SysClkSelect(SystemClockSel);
	Clock_SysClkDivSet(SystemClockDivGet);
	Clock_UARTClkSelect(RC_CLK_MODE);

	UARTLogIOConfig();

	SysTickInit();

	SpiFlashInit(80000000, MODE_4BIT, 0, 1);

	#ifdef SW_PORTB  //������B��ζSW���ؿڣ�������A��

		GPIO_PortBModeSet(GPIOB0, 0x1);////��ǰ��ر���GPIOB0��B1����ΪSW���ص��Կڸ���ģʽ�����ڴ˴����� ;
		GPIO_PortBModeSet(GPIOB1, 0x1);
	 #else
	 	GPIO_PortAModeSet(GPIOA30, 0x5);//��ǰ��ر���GPIOA30��A31����ΪSW���ص��Կڸ���ģʽ�����ڴ˴����� ;
	 	GPIO_PortAModeSet(GPIOA31, 0x4);

	 #endif

	    DBG("-------Wakeup from DeepSleep-------\n");
}

////////////////////B1P10 System DeepSleep Example///////////////////////////////////////////

//#define IR_WAKEUP
#define IR_NEC_KEY (0xED127F80)
#define IR_SONY_KEY (0x95)

extern void SystermGPIOWakeupConfig();
extern void SystermRTCWakeupConfig();
extern void SystermONKEYWakeupConfig();
extern void SystermIRWakeupConfig();
extern void SystermTimer3IntWakeupConfig();
extern void SystermUART_RXWakeupConfig();
extern void Systerm_LVD_WakeupConfig();
extern void Systerm_BT_WakeupConfig();
extern void DeepSleepPowerConsumption_Config(void);
extern void WakeupSourceGet(void);
extern void SleepPowerConsumption_Config(void);

 void SystemDeepSleepPowerConsumption_Example(void)
 {
	 uint8_t Temp = 0;
	 uint32_t Cmd = 0;

	 DBG("Input 'e' to enter DeepSleep  !\n");

	 while(1)
	 {
		 UARTS_Recv(UART_RX_IO, &Temp, 1, 1000);

		 if(Temp == 'e')
		 {
				DBG("Wait 3s then enter DeepSleep\n");

				DelayMs(3000);

			 	//DeepSleepPowerConsumption Config
			 	DeepSleepPowerConsumption_Config();

			 	////Select Wakeup Source
			 	SystermRTCWakeupConfig(15);//RTC����Դ�������ã�0��ʾ��0�뿪ʼ��ʱ��15��ʾRTC������ʱ15�����deepsleep
			 	//SystermGPIOWakeupConfig(SYSWAKEUP_SOURCE0_GPIO, WAKEUP_GPIOB3, SYSWAKEUP_SOURCE_POSE_TRIG);
			 	//Systerm_LVD_WakeupConfig(PWR_LVD_Threshold_3V0);
			 	//SystermPowerKeyWakeupConfig(SYSWAKEUP_SOURCE6_POWERKEY, SYSWAKEUP_SOURCE_POSE_TRIG);

			#ifdef IR_WAKEUP
			 	SystermIRWakeupConfig(IR_MODE_NEC, IR_GPIOB6, IR_NEC_32BITS);
			#endif

			 	//SystermTimer3IntWakeupConfig(PWC1_CAP_DATA_INTERRUPT_SRC);PWC�����ж�(only Timer3)
			 	//SystermTimer3IntWakeupConfig(UPDATE_INTERRUPT_SRC); //PWC��ʱ�ж�(only Timer3)
			 	//SystermUART_RXWakeupConfig(UART_PORT0);
			 	//SystermUART_RXWakeupConfig(UART_PORT1);
			 	//Systerm_BT_WakeupConfig(SYSWAKEUP_SOURCE13_BT);

			 	//Enter Deepsleep
			 	Power_GotoDeepSleep();

				//Wakeup from DeepSleep
				SystermAfterWackup();

				//Wakeup Source Get
				WakeupSourceGet();

			#ifdef IR_WAKEUP

					while(1)
					{
						//��ȡIR��ֵ
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
							 	SystermIRWakeupConfig(IR_MODE_NEC, IR_GPIOB6, IR_NEC_32BITS);//���û���Դ

							 	//Enter Deepsleep
							 	Power_GotoDeepSleep();

								//Wakeup from DeepSleep
								SystermAfterWackup();

								//Wakeup Source Get
								WakeupSourceGet();
							}
						}
					}
			#endif
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

void SystemSleepPowerConsumption_Example(void)
{
	GPIO_TriggerType triggerType;

	DBG("\n   Wait 5 sec then Enter SystemSleepPowerConsumption_Example                   \n");

 	DelayMs(5000);

 	//SleepPowerConsumption Config
	 SleepPowerConsumption_Config();

	 //���������жϺ��������˳�Sleepģʽ���κ��ж϶�����ʹϵͳ�����˳�Sleepģʽ
	 triggerType = GPIO_NEG_EDGE_TRIGGER;
	 // A4 Ϊ�жϼ���
	 GPIO_RegOneBitSet(GPIO_A_IE, GPIO_INDEX4);
	 GPIO_RegOneBitClear(GPIO_A_OE, GPIO_INDEX4);
	 GPIO_RegOneBitSet(GPIO_A_PD, GPIO_INDEX4);
	 GPIO_RegOneBitClear(GPIO_A_PU, GPIO_INDEX4);
	 GPIO_INTEnable(GPIO_A_INT, GPIO_INDEX4, triggerType);
	 NVIC_EnableIRQ(Gpio_IRQn);
	#ifdef WAKEUP_BY_UART_INT
	#ifdef WAKEUP_BY_UART0
	//ʹ��uart0�жϻ��ѹ���
	 NVIC_EnableIRQ(UART0_IRQn);
	 UARTS_IOCTL(UART_PORT0, UART_IOCTL_RXINT_SET, 1);
	 UARTS_IOCTL(UART_PORT0, UART_IOCTL_RXINT_CLR, 1);
	 UARTS_IOCTL(UART_PORT0, UART_IOCTL_RXFIFO_CLR, 1);
	#endif
	#ifdef WAKEUP_BY_UART1
	 //ʹ��uart1�жϻ��ѹ���
	 NVIC_EnableIRQ(UART1_IRQn);
	 UARTS_IOCTL(UART_PORT1, UART_IOCTL_RXINT_SET, 1);
	 UARTS_IOCTL(UART_PORT1, UART_IOCTL_RXINT_CLR, 1);
	 UARTS_IOCTL(UART_PORT1, UART_IOCTL_RXFIFO_CLR, 1);
	#endif
	#endif
	 NVIC_SetPriority(Gpio_IRQn, 0);
	 GIE_ENABLE();

 	//Enter Sleep
	Power_GotoSleep();

	//Wakeup from Sleep
	SystermAfterWackup();

	DBG("\nExit from sleep\n");
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

	UARTLogIOConfig();

	SysTickInit();

	Remap_InitTcm(0, 12);
	SpiFlashInit(80000000, MODE_4BIT, 0, 1);

	DBG("\n");
    DBG("/-----------------------------------------------------\\\n");
    DBG("|                     Power Consumption Example       |\n");
    DBG("|      Mountain View Silicon Technology Co.,Ltd.      |\n");
    DBG("\\-----------------------------------------------------/\n");
    DBG("\n");

	DBG("-------------------------------------------------------\n");
	////////////BP10 deepsleep and wakeup from GPIO/RTC/LVD/ONKEY/IR
	DBG("Input '1' to enter SystemDeepSleepPowerConsumption_Example\n");
	DBG("Input '2' to enter SystemSleepPowerConsumption_Example\n");
	DBG("-------------------------------------------------------\n");

	PllFreqGet = Clock_PllFreqGet();
	APllFreqGet = Clock_APllFreqGet();
	SystemClockDivGet = Clock_SysClkDivGet();

	while(1)
	{
		recvBuf = 0;
		UARTS_Recv(UART_RX_IO, (uint8_t *)&recvBuf, 1,	1000);

		if(recvBuf == '1')
		{
			////////////SystemDeepSleepPowerConsumption_Example///////
			SystemDeepSleepPowerConsumption_Example();
		}
		else if(recvBuf == '2')
		{
			////////////SystemSleepPowerConsumption_Example///////
			SystemSleepPowerConsumption_Example();
		}
	}

    	return 0;
}

