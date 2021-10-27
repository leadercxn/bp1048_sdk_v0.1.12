/**
 **************************************************************************************
 * @file    timer_example.c
 *
 * @author  Bruce
 * @version V1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */


#include <string.h>
#include <nds32_intrinsic.h>
#include "type.h"
#include "irqn.h"
#include "debug.h" 
#include "watchdog.h"
#include "gpio.h"
#include "uarts.h"
#include "timeout.h"
#include "delay.h"
#include "uarts_interface.h"
#include "clk.h"
#include "spi_flash.h"
#include "remap.h"
#include "chip_info.h"
#include "timer.h"

#define UART_TX_IO  1
#define UART_RX_IO  1

extern uint32_t  ClockFreqGet(TIMER_INDEX TimerIndex);
extern void SysTickInit(void);

char    tempString[20];

char GetChar(void)
{
    uint8_t     ch = 0;
    uint8_t     ret=0;
    while(ret == 0)
    {
        if(UARTS_Recv(UART_RX_IO, &ch, 1,10) > 0)
        {
            ret=1;
        }
    }
    return ch;
}

char * GetString(void)
{
    char            ch;
    char            *tempP;
    memset(tempString, 0, sizeof(tempString));
    tempP = tempString;

    ch = GetChar();
    while(ch != 0x0D)
    {
        DBG("%c", ch);
        *(tempP++) = ch;
        ch = GetChar();
    }

    return tempString;
}

uint32_t    StrToI(char * str)
{
    uint32_t            result;

    result = 0;
    while(*str)
    {
        result = result*10 + *(str++) - '0';
    }
    return result;
}

__attribute__((section(".driver.isr"))) void Timer2Interrupt(void)
{
	DBG("Timer2Interrupt in\n");
    GPIO_RegOneBitSet(GPIO_A_TGL, GPIO_INDEX22); // toggle A22
    Timer_InterruptFlagClear(TIMER2, UPDATE_INTERRUPT_SRC);
}

__attribute__((section(".driver.isr"))) void Timer3Interrupt(void)
{
	DBG("Timer3Interrupt in\n");
    GPIO_RegOneBitSet(GPIO_A_TGL, GPIO_INDEX22); // toggle A22
    Timer_InterruptFlagClear(TIMER3, UPDATE_INTERRUPT_SRC);
}

__attribute__((section(".driver.isr"))) void Timer4Interrupt(void)
{
	DBG("Timer4Interrupt in\n");
    GPIO_RegOneBitSet(GPIO_A_TGL, GPIO_INDEX22); // toggle A22
    Timer_InterruptFlagClear(TIMER4, UPDATE_INTERRUPT_SRC);
}

__attribute__((section(".driver.isr"))) void Timer5Interrupt(void)
{
	DBG("Timer5Interrupt in\n");
    GPIO_RegOneBitSet(GPIO_A_TGL, GPIO_INDEX22); // toggle A22
    Timer_InterruptFlagClear(TIMER5, UPDATE_INTERRUPT_SRC);
}

__attribute__((section(".driver.isr"))) void Timer6Interrupt(void)
{
	DBG("Timer6Interrupt in\n");
    GPIO_RegOneBitSet(GPIO_A_TGL, GPIO_INDEX22); // toggle A22
    Timer_InterruptFlagClear(TIMER6, UPDATE_INTERRUPT_SRC);
}

//uint32_t usec: IO口翻转的时间，单位:1us
//TImer1默认作为系统定时Timer，用户应使用TIMER2~TIMER6
void TimerInterruptModeExample(TIMER_INDEX TimerId,uint32_t usec)
{
    uint8_t recvBuf;

    DBG("TimerInterruptModeExample:toggle A22\n");
    DBG("Input 'x' to exit this example\n");
    DBG("Input 'p' to Pause\n");
    DBG("Input 'c' to Continues\n");

    NVIC_EnableIRQ(Timer2_IRQn);
    NVIC_EnableIRQ(Timer3_IRQn);
    NVIC_EnableIRQ(Timer4_IRQn);
    NVIC_EnableIRQ(Timer5_IRQn);
    NVIC_EnableIRQ(Timer6_IRQn);

    Timer_InterruptFlagClear(TimerId, UPDATE_INTERRUPT_SRC);

    GPIO_RegOneBitSet(GPIO_A_TGL, GPIO_INDEX22);

    Timer_Config(TimerId, usec, 0);
    Timer_Start(TimerId);

    while(1)
    {
        recvBuf = 0;
        UARTS_Recv(UART_RX_IO, &recvBuf, 1,100);
        if(recvBuf == 'x')
        {
            Timer_Pause(TimerId, 1);
            break;
        }
        if(recvBuf == 'p')
        {
            Timer_Pause(TimerId, 1);
            DBG("Timer Pause\n");
        }
        if(recvBuf == 'c')
        {
            Timer_Pause(TimerId, 0);
            DBG("Timer Continues\n");
        }
    }
}

