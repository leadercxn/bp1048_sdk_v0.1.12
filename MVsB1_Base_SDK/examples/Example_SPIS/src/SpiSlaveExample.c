/**
 **************************************************************************************
 * @file    SpiSlaveExample.c
 * @brief   Spi Slave
 * 
 * @author  grayson
 * @version V1.0.0 	initial release
 * 
 * $Id$
 * $Created: 2017-10-29 17:31:10$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include <nds32_intrinsic.h>
#include "gpio.h"
#include "uarts.h"
#include "uarts_interface.h"
#include "type.h"
#include "debug.h"
#include "spi_flash.h"
#include "timeout.h"
#include "clk.h"
#include "pwm.h"
#include "delay.h"
#include "rtc.h"
#include "spis.h"
#include "watchdog.h"
#include "irqn.h"
#include "spi_flash.h"
#include "remap.h"
#include "chip_info.h"

extern void SysTickInit(void);

#define LIMIT(Val,lmt)   (((Val) > (lmt)) ? (lmt):(Val) )

#define MAX_BUF_LEN   4096

uint8_t spisPort = SPIS_PORT1_A20A21A22A23;
uint8_t spisMode = 0;

uint8_t SpisSendBuf[MAX_BUF_LEN];
uint8_t SpisRecvBuf[MAX_BUF_LEN];



uint8_t DmaChannelMapSPIS[28] =
{
		1,//PERIPHERAL_ID_SPIS_RX = 0,		    //0
		2,//PERIPHERAL_ID_SPIS_TX,			    //1
		255,//PERIPHERAL_ID_TIMER3,				//2
		255,//PERIPHERAL_ID_SDIO0_RX,			//3
		255,//PERIPHERAL_ID_SDIO0_TX,			//4
		255,//PERIPHERAL_ID_UART0_RX,			//5
		255,//PERIPHERAL_ID_SDIO1_RX,			//6
		255,//PERIPHERAL_ID_SDIO1_TX,			//7
		255,//PERIPHERAL_ID_SDPIF_RX,			//8 SPDIF_RX /TX需使用同一通道
		255,//PERIPHERAL_ID_SDPIF_TX,			//9
		255,//PERIPHERAL_ID_SPIM_RX,			//10
		255,//PERIPHERAL_ID_SPIM_TX,		    //11
		255,//PERIPHERAL_ID_UART0_TX,			//12
		255,//PERIPHERAL_ID_UART1_RX,			//13
		255,//PERIPHERAL_ID_UART1_TX,			//14
		255,//PERIPHERAL_ID_PSRAM_CONTROL_TX,	//15
		255,//PERIPHERAL_ID_PSRAM_CONTROL_RX,	//16
		255,//PERIPHERAL_ID_TIMER4,				//17
		255,//PERIPHERAL_ID_AUDIO_ADC0_RX,		//18
		255,//PERIPHERAL_ID_AUDIO_ADC1_RX,		//19
		255,//PERIPHERAL_ID_AUDIO_DAC0_TX,		//20
		255,//PERIPHERAL_ID_AUDIO_DAC1_TX,		//21
		255,//PERIPHERAL_ID_I2S0_TX,			//22
		255,//PERIPHERAL_ID_I2S0_RX,			//23
		255,//PERIPHERAL_ID_I2S1_TX,			//24
		255,//PERIPHERAL_ID_I2S1_RX,			//25
		255,//PERIPHERAL_ID_ADC,     			//26
		255,//PERIPHERAL_ID_SOFTWARE,			//27
};


static int32_t Wait4Datum4Ever(void)//从串口等待接收一个字节
{
	uint8_t Data;
	do{
		if(0 < UART1_Recv(&Data, 1,10))
        {
			break;
		}
	}while(1);
	return Data - '0';
}


const char* spisIO[][4] =
{
	//    cs      miso     clk      mosi
		{"A3",    "A2",    "A1",     "A0"},
		{"A23",   "A22",   "A21",    "A20"},
};

void SpisSetting(void)
{
	DBG("\nSPIS init setting...\n");

	SPIS_IoConfig(spisPort);
	SPIS_Init(spisMode);
	SPIS_BlockDMA_Init();
	DBG("spis mode: %d\n",spisMode);//
	DBG("spis_cs  : %s\n",spisIO[spisPort][0]);
	DBG("spis_miso: %s\n",spisIO[spisPort][1]);
	DBG("spis_clk : %s\n",spisIO[spisPort][2]);
	DBG("spis_mosi: %s\n",spisIO[spisPort][3]);
}




int main(void)
{
	uint8_t Temp;
	uint32_t i;
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

	SysTickInit();

	Remap_InitTcm(0, 12);
	SpiFlashInit(80000000, MODE_4BIT, 0, 1);

	DBG("\n");
    DBG("/-----------------------------------------------------\\\n");
    DBG("|                    SPIS Example                     |\n");
    DBG("|      Mountain View Silicon Technology Co.,Ltd.      |\n");
    DBG("\\-----------------------------------------------------/\n");
    DBG("\n");

	for(i = 0; i< MAX_BUF_LEN;i++)
	{
		SpisSendBuf[i]=(uint8_t)i;
	}
	DMA_ChannelAllocTableSet(DmaChannelMapSPIS);
	SpisSetting();//spis配置
	while(1)
	{
		DBG("\n=============================================\n");
		DBG("\n SPIS Example...\n");
		DBG(" 1. SPIS TX by MCU\n");
		DBG(" 2. SPIS RX by MCU\n");
		DBG(" 3. SPIS TX by DMA\n");
		DBG(" 4. SPIS RX by DMA\n");
		DBG(" 5. SPIS TX&RX by DMA\n");
		DBG("\n=============================================\n");
		Temp = Wait4Datum4Ever();//注意UART端口号
		switch(Temp)
		{
			case 1:
				DBG("TX MCU\n");
				 SPIS_ClrTxFIFO();
				SPIS_Send(SpisSendBuf,4096,4000);
				DBG("send ok\n");
				break;
			case 2:
				DBG("RX MCU\n");
				 SPIS_ClrRxFIFO();
				SPIS_Recv(SpisRecvBuf,4096,4000);
				if(memcmp(SpisSendBuf,SpisRecvBuf,4096)==0)
				{
					DBG("recv ok\n");
				}
				for(i = 0;i < LIMIT(MAX_BUF_LEN,128);i++)//最多只打印128个值
				{
					DBG("[%02x]  ", SpisRecvBuf[i]);
				}
				break;
				break;
			case 3:
				DBG("TX DMA\n");
				 SPIS_ClrTxFIFO();
				SPIS_DMA_StartSend(SpisSendBuf,4096);
				while(!SPIS_DMA_SendState());
				DBG("send ok\n");
				break;

			case 4:
				DBG("RX DMA\n");
				 SPIS_ClrRxFIFO();
				SPIS_DMA_StartRecv(SpisRecvBuf,4096);
				while(!SPIS_DMA_RecvState());
				if(memcmp(SpisSendBuf,SpisRecvBuf,4096)==0)
				{
					DBG("recv ok\n");
				}
				for(i = 0;i < LIMIT(MAX_BUF_LEN,128);i++)//最多只打印128个值
				{
					DBG("[%02x]  ", SpisRecvBuf[i]);
				}
				break;
			case 5:
				DBG("TX&RX DMA\n");
				 SPIS_ClrRxFIFO();
				 SPIS_ClrTxFIFO();
				SPIS_DMA_StartSendRecv(SpisSendBuf,SpisRecvBuf,4096);
				while(!SPIS_DMA_RecvState());
				if(memcmp(SpisSendBuf,SpisRecvBuf,4096)==0)
				{
					DBG("recv ok\n");
				}
				for(i = 0;i < LIMIT(MAX_BUF_LEN,128);i++)//最多只打印128个值
				{
					DBG("[%02x]  ", SpisRecvBuf[i]);
				}
				break;
			default :break;
		}
	}
}

