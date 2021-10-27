/**
 **************************************************************************************
 * @file    voice_agc_demo.c
 * @brief
 *
 * @author  pi
 * @version V1.0.0
 *
 * $Created: 2019-05-14 11:30:00$
 *
 * @copyright Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <stdlib.h>
#include <nds32_intrinsic.h>
#include "uarts.h"
#include "sys.h"
#include "uarts_interface.h"
#include "type.h"
#include "debug.h"
#include "timeout.h"
#include "clk.h"
#include "dma.h"
#include "dac.h"
#include "timer.h"
#include "adc.h"
#include "watchdog.h"
#include "spi_flash.h"
#include "remap.h"
#include "audio_adc.h"
#include "gpio.h"
#include "chip_info.h"
#include "irqn.h"
#include "adc_interface.h"
#include "dac_interface.h"

//定义2个全局buf，用于缓存ADC和DAC的数据，注意单位
uint32_t AudioADCBuf[512] = {0}; //512 * 4 = 2K
uint32_t AudioDACBuf[512] = {0}; //512 * 4 = 2K


static uint32_t PcmBuf1[512] = {0};

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





int main(void)
{
    uint32_t  SampleRate = 16000;
    uint16_t  RealLen, n;
    uint8_t   AgcGain = 0;

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

	GIE_ENABLE();	//开启总中断

	DMA_ChannelAllocTableSet(DmaChannelMap);

	DBG("\n");
    DBG("/-----------------------------------------------------\\\n");
    DBG("|                     AudioADC AGC Example                     |\n");
    DBG("|      Mountain View Silicon Technology Co.,Ltd.      |\n");
    DBG("\\-----------------------------------------------------/\n");
    DBG("\n");

    //DAC init
    AudioDAC_Init(DAC0, SampleRate, (void*)AudioDACBuf, sizeof(AudioDACBuf), NULL, 0);

	//Mic1 Mic2  analog
	AudioADC_AnaInit();

	AudioADC_MicBias1Enable(TRUE);
	//模拟通道先配置为NONE，防止上次配置通道残留，然后再配置需要的模拟通道
	AudioADC_PGASel(ADC1_MODULE, CHANNEL_RIGHT, LINEIN_NONE);
	AudioADC_PGASel(ADC1_MODULE, CHANNEL_LEFT, LINEIN_NONE);

	AudioADC_PGASel(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2);
	AudioADC_PGASel(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1);

	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_LEFT_OR_MIC1, 15, 4);
	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_RIGHT_OR_MIC2, 15, 4);

	//Mic1 Mic2  digital
	AudioADC_DigitalInit(ADC1_MODULE, SampleRate, (void*)AudioADCBuf, sizeof(AudioADCBuf));

	AudioADC_AGCChannelSel(ADC1_MODULE, TRUE, FALSE);
	AudioADC_AGCGainOffset(ADC1_MODULE,0);
	AudioADC_AGCFrameTime(ADC1_MODULE, 96);
	AudioADC_AGCHoldTime(ADC1_MODULE, 192);
	AudioADC_AGCAttackStepTime(ADC1_MODULE, 32);
	AudioADC_AGCDecayStepTime(ADC1_MODULE, 64);
	AudioADC_AGCMaxLevel(ADC1_MODULE, 10);//?
	AudioADC_AGCTargetLevel(ADC1_MODULE,20);
	AudioADC_AGCMaxGain(ADC1_MODULE, 0x16);
	AudioADC_AGCMinGain(ADC1_MODULE, 0x3F);
	AudioADC_AGCNoiseThreshold(ADC1_MODULE, 15);
	AudioADC_AGCNoiseMode(ADC1_MODULE, 0);
	AudioADC_AGCNoiseHoldTime(ADC1_MODULE, 255);
	AudioADC_AGCNoiseGateEnable(ADC1_MODULE, TRUE);

	while(1)
	{
		if(AgcGain != AudioADC_AGCGainGet(ADC1_MODULE))
		{
			AgcGain = AudioADC_AGCGainGet(ADC1_MODULE);
			DBG("AgcGain Change:%d\n", AgcGain);
		}
		if(AudioADC_DataLenGet(ADC1_MODULE) >= 256)
		{
			n = AudioDAC_DataSpaceLenGet(DAC0);
			RealLen = AudioADC_DataGet(ADC1_MODULE, PcmBuf1, 256);

			if(n > RealLen)
			{
				n = RealLen;
			}
			AudioDAC_DataSet(DAC0, PcmBuf1, n);
		}
	}
}