//uint32_t usec: IO口翻转的时间，单位:1us
void TimerCheckModeExample(TIMER_INDEX TimerId,uint32_t usec)
{
    uint8_t recvBuf;

    DBG("TimerCheckModeExample: toggle A22\n");
    DBG("Input 'x' to exit this example\n");

    NVIC_DisableIRQ(Timer2_IRQn);
    NVIC_DisableIRQ(Timer3_IRQn);
    NVIC_DisableIRQ(Timer4_IRQn);
    NVIC_DisableIRQ(Timer5_IRQn);
    NVIC_DisableIRQ(Timer6_IRQn);

    Timer_InterruptFlagClear(TimerId, UPDATE_INTERRUPT_SRC);
    GPIO_RegOneBitSet(GPIO_A_TGL, GPIO_INDEX22);

    Timer_Config(TimerId, usec, 0);
    Timer_Start(TimerId);

    while(1)
    {
        recvBuf = 0;
        UARTS_Recv(UART_RX_IO, &recvBuf, 1,100);
        if(recvBuf == 'x')
        {
            Timer_Pause(TimerId, 1);
            break;
        }

        if(Timer_InterruptFlagGet(TimerId, UPDATE_INTERRUPT_SRC))
        {
            GPIO_RegOneBitSet(GPIO_A_TGL, GPIO_INDEX22);
            Timer_InterruptFlagClear(TimerId, UPDATE_INTERRUPT_SRC);
            if(usec >= 500000)
            {
                DBG("Timer%d Update\n",TimerId+1);
            }
        }
    }
}

void MastSlaveModeTest(void)
{
	TIMER_CTRL2 TimerArgs;
	uint8_t recvBuf;

	DBG("MastSlaveModeTest in\n");

    NVIC_EnableIRQ(Timer3_IRQn);
    NVIC_EnableIRQ(Timer4_IRQn);
    NVIC_EnableIRQ(Timer5_IRQn);
    NVIC_EnableIRQ(Timer6_IRQn);

    Timer_InterruptFlagClear(TIMER3, UPDATE_INTERRUPT_SRC);
    Timer_InterruptFlagClear(TIMER4, UPDATE_INTERRUPT_SRC);
    Timer_InterruptFlagClear(TIMER5, UPDATE_INTERRUPT_SRC);
    Timer_InterruptFlagClear(TIMER6, UPDATE_INTERRUPT_SRC);

    TimerArgs.MasterModeSel = 1;
    TimerArgs.MasterSlaveSel = 0;
    TimerArgs.TriggerInSrc = 0;
    TimerArgs.CcxmAndCccUpdataSel = 0;
    Timer_ConfigMasterSlave(TIMER3,3000*1000 , 1, &TimerArgs);
    TimerArgs.MasterModeSel = 0;
    TimerArgs.MasterSlaveSel = 1;
    TimerArgs.TriggerInSrc = 0;
    TimerArgs.CcxmAndCccUpdataSel = 1;
    Timer_ConfigMasterSlave(TIMER4,2200*1000 , 0, &TimerArgs);
    TimerArgs.MasterModeSel = 0;
    TimerArgs.MasterSlaveSel = 1;
    TimerArgs.TriggerInSrc = 0;
    TimerArgs.CcxmAndCccUpdataSel = 1;
    Timer_ConfigMasterSlave(TIMER5,2500*1000 , 0, &TimerArgs);
    TimerArgs.MasterModeSel = 0;
    TimerArgs.MasterSlaveSel = 1;
    TimerArgs.TriggerInSrc = 0;
    TimerArgs.CcxmAndCccUpdataSel = 1;
    Timer_ConfigMasterSlave(TIMER6,2800*1000 , 0, &TimerArgs);
    Timer_Start(TIMER3);

    while(1)
	{
		recvBuf = 0;
		UARTS_Recv(UART_RX_IO, &recvBuf, 1,100);
		if(recvBuf == 'x')
		{
			Timer_Pause(TIMER3, 1);
			Timer_Pause(TIMER4, 1);
			Timer_Pause(TIMER5, 1);
			Timer_Pause(TIMER6, 1);
			break;
		}
		if(recvBuf == 'p')
		{
			Timer_Pause(TIMER3, 1);
			Timer_Pause(TIMER4, 1);
			Timer_Pause(TIMER5, 1);
			Timer_Pause(TIMER6, 1);
			DBG("Timer Pause\n");
		}
		if(recvBuf == 'c')
		{
			Timer_Pause(TIMER3, 0);
			Timer_Pause(TIMER4, 0);
			Timer_Pause(TIMER5, 0);
			Timer_Pause(TIMER6, 0);
			DBG("Timer Continues\n");
		}
	}
}

