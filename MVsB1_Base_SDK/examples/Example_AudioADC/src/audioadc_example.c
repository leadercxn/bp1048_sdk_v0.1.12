/**
 **************************************************************************************
 * @file    AudioADC_example.c
 * @brief   AudioADC example
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2019-10-30 11:30:00$
 *
 * @copyright Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <stdlib.h>
#include <nds32_intrinsic.h>
//#include <string.h>
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
#include "dac_interface.h"

//定义2个全局buf，用于缓存ADC和DAC的数据，注意单位
uint32_t AudioADC1Buf[1024] = {0}; //1024 * 4 = 4K
uint32_t AudioADC2Buf[1024] = {0}; //1024 * 4 = 4K
uint32_t AudioDACBuf[1024] = {0}; //1024 * 4 = 4K

static uint32_t PcmBuf1[512] = {0};
static uint32_t PcmBuf2[512] = {0};
static uint32_t PcmBuf3[512] = {0};

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

static void AudioMicExample(void)
{
    uint32_t  SampleRate = 44100;
    uint16_t  RealLen;
    uint16_t  n;

	//DAC init
	AudioDAC_Init(DAC0, SampleRate, (void*)AudioDACBuf, sizeof(AudioDACBuf), NULL, 0);

	//Mic1 Mic2  analog
	AudioADC_AnaInit();


	AudioADC_MicBias1Enable(TRUE);
	//AudioADC_VcomConfig(1);//MicBias en
	//模拟通道先配置为NONE，防止上次配置通道残留，然后再配置需要的模拟通道

	AudioADC_DynamicElementMatch(ADC1_MODULE, TRUE, TRUE);

	AudioADC_PGASel(ADC1_MODULE, CHANNEL_RIGHT, LINEIN_NONE);
	AudioADC_PGASel(ADC1_MODULE, CHANNEL_LEFT, LINEIN_NONE);

	AudioADC_PGASel(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2);
	AudioADC_PGASel(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1);

	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2, 15, 4);
	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1, 15, 4);

	//Mic1 Mic2  digital
	AudioADC_DigitalInit(ADC1_MODULE, SampleRate, (void*)AudioADC1Buf, sizeof(AudioADC1Buf));

	while(1)
	{
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

static void AudioLineInExample()
{
    uint32_t  SampleRate = 48000;
    uint16_t  RealLen;
    uint16_t  n;

	//DAC init
	AudioDAC_Init(DAC0, SampleRate, (void*)AudioDACBuf, sizeof(AudioDACBuf), NULL, 0);

	//LineIn1  analog
	AudioADC_AnaInit();

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

	while(1)
	{
		if(AudioADC_DataLenGet(ADC0_MODULE) >= 256)
		{
			n = AudioDAC_DataSpaceLenGet(DAC0);
			RealLen = AudioADC_DataGet(ADC0_MODULE, PcmBuf1, 256);
			if(n > RealLen)
			{
				n = RealLen;
			}
			AudioDAC_DataSet(DAC0, PcmBuf1, n);
		}
	}
}

static void AudioMixerExample(void)
{
	uint32_t  SampleRate = 48000;
	uint16_t  RealLen;
	uint16_t  n;
	uint32_t  cnt;

	//DAC init
	AudioDAC_Init(DAC0, SampleRate, (void*)AudioDACBuf, sizeof(AudioDACBuf), NULL, 0);

	//ADC
	//LineIn1  analog
	AudioADC_AnaInit();

	AudioADC_DynamicElementMatch(ADC0_MODULE, TRUE, TRUE);

	//模拟通道先配置为NONE，防止上次配置通道残留，然后再配置需要的模拟通道
	AudioADC_PGASel(ADC0_MODULE, CHANNEL_RIGHT, LINEIN_NONE);
	AudioADC_PGASel(ADC0_MODULE, CHANNEL_LEFT, LINEIN_NONE);

	AudioADC_PGASel(ADC0_MODULE, CHANNEL_RIGHT, LINEIN1_RIGHT);
	AudioADC_PGASel(ADC0_MODULE, CHANNEL_LEFT, LINEIN1_LEFT);

	AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, LINEIN1_RIGHT, 12, 4);
	AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT, LINEIN1_LEFT, 12, 4);
	//LineIn2  digital
	AudioADC_DigitalInit(ADC0_MODULE, SampleRate, (void*)AudioADC1Buf, sizeof(AudioADC1Buf));

	//DAC init
	AudioDAC_Init(DAC0, SampleRate, (void*)AudioDACBuf, sizeof(AudioDACBuf), NULL, 0);


	AudioADC_MicBias1Enable(TRUE);

	AudioADC_DynamicElementMatch(ADC1_MODULE, TRUE, TRUE);

	//模拟通道先配置为NONE，防止上次配置通道残留，然后再配置需要的模拟通道
	AudioADC_PGASel(ADC1_MODULE, CHANNEL_RIGHT, LINEIN_NONE);
	AudioADC_PGASel(ADC1_MODULE, CHANNEL_LEFT, LINEIN_NONE);

	AudioADC_PGASel(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2);
	AudioADC_PGASel(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1);

	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2, 15, 4);
	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1, 15, 4);
	//Mic1 Mic2  digital
	AudioADC_DigitalInit(ADC1_MODULE, SampleRate, (void*)AudioADC2Buf, sizeof(AudioADC2Buf));

	while(1)
	{
		if(AudioADC_DataLenGet(ADC1_MODULE) >= 256)
		{
			n = AudioDAC_DataSpaceLenGet(DAC0);
			RealLen = AudioADC_DataGet(ADC1_MODULE, PcmBuf1, 256);
			if(n > RealLen)
			{
				n = RealLen;
			}

			RealLen = AudioADC_DataGet(ADC0_MODULE, PcmBuf2, 256);
			if(n > RealLen)
			{
				n = RealLen;
			}
			for(cnt = 0; cnt< n; cnt++)
			{
				PcmBuf3[cnt] = PcmBuf1[cnt] + PcmBuf2[cnt];
			}
			AudioDAC_DataSet(DAC0, PcmBuf3, n);
		}
	}
}
//ADC演示工程，主要演示ADC配置流程
//1: MIC1,2通路配置（ASDM1）
//2: LineIn2通路配置（ASDM1）
//3: MIC1,2（ASDM1） + LineIn2（ASDM0） 通路,2路 mixer之后DAC0输出
int main(void)
{
	uint8_t Key = 0;

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

	Remap_InitTcm(0, 12);
	SpiFlashInit(80000000, MODE_4BIT, 0, 1);
	DMA_ChannelAllocTableSet(DmaChannelMap);

	DBG("\n");
  DBG("/-----------------------------------------------------\\\n");
  DBG("|                     AudioADC Example                     |\n");
  DBG("|      Mountain View Silicon Technology Co.,Ltd.      |\n");
  DBG("\\-----------------------------------------------------/\n");
  DBG("\n");
  DBG("please choose the channel:\nM: MIC;\nL: LINEIN;\nX: Mixer(LINEIN + MIC)\n");

    while(1)
    {
    	if(UART1_RecvByte(&Key))
			{
				switch(Key)
				{
					case 'M':
					case 'm':
						DBG("MIC ---> DAC\n");
						AudioMicExample();
						break;
					case 'L':
					case 'l':
						DBG("LineIn ---> DAC\n");
						AudioLineInExample();
						break;
					case 'X':
					case 'x':
						DBG("LineIn + Mic ---> DAC\n");
						AudioMixerExample();
						break;
					default:
						break;
				}
			}
    }
	while(1);
}
