/**
 **************************************************************************************
 * @file    dac_example.c
 * @brief   dac example
 *
 * @author  Castle
 * @version V1.0.0
 *
 * $Created: 2019-11-03 19:17:00$
 *
 * @Copyright (C) 2019, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include <stdlib.h>
#include <nds32_intrinsic.h>
#include <string.h>
#include "gpio.h"
#include "watchdog.h"
#include "sys.h"
#include "uarts.h"
#include "uarts_interface.h"
#include "dac_interface.h"
#include "type.h"
#include "debug.h"
#include "timeout.h"
#include "clk.h"
#include "dma.h"
#include "timer.h"
#include "spi_flash.h"
#include "remap.h"
#include "irqn.h"
#include "dac.h"
#include "chip_info.h"

//#define TEST_24BIT


#define USER_DEFINE_MCLK_112896M_DIV_VALUE 11289600
#define USER_DEFINE_MCLK_12288M_DIV_VALUE	12288000

#define DAC_FIFO_SAMPLES		 	1024
uint32_t DAC0_FIFO[DAC_FIFO_SAMPLES];
#define DAC0_FIFO_LEN		 	sizeof(DAC0_FIFO)
uint32_t DAC1_FIFO[DAC_FIFO_SAMPLES];
#define DAC1_FIFO_LEN		 	sizeof(DAC1_FIFO)

static uint8_t DmaChannelMap[29] = {
	255,//PERIPHERAL_ID_SPIS_RX = 0,		//0
	255,//PERIPHERAL_ID_SPIS_TX,			//1
	255,//PERIPHERAL_ID_TIMER3,			//2
	255,//PERIPHERAL_ID_SDIO_RX,			//3
	255,//PERIPHERAL_ID_SDIO_TX,			//4
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
	2,//PERIPHERAL_ID_AUDIO_ADC0_RX,		//18
	3,//PERIPHERAL_ID_AUDIO_ADC1_RX,		//19
	0,//PERIPHERAL_ID_AUDIO_DAC0_TX,		//20
	1,//PERIPHERAL_ID_AUDIO_DAC1_TX,		//21
	4,//PERIPHERAL_ID_I2S0_RX,			//22
	5,//PERIPHERAL_ID_I2S0_TX,			//23
	6,//PERIPHERAL_ID_I2S1_RX,			//24
	7,//PERIPHERAL_ID_I2S1_TX,			//25
	255,//PERIPHERAL_ID_PPWM,				//26
	255,//PERIPHERAL_ID_ADC,     			//27
	255,//PERIPHERAL_ID_SOFTWARE,			//28
};

//ADC parameter config ON/OFF
#define USE_SCRAMBLE
//#define USE_DITHER
//#define USE_ZERO_DETECT
#define CHANNEL_SEL 0//0=LR,1=X,2=ALL

//clock config
static const uint32_t sr_table[]={8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000};



#ifdef TEST_24BIT
static uint32_t Sin48k_Buf_24bit[48*2] =
{
	0x00000000,
	0x00000000,
	0x00085F8D,
	0x00085F8D,
	0x00109A6E,
	0x00109A6E,
	0x00188C94,
	0x00188C94,
	0x00201333,
	0x00201333,
	0x00270D53,
	0x00270D53,
	0x002D5C64,
	0x002D5C64,
	0x0032E4C4,
	0x0032E4C4,
	0x00378E37,
	0x00378E37,
	0x003B4452,
	0x003B4452,
	0x003DF6D2,
	0x003DF6D2,
	0x003F99E8,
	0x003F99E8,
	0x00402667,
	0x00402667,
	0x003F99E8,
	0x003F99E8,
	0x003DF6D2,
	0x003DF6D2,
	0x003B4452,
	0x003B4452,
	0x00378E37,
	0x00378E37,
	0x0032E4C4,
	0x0032E4C4,
	0x002D5C64,
	0x002D5C64,
	0x00270D53,
	0x00270D53,
	0x00201333,
	0x00201333,
	0x00188C94,
	0x00188C94,
	0x00109A6E,
	0x00109A6E,
	0x00085F8D,
	0x00085F8D,
	0x00000000,
	0x00000000,
	0x00F7A073,
	0x00F7A073,
	0x00EF6592,
	0x00EF6592,
	0x00E7736C,
	0x00E7736C,
	0x00DFECCD,
	0x00DFECCD,
	0x00D8F2AD,
	0x00D8F2AD,
	0x00D2A39C,
	0x00D2A39C,
	0x00CD1B3C,
	0x00CD1B3C,
	0x00C871C9,
	0x00C871C9,
	0x00C4BBAE,
	0x00C4BBAE,
	0x00C2092E,
	0x00C2092E,
	0x00C06618,
	0x00C06618,
	0x00BFD999,
	0x00BFD999,
	0x00C06618,
	0x00C06618,
	0x00C2092E,
	0x00C2092E,
	0x00C4BBAE,
	0x00C4BBAE,
	0x00C871C9,
	0x00C871C9,
	0x00CD1B3C,
	0x00CD1B3C,
	0x00D2A39C,
	0x00D2A39C,
	0x00D8F2AD,
	0x00D8F2AD,
	0x00DFECCD,
	0x00DFECCD,
	0x00E7736C,
	0x00E7736C,
	0x00EF6592,
	0x00EF6592,
	0x00F7A073,
	0x00F7A073,
};

static uint32_t Sin48k_Buf_24_X[48] =
{
	0x00000000,
	0x00085F8D,
	0x00109A6E,
	0x00188C94,
	0x00201333,
	0x00270D53,
	0x002D5C64,
	0x0032E4C4,
	0x00378E37,
	0x003B4452,
	0x003DF6D2,
	0x003F99E8,
	0x00402667,
	0x003F99E8,
	0x003DF6D2,
	0x003B4452,
	0x00378E37,
	0x0032E4C4,
	0x002D5C64,
	0x00270D53,
	0x00201333,
	0x00188C94,
	0x00109A6E,
	0x00085F8D,
	0x00000000,
	0x00F7A073,
	0x00EF6592,
	0x00E7736C,
	0x00DFECCD,
	0x00D8F2AD,
	0x00D2A39C,
	0x00CD1B3C,
	0x00C871C9,
	0x00C4BBAE,
	0x00C2092E,
	0x00C06618,
	0x00BFD999,
	0x00C06618,
	0x00C2092E,
	0x00C4BBAE,
	0x00C871C9,
	0x00CD1B3C,
	0x00D2A39C,
	0x00D8F2AD,
	0x00DFECCD,
	0x00E7736C,
	0x00EF6592,
	0x00F7A073,
};
#else
static uint32_t Sin48k_Buf[48] =
{
	0x00010001,
	0x085b085b,
	0x10901090,
	0x187e187e,
	0x20002000,
	0x26f626f6,
	0x2d412d41,
	0x32c632c6,
	0x376d376d,
	0x3b213b21,
	0x3dd23dd2,
	0x3f743f74,
	0x40004000,
	0x3f743f74,
	0x3dd23dd2,
	0x3b213b21,
	0x376d376d,
	0x32c632c6,
	0x2d412d41,
	0x26f626f6,
	0x20002000,
	0x187e187e,
	0x10901090,
	0x085b085b,
	0x00000000,
	0xf7a5f7a5,
	0xef70ef70,
	0xe782e782,
	0xe000e000,
	0xd90ad90a,
	0xd2bfd2bf,
	0xcd3acd3a,
	0xc893c893,
	0xc4dfc4df,
	0xc22ec22e,
	0xc08cc08c,
	0xc000c000,
	0xc08cc08c,
	0xc22ec22e,
	0xc4dfc4df,
	0xc893c893,
	0xcd3acd3a,
	0xd2bfd2bf,
	0xd90ad90a,
	0xe000e000,
	0xe782e782,
	0xef70ef70,
	0xf7a5f7a5,
};

static uint16_t Sin48k_Buf_x[48] =
{
	0x0001,
	0x085b,
	0x1090,
	0x187e,
	0x2000,
	0x26f6,
	0x2d41,
	0x32c6,
	0x376d,
	0x3b21,
	0x3dd2,
	0x3f74,
	0x4000,
	0x3f74,
	0x3dd2,
	0x3b21,
	0x376d,
	0x32c6,
	0x2d41,
	0x26f6,
	0x2000,
	0x187e,
	0x1090,
	0x085b,
	0x0000,
	0xf7a5,
	0xef70,
	0xe782,
	0xe000,
	0xd90a,
	0xd2bf,
	0xcd3a,
	0xc893,
	0xc4df,
	0xc22e,
	0xc08c,
	0xc000,
	0xc08c,
	0xc22e,
	0xc4df,
	0xc893,
	0xcd3a,
	0xd2bf,
	0xd90a,
	0xe000,
	0xe782,
	0xef70,
	0xf7a5,
};
#endif

int main(void)
{
	Chip_Init(1);
	WDG_Disable();

	Clock_Module1Enable(ALL_MODULE1_CLK_SWITCH);
	Clock_Module2Enable(ALL_MODULE2_CLK_SWITCH);
	Clock_Module3Enable(ALL_MODULE3_CLK_SWITCH);

	Clock_Config(1, 24000000);
	Clock_PllLock(288000);
	Clock_APllLock(288000);
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
  DBG("|                     DAC Example                     |\n");
  DBG("|      Mountain View Silicon Technology Co.,Ltd.      |\n");
  DBG("\\-----------------------------------------------------/\n");
  DBG("\n");

	DMA_ChannelAllocTableSet(DmaChannelMap);

#ifdef TEST_24BIT
	DMA_ConfigModify(PERIPHERAL_ID_AUDIO_DAC1_TX, DMA_SRC_DWIDTH_WORD, DMA_DST_DWIDTH_WORD, DMA_SRC_AINCR_SRC_WIDTH, DMA_DST_AINCR_NO);
#endif

	AudioDAC_Init(ALL, 48000, (void*)DAC0_FIFO, DAC0_FIFO_LEN, (void*)DAC1_FIFO, DAC1_FIFO_LEN);

#ifdef TEST_24BIT
	AudioDAC_DoutModeSet(AUDIO_DAC0, MODE0, WIDTH_24_BIT_2);
	AudioDAC_DoutModeSet(AUDIO_DAC1, MODE0, WIDTH_24_BIT_2);
#endif

	while(1)
	{
		uint16_t n = AudioDAC_DataSpaceLenGet(DAC0);
#ifdef TEST_24BIT
		if(n > 96)
		{
			//DBG(".");
			AudioDAC_DataSet(DAC0, Sin48k_Buf_24bit, 48*2);
			AudioDAC_DataSet(DAC1, Sin48k_Buf_24_X, 48*2);
		}
#else
		if(n > 96)
		{
			//DBG(".");
			AudioDAC_DataSet(DAC0, Sin48k_Buf, 48);
			AudioDAC_DataSet(DAC1, Sin48k_Buf_x, 48);
		}
#endif

	}

	while(1);
}
