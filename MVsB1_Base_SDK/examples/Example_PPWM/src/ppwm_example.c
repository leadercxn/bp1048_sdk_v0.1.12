/**
 **************************************************************************************
 * @file    ppwm_example.c
 * @brief   ppwm example
 *
 * @author
 * @version V1.0.0
 *
 * $Created: 2019-05-30 11:30:00$
 *
 * @copyright Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <stdlib.h>
#include <nds32_intrinsic.h>
#include "uarts.h"
#include "uarts_interface.h"
#include "type.h"
#include "debug.h"
#include "timeout.h"
#include "clk.h"
#include "dma.h"
#include "dac.h"
#include "timer.h"
#include "adc.h"
#include "i2s.h"
#include "watchdog.h"
#include "spi_flash.h"
#include "remap.h"
#include "audio_adc.h"
#include "gpio.h"
#include "chip_info.h"
#include "adc_interface.h"
#include "ppwm_interface.h"

//定义2个全局buf，用于缓存ADC和DAC的数据，注意单位
uint32_t AudioADC1Buf[1024] = {0};
uint32_t PPWMBuf[1024] = {0};

static uint32_t PcmBuf1[512] = {0};
static int16_t  PcmBuf2[512] = {0};

static uint8_t DmaChannelMap[29] = {
	255,//PERIPHERAL_ID_SPIS_RX = 0,		//0
	255,//PERIPHERAL_ID_SPIS_TX,			//1
	255,//PERIPHERAL_ID_TIMER3,			//2
	255,//PERIPHERAL_ID_SDIO_RX,			//3
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
	0,//PERIPHERAL_ID_AUDIO_ADC0_RX,		//18
	255,//PERIPHERAL_ID_AUDIO_ADC1_RX,		//19
	255,//PERIPHERAL_ID_AUDIO_DAC0_TX,		//20
	255,//PERIPHERAL_ID_AUDIO_DAC1_TX,		//21
	255,//PERIPHERAL_ID_I2S0_RX,			//22
	255,//PERIPHERAL_ID_I2S0_TX,			//23
	255,//PERIPHERAL_ID_I2S1_RX,			//24
	255,//PERIPHERAL_ID_I2S1_TX,			//25
	1,//PERIPHERAL_ID_PPWM,				//26
	255,//PERIPHERAL_ID_ADC,     			//27
	255,//PERIPHERAL_ID_SOFTWARE,			//28
};

static void AudioLineIn2PPWMExample(void)
{
    uint32_t  SampleRate = 48000;
    uint16_t  RealLen;
    uint16_t  n,i;

	//LineIn1  analog
	AudioADC_AnaInit();

	AudioADC_VcomConfig(1);

	AudioADC_DynamicElementMatch(ADC0_MODULE, TRUE, TRUE);

	//模拟通道先配置为NONE，防止上次配置通道残留，然后再配置需要的模拟通道
	AudioADC_PGASel(ADC0_MODULE, CHANNEL_RIGHT, LINEIN_NONE);
	AudioADC_PGASel(ADC0_MODULE, CHANNEL_LEFT, LINEIN_NONE);

	AudioADC_PGASel(ADC0_MODULE, CHANNEL_RIGHT, LINEIN1_RIGHT);
	AudioADC_PGASel(ADC0_MODULE, CHANNEL_LEFT, LINEIN1_LEFT);

	AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, LINEIN1_RIGHT, 12, 4);
	AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT, LINEIN1_LEFT, 12, 4);

	//LineIn1  digital
	AudioADC_DigitalInit(ADC0_MODULE, SampleRate, (void*)AudioADC1Buf, sizeof(AudioADC1Buf));

	GPIO_PortBModeSet(GPIOB4, 2);
	GPIO_PortBModeSet(GPIOB5, 2);
	AudioPPWM_Init(SampleRate, (void*)PPWMBuf, sizeof(PPWMBuf));

	while(1)
	{
		if(AudioADC_DataLenGet(ADC0_MODULE) >= 256)
		{
			n = AudioPPWM_DataSpaceLenGet();
			RealLen = AudioADC_DataGet(ADC0_MODULE, PcmBuf1, 256);
			if(n > RealLen)
			{
				n = RealLen;
			}
			for(i=0; i<n; i++)
			{
				PcmBuf2[i] = (int16_t)PcmBuf1[i];//双声道转单声道
			}
			AudioPPWM_DataSet(PcmBuf2, n);
		}
	}
}
void SysTickInit(void);
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
	DMA_ChannelAllocTableSet(DmaChannelMap);

	DBG("\n");
    DBG("/-----------------------------------------------------\\\n");
    DBG("|                     PPWM Example                     |\n");
    DBG("|      Mountain View Silicon Technology Co.,Ltd.      |\n");
    DBG("\\-----------------------------------------------------/\n");
    DBG("\n");

	DBG("LineIn ---> PPWM\n");
	AudioLineIn2PPWMExample();
	while(1);
}
