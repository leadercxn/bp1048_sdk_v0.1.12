/**
 **************************************************************************************
 * @file    adc_example.c
 * @brief   adc example
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2019-5-30 13:25:00$
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
#include "chip_info.h"
#include "sadc_interface.h"
#include "irqn.h"

static uint8_t DmaChannelMap[29] = {
	255,//PERIPHERAL_ID_SPIS_RX = 0,		//0
	255,//PERIPHERAL_ID_SPIS_TX,			//1
	0,//PERIPHERAL_ID_TIMER3,			//2
	255,//PERIPHERAL_ID_SDIO0_RX,			//3
	255,//PERIPHERAL_ID_SDIO0_TX,			//4
	255,//PERIPHERAL_ID_UART0_RX,			//5
	255,//PERIPHERAL_ID_TIMER1,				//6
	255,//PERIPHERAL_ID_TIMER2,				//7
	255,//PERIPHERAL_ID_SDPIF_RX,			//8 SPDIF_RX /TX需使用同一通道
	255,//PERIPHERAL_ID_SDPIF_TX,			//9
	255,//PERIPHERAL_ID_SPIM_RX,			//10
	255,//PERIPHERAL_ID_SPIM_TX,			//11
	255,//PERIPHERAL_ID_UART0_TX,			//12
	255,//PERIPHERAL_ID_UART1_RX,			//13
	255,//PERIPHERAL_ID_UART1_TX,			//14
	255,//PERIPHERAL_ID_TIMER4,				//15
	255,//PERIPHERAL_ID_TIMER5,				//16
	255,//PERIPHERAL_ID_TIMER6,				//17
	255,//PERIPHERAL_ID_AUDIO_ADC0_RX,		//18
	255,//PERIPHERAL_ID_AUDIO_ADC1_RX,		//19
	5,//PERIPHERAL_ID_AUDIO_DAC0_TX,		//20
	255,//PERIPHERAL_ID_AUDIO_DAC1_TX,		//21
	3,//PERIPHERAL_ID_I2S0_RX,			//22
	4,//PERIPHERAL_ID_I2S0_TX,			//23
	1,//PERIPHERAL_ID_I2S1_RX,			//24
	255,//PERIPHERAL_ID_I2S1_TX,			//25
	2,//PERIPHERAL_ID_PPWM,				//26
	6,//PERIPHERAL_ID_ADC,     			//27
	255,//PERIPHERAL_ID_SOFTWARE,			//28
};

//	ADC_CHANNEL_VIN = 0,		/**channel 0*/
//	ADC_CHANNEL_VBK,			/**channel 1*/
//	ADC_CHANNEL_VDD1V2,			/**channel 2*/
//	ADC_CHANNEL_VCOM,			/**channel 3*/
//	ADC_CHANNEL_GPIOA20_A23,	/**channel 4*/
//	ADC_CHANNEL_GPIOA21_A24,	/**channel 5*/
//	ADC_CHANNEL_GPIOA22_A25,	/**channel 6*/
//	ADC_CHANNEL_GPIOA26,		/**channel 7*/
//	ADC_CHANNEL_GPIOA27,		/**channel 8*/
//	ADC_CHANNEL_GPIOA28,		/**channel 9*/
//	ADC_CHANNEL_GPIOA29,		/**channel 10*/
//	ADC_CHANNEL_GPIOA30,		/**channel 11*/
//	ADC_CHANNEL_GPIOA31,		/**channel 12*/
//	ADC_CHANNEL_GPIOB0,			/**channel 13*/
//	ADC_CHANNEL_GPIOB1,			/**channel 14*/
//	ADC_CHANNEL_POWERKEY		/**channel 15*/

//获取相对电压采样值，为采样数值
void SarADC_DCSingleMode(void)
{
	uint16_t DC_Data;
	uint32_t i;

	DBG("SarADC DC example\n");
	//GPIOA23做为ADC采样输入引脚
	//BP10开发板上按下不同的按键串口会打印输出不同的电压采样值
	SarADC_Init();
	//GPIO_RegOneBitSet(GPIO_A_ANA_EN, GPIO_INDEX26);//channel

	while(1)
	{
		GPIO_RegOneBitClear(GPIO_A_ANA_EN, GPIO_INDEX20);//channel
		GPIO_RegOneBitSet(GPIO_A_ANA_EN, GPIO_INDEX23);//channel
		DC_Data = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOA20_A23);
		DBG("DC_Data = %d\n",DC_Data);
		for(i=0; i < 0x1000000; i++);//仅仅用于延时，串口打印显示看的清楚

		GPIO_RegOneBitClear(GPIO_A_ANA_EN, GPIO_INDEX23);//channel
		GPIO_RegOneBitSet(GPIO_A_ANA_EN, GPIO_INDEX20);//channel
		DC_Data = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOA20_A23);
		DBG("DC_Data = %d\n",DC_Data);
		for(i=0; i < 0x1000000; i++);//仅仅用于延时，串口打印显示看的清楚
	}
}

