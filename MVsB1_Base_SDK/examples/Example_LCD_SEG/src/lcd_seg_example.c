/**
 **************************************************************************************
 * @file    gpio_example.c
 * @brief   gpio example
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
#include "lcd_seg.h"

//注意使用的UART端口
#define UART_TX_IO	1
#define UART_RX_IO  1

#define  SEG_PORT LCD14_A24|LCD0_A5|LCD11_A16
#define  COM_PORT LCD1_A6
#define  BIAS_MODE_1
void LCD_Seg_MultiCtrl_Example(void)
{
	uint8_t i = 0;
	uint8_t TeatBufLen = 0;
	//LCD_CTRL SegCtrl = LCD_BIAS1_FOUR_QUARTERS;
	//LCD_CTRL ComCtrl = LCD_BIAS1_0V;
	//LCD_CTRL SegCtrlBuf1[] = {LCD_BIAS0_THREE_THIRDS, LCD_BIAS0_TWO_THIRDS, LCD_BIAS0_ONE_THIRDS, LCD_BIAS0_0V};
	//LCD_CTRL ComCtrlBuf1[] = {LCD_BIAS0_0V, LCD_BIAS0_ONE_THIRDS, LCD_BIAS0_TWO_THIRDS, LCD_BIAS0_THREE_THIRDS};
	LCD_CTRL SegCtrlBuf2[] = {LCD_BIAS1_FOUR_QUARTERS, LCD_BIAS1_THREE_QUARTERS, LCD_BIAS1_TWO_QUARTERS, LCD_BIAS1_QUARTER, LCD_BIAS1_0V};
	LCD_CTRL ComCtrlBuf2[] = {LCD_BIAS1_0V, LCD_BIAS1_QUARTER, LCD_BIAS1_TWO_QUARTERS, LCD_BIAS1_THREE_QUARTERS, LCD_BIAS1_FOUR_QUARTERS};

	DBG("\n---------Enter LCD_Seg_Example---------");

	#ifdef BIAS_MODE_1
		LCD_AnalogConfig(LCD_BIAS_ONE_FOURTH, LCD_DRIVE_CURRENT_LOW, LCD_VDD33);//bias for 1/4
		TeatBufLen = sizeof(SegCtrlBuf2);
	#else
		LCD_AnalogConfig(LCD_BIAS_ONE_THIRD, LCD_DRIVE_CURRENT_LOW, LCD_VDD33);//bias for 1/3
		TeatBufLen = sizeof(SegCtrlBuf1);
	#endif


	LCD_SegInit(SEG_PORT, COM_PORT);
	LCD_SegEnable(SEG_PORT, COM_PORT);
	//LCD_SegDisable(LCD0_A5, LCD1_A6);

	Timer_Config(TIMER2, 20, 0);
	Timer_Start(TIMER2);

	while(i < TeatBufLen)
	{
		while(1)
		{
			if(Timer_InterruptFlagGet(TIMER2, UPDATE_INTERRUPT_SRC))
			{
				Timer_InterruptFlagClear(TIMER2, UPDATE_INTERRUPT_SRC);
				break;
			}
		}

		#ifdef BIAS_MODE_1
			LCD_SegData(SEG_PORT, SegCtrlBuf2[i], COM_PORT, ComCtrlBuf2[i]);
		#else
			LCD_SegData(SEG_PORT, SegCtrlBuf1[i], COM_PORT, ComCtrlBuf1[i]);
		#endif

		if( i == (TeatBufLen - 1))
		{
			i = 0;
		}

		i++;
	}

}

void LCD_Seg_OneCtrl_Example(void)
{
	LCD_CTRL SegCtrl = LCD_BIAS1_FOUR_QUARTERS;
	LCD_CTRL ComCtrl = LCD_BIAS1_0V;

	DBG("\n---------Enter LCD_Seg_Example---------");

	#ifdef BIAS_MODE_1
		LCD_AnalogConfig(LCD_BIAS_ONE_FOURTH, LCD_DRIVE_CURRENT_LOW, LCD_VDD33);//bias for 1/4
	#else
		LCD_AnalogConfig(LCD_BIAS_ONE_THIRD, LCD_DRIVE_CURRENT_LOW, LCD_VDD33);//bias for 1/3
	#endif

	LCD_SegInit(SEG_PORT, COM_PORT);
	LCD_SegEnable(SEG_PORT, COM_PORT);
	//LCD_SegDisable(LCD0_A5, LCD1_A6);

	Timer_Config(TIMER2, 200, 0);
	Timer_Start(TIMER2);

	while(1)
	{
		while(1)
		{
			if(Timer_InterruptFlagGet(TIMER2, UPDATE_INTERRUPT_SRC))
			{
				//GPIO_RegOneBitSet(GPIO_A_TGL, GPIOA22);
				Timer_InterruptFlagClear(TIMER2, UPDATE_INTERRUPT_SRC);
				break;
			}
		}

		SegCtrl = LCD_BIAS1_FOUR_QUARTERS;
		ComCtrl = LCD_BIAS1_0V;
		LCD_SegData(SEG_PORT, SegCtrl, COM_PORT, ComCtrl);

		while(1)
		{
			if(Timer_InterruptFlagGet(TIMER2, UPDATE_INTERRUPT_SRC))
			{
				//GPIO_RegOneBitSet(GPIO_A_TGL, GPIOA22);
				Timer_InterruptFlagClear(TIMER2, UPDATE_INTERRUPT_SRC);
				break;
			}
		}

		SegCtrl = LCD_BIAS1_0V;
		ComCtrl = LCD_BIAS1_FOUR_QUARTERS;
		LCD_SegData(SEG_PORT, SegCtrl, COM_PORT, ComCtrl);

	}

}

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

	GPIO_PortAModeSet(GPIOA24, 1);//Rx,A24:uart1_rxd_0
	GPIO_PortAModeSet(GPIOA25, 6);//Tx,A25:uart1_txd_0
	DbgUartInit(1, 256000, 8, 0, 1);

	SysTickInit();

	Remap_InitTcm(0, 12);
	SpiFlashInit(80000000, MODE_4BIT, 0, 1);

	DBG("\n");
    DBG("/-----------------------------------------------------\\\n");
    DBG("|                     LCD Seg Example                     |\n");
    DBG("|      Mountain View Silicon Technology Co.,Ltd.      |\n");
    DBG("\\-----------------------------------------------------/\n");
    DBG("\n");

    LCD_Seg_OneCtrl_Example();
    //LCD_Seg_MultiCtrl_Example();


    return 0;
}