int main(void)
{
    uint8_t recvBuf = 0;
    TIMER_INDEX TimerId = TIMER2;

    Chip_Init(1);
    WDG_Disable();
    //使用外挂24MHz晶振，锁定PLL到288MHz，此时CPU频率288MHz,系统频率144MHz
    Clock_Config(1, 24000000);
    Clock_PllLock(288000);

    Clock_Module1Enable(ALL_MODULE1_CLK_SWITCH);
    Clock_Module2Enable(ALL_MODULE2_CLK_SWITCH);
    Clock_Module3Enable(ALL_MODULE3_CLK_SWITCH);

    Clock_SysClkSelect(PLL_CLK_MODE);
    Clock_UARTClkSelect(PLL_CLK_MODE);

    //Remap_AddrRemapSet(0, 0, 0x20010000, 64);
    Remap_InitTcm(FLASH_ADDR, TCM_SIZE);
    SpiFlashInit(80000000, MODE_4BIT, 0, 1);
    Clock_UARTClkSelect(PLL_CLK_MODE);
	/********************GPIOA25---->TXD***************/
	/********************GPIOA24---->RXD***************/
	GPIO_PortAModeSet(GPIOA24, 1);//Rx, A24:uart1_rxd_0
	GPIO_PortAModeSet(GPIOA25, 6);//Tx, A25:uart1_txd_0
	DbgUartInit(1, 256000, 8, 0, 1);
	
	//设置GPIOA22为输出
	GPIO_RegOneBitClear(GPIO_A_IE, GPIO_INDEX22);
	GPIO_RegOneBitSet(GPIO_A_OE, GPIO_INDEX22);

    DBG("\nBuilt time: %s %s\n",__TIME__,__DATE__);

    SysTickInit();//如果有用到timeout接口函数，需要调用SysTickInit
    
    GIE_ENABLE();

    RESTART:

    while(1)
    {
        DBG("-------------------------------------------------------\n");
        DBG("\t\tTimer Example\n");
        DBG("Examp Menu:\n");
        DBG("a: Timer Interrupt mode example start,default is Timer2\n");
        DBG("b: Timer Check mode example start,default is Timer2\n");
        DBG("c: Timer Master Slave mode test\n");
        DBG("n: Next Timer\n");
        DBG("-------------------------------------------------------\n");

        recvBuf = 0;
        recvBuf = GetChar();

        if(recvBuf == 'a')
        {
            uint32_t usec = 1000000;
            //Timer2 中断模式Toggle GPIO A22

            DBG("Timer%d Interrupt mode, toggle gpio every %lu ms\n",TimerId+1, usec/1000);

            TimerInterruptModeExample(TimerId,usec); // 1000ms
            goto RESTART;
        }
        else if(recvBuf == 'b')
        {
            uint32_t usec = 1000000;
            //Timer2 查询模式 Toggle GPIO A22

            DBG("Timer%d check mode, toggle gpio every %d ms\n",TimerId+1, (int)usec/1000);

            TimerCheckModeExample(TimerId,usec); // 1000ms
            goto RESTART;
        }
        else if(recvBuf == 'c')
        {
        	MastSlaveModeTest();
            goto RESTART;
        }
        else if(recvBuf == 'n')
        {
        	TimerId ++;
        	if(TimerId > TIMER6)
        	{
        		TimerId = TIMER2;
        	}
        	DBG("Select Timer%d now,Press a or A please\n",TimerId);
        }
    }
}