//LDOIN采样为绝对电压,返回数值已经转换为电压值，单位mv
void SarADC_DCSingleMode_LDOIN(void)
{
	uint32_t i;
	uint16_t DC_Data;

	DBG("SarADC DC example for LDO\n");

	SarADC_Init();

	while(1)
	{
		DC_Data = SarADC_LDOINVolGet();
		DBG("LDO: = %d\n",DC_Data);
		for(i=0; i < 0x1000000; i++);//仅仅用于延时，串口打印显示看的清楚
	}
}

//获取Powerkey电压采样值，为采样数值
void SarADC_DCSingleMode_PowerKey(void)
{
	uint16_t DC_Data;
	uint32_t i;

	DBG("SarADC DC example\n");
	//GPIOA23做为ADC采样输入引脚
	//BP10开发板上按下不同的按键串口会打印输出不同的电压采样值
	SarADC_Init();//PowerKey 初始化已经配置
	//GPIO_RegOneBitSet(GPIO_A_ANA_EN, GPIO_INDEX26);//channel

	while(1)
	{
		DC_Data = ADC_SingleModeDataGet(ADC_CHANNEL_POWERKEY);
		DBG("DC_Data = %d\n",DC_Data);
		for(i=0; i < 0x1000000; i++);//仅仅用于延时，串口打印显示看的清楚
	}
}

static uint16_t DcData[100] = {0};
uint32_t DataDoneFlag;
static void InterruptCallBack()//0
{
	DMA_InterruptFlagClear(PERIPHERAL_ID_ADC, DMA_DONE_INT);
	ADC_ContinuModeStop();
	DataDoneFlag = TRUE;
}

void SarADC_DcContinuousModeWithDMA(void)
{
	uint16_t i;

	DataDoneFlag = FALSE;
	DMA_ChannelAllocTableSet(DmaChannelMap);

	ADC_ClockDivSet(12);
	ADC_VrefSet(1);//1:VDDA; 0:VDD
	ADC_Enable();
	ADC_ModeSet(ADC_CON_CONTINUA);

	DMA_ChannelDisable(PERIPHERAL_ID_ADC);
	DMA_BlockConfig(PERIPHERAL_ID_ADC);
	GIE_ENABLE();
	DMA_InterruptFunSet(PERIPHERAL_ID_ADC, DMA_DONE_INT, InterruptCallBack);
	DMA_InterruptEnable(PERIPHERAL_ID_ADC, DMA_DONE_INT, 1);

	ADC_DMAEnable();
	DMA_BlockBufSet(PERIPHERAL_ID_ADC, DcData, sizeof(DcData));
	DMA_ChannelEnable(PERIPHERAL_ID_ADC);

	GPIO_RegOneBitSet(GPIO_A_ANA_EN, GPIO_INDEX26);//channel
	ADC_ContinuModeStart(ADC_CHANNEL_GPIOA26);
	while(1)
	{
		if(DataDoneFlag == TRUE)
		{
			for(i=0; i<sizeof(DcData)/2; i++)
			{
				DBG("%d, 0x%x\n", i, DcData[i]);
			}
			break;
		}
	}
	DBG("test end\n");
}

int main(void)
{
	uint8_t Key;

	Chip_Init(1);
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
    DBG("|                     ADC Example                     |\n");
    DBG("|      Mountain View Silicon Technology Co.,Ltd.      |\n");
    DBG("\\-----------------------------------------------------/\n");
    DBG("\n");

    DBG("please press any key\n");
    while(1)
    {
    	if(UARTS_Recv(1, &Key, 1,100) > 0)
		{
			switch(Key)
			{
			case 'A':
			case 'a':
				SarADC_DCSingleMode();//单通道电压采样
				break;
			case 'B':
			case 'b':
				SarADC_DCSingleMode_LDOIN();//电源电压采样
				break;
			case 'C':
			case 'c':
				SarADC_DCSingleMode_PowerKey();//PowerKey复用功能
				break;
			case 'D':
			case 'd':
				SarADC_DcContinuousModeWithDMA();
				break;
			default:
				break;
			}
		}
    }
    DBG("Err!\n");
	while(1);
}

//__attribute__((section(".driver.isr"))) void BT_Interrupt(void)
//{
//}
//
//__attribute__((section(".driver.isr"))) void BLE_Interrupt(void)
//{
//}
//
//__attribute__((section(".driver.isr"))) void IRInterrupt(void)
//{
//}
