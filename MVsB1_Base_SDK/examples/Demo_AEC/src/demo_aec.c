/**
 **************************************************************************************
 * @file    Demo_AEC.c
 * @brief
 *
 * @author  castle
 * @version V1.0.0
 *
 * $Created: 2018-6-7 19:30:00$
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

#include "blue_aec.h"
#include "blue_ns_core.h"

#include "mvintrinsics.h"
#include "typedefine.h"

#include "mvstdio.h"

//定义2个全局buf，用于DMA缓存ADC和DAC的数据，注意单位
uint32_t AudioADC1Buf[1024*4] = {0}; //1024 * 4 = 4K
uint32_t AudioADC2Buf[1024*4] = {0}; //1024 * 4 = 4K
uint32_t AudioDAC0Buf[1024*4] = {0}; //1024 * 4 = 4K
uint32_t AudioDAC1Buf[1024*4] = {0}; //1024 * 4 = 4K


static uint32_t PcmBuf1[512] = {0};
static uint32_t PcmBuf2[512] = {0};
static uint32_t DAC0PcmBuf[512] = {0};//output for DAC0
static uint32_t DAC1PcmBuf[512] = {0};//output for DAC1

static int16_t AecBuf1[512] = {0};
static int16_t AecBuf2[512] = {0};
static int16_t AecBuf3[512] = {0};

#define FRAME_SIZE BLK_LEN
#define AEC_SAMPLE_RATE 16000
#define LEN_PER_SAMPLE 4

BlueAECContext  Aecct;

#define MAX_DELAY_BLOCK 32
#define DEFAULT_DELAY_BLK 2
int8_t cur_delay_block = DEFAULT_DELAY_BLK;

uint8_t delay_buf[BLK_LEN*LEN_PER_SAMPLE*MAX_DELAY_BLOCK] = {0};
MemHandle delay_ring_buf = {delay_buf, sizeof(delay_buf)-1, BLK_LEN*LEN_PER_SAMPLE*DEFAULT_DELAY_BLK, 0};


static uint8_t DmaChannelMap[29] = {
	255,//PERIPHERAL_ID_SPIS_RX = 0,		//0
	255,//PERIPHERAL_ID_SPIS_TX,			//1
	255,//PERIPHERAL_ID_TIMER3,			//2
	255,//PERIPHERAL_ID_SDIO_RX,			//3
	255,//PERIPHERAL_ID_SDIO_TX,			//4
	255,//PERIPHERAL_ID_UART0_RX,			//5
	255,//PERIPHERAL_ID_TIMER1,				//6
	255,//PERIPHERAL_ID_TIMER2,				//7
	255,//PERIPHERAL_ID_SDPIF_RX,			//8 SPDIF_RX /TXDèê1ó?í?ò?í¨μà
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


/*
* ADC0 for reference, ADC1 for mic, DAC0 & DAC1 for showing output
* monitor_mode = 0: ADC0(L+R)-->DAC1(L)
*					ADC1(L+R)-->DAC0(L)
*					AEC(mono)-->DAC0(R)
* monitor_mode = 1: ADC0(L+R)-->DAC0(L)
*					ADC1(L+R)-->DAC1(L) to see delay between reference and mic voice
*/
uint8_t monitor_mode = 0;

bool if_do_aec = TRUE;

static void AECExample(void)
{
	uint8_t key;

	//DAC init
	AudioDAC_Init(ALL, AEC_SAMPLE_RATE, (void*)AudioDAC0Buf, sizeof(AudioDAC0Buf), (void*)AudioDAC1Buf, sizeof(AudioDAC1Buf));

	//ADC
	//LineIn1  analog
	AudioADC_AnaInit();
	//模拟通道先配置为NONE，防止上次配置通道残留，然后再配置需要的模拟通道
	AudioADC_PGASel(ADC0_MODULE, CHANNEL_RIGHT, LINEIN_NONE);
	AudioADC_PGASel(ADC0_MODULE, CHANNEL_LEFT, LINEIN_NONE);

	AudioADC_PGASel(ADC0_MODULE, CHANNEL_RIGHT, LINEIN1_RIGHT);
	AudioADC_PGASel(ADC0_MODULE, CHANNEL_LEFT, LINEIN1_LEFT);
	AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, LINEIN1_RIGHT, 0x0c, 4);
	AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT, LINEIN1_LEFT, 0x0c, 4);
	//LineIn2  digital
	AudioADC_DigitalInit(ADC0_MODULE, AEC_SAMPLE_RATE, (void*)AudioADC1Buf, sizeof(AudioADC1Buf));

	//mic内部偏置选择，和硬件配置有关系，需要根据实际情况选择
	//AudioADC_MicBias1Enable(TRUE);
	//AudioADC_MicBias2Enable(TRUE);
	AudioADC_VcomConfig(1);//MicBias en
	//模拟通道先配置为NONE，防止上次配置通道残留，然后再配置需要的模拟通道
	AudioADC_PGASel(ADC1_MODULE, CHANNEL_RIGHT, LINEIN_NONE);
	AudioADC_PGASel(ADC1_MODULE, CHANNEL_LEFT, LINEIN_NONE);

	AudioADC_PGASel(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2);
	AudioADC_PGASel(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1);

	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2, 0, 4);
	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1, 0, 4);
	//Mic1 Mic2  digital
	AudioADC_DigitalInit(ADC1_MODULE, AEC_SAMPLE_RATE, (void*)AudioADC2Buf, sizeof(AudioADC2Buf));

	blue_aec_init(&Aecct, 0, 0);

	AudioADC_DMARestart(ADC0_MODULE, AudioADC1Buf, sizeof(AudioADC1Buf));
	AudioADC_DMARestart(ADC1_MODULE, AudioADC2Buf, sizeof(AudioADC2Buf));

	while(1)
	{
		if(AudioADC_DataLenGet(ADC1_MODULE) > FRAME_SIZE && AudioADC_DataLenGet(ADC0_MODULE) > FRAME_SIZE)
		{
			int16_t i, *one_sample;

			//get voice on MIC1
			AudioADC_DataGet(ADC1_MODULE, PcmBuf1, FRAME_SIZE);

			//push the new data, pop the old one
			mv_mwrite(PcmBuf1, 1, FRAME_SIZE*LEN_PER_SAMPLE, &delay_ring_buf);
			mv_mread(PcmBuf1, 1, FRAME_SIZE*LEN_PER_SAMPLE, &delay_ring_buf);

			//stereo to mono
			one_sample = (int16_t*)PcmBuf1;
			for(i = 0; i < FRAME_SIZE; i++)
			{
				AecBuf1[i] = CLIP_16BIT((int)*one_sample + (int)*(one_sample+1));
				if(monitor_mode == 0)
				{
					((int16_t*)DAC0PcmBuf)[i*2] = AecBuf1[i];//DAC0_L
				}
				else if(monitor_mode == 1)
				{
					((int16_t*)DAC0PcmBuf)[i*2] = AecBuf1[i];//DAC0_L
				}
				one_sample +=2;
			}

			//get reference on LineIn1
			AudioADC_DataGet(ADC0_MODULE, PcmBuf2, FRAME_SIZE);

			//stereo to mono
			one_sample = (int16_t*)PcmBuf2;
			for(i = 0; i < FRAME_SIZE; i++)
			{
				AecBuf2[i] = CLIP_16BIT((int)*one_sample + (int)*(one_sample+1));
				if(monitor_mode == 0)
				{
					DAC1PcmBuf[i] = AecBuf2[i];
				}
				else if(monitor_mode == 1)
				{
					((int16_t*)DAC0PcmBuf)[i*2+1] = AecBuf2[i];//DAC0_R
				}
				one_sample +=2;
			}

			//DO or NOT do AEC
			if(if_do_aec)
				blue_aec_run(&Aecct, AecBuf2, AecBuf1, AecBuf3);
			else
				memcpy(AecBuf3, AecBuf1, FRAME_SIZE*LEN_PER_SAMPLE);

			if(monitor_mode == 0)
			{
				for(i = 0; i < FRAME_SIZE; i++)
				{
					((int16_t*)DAC0PcmBuf)[i*2+1] = AecBuf3[i];//DAC0_R
				}
			}
			else if(monitor_mode == 1)
			{

			}

			//output result, or reference
			AudioDAC_DataSet(DAC0, DAC0PcmBuf, FRAME_SIZE);
			AudioDAC_DataSet(DAC1, DAC1PcmBuf, FRAME_SIZE);

		}

		if(UARTS_RecvByte(1, &key))
		{
			switch(key)
			{
				case 'n':
					if(cur_delay_block > 0)
					{
						cur_delay_block--;
						DBG("delay_blockds=%d\n", cur_delay_block);
					}
					else
					{
						cur_delay_block = MAX_DELAY_BLOCK - 1;
						DBG("delay_blockds=%d\n", cur_delay_block);
					}
					delay_ring_buf.mem_len = BLK_LEN*LEN_PER_SAMPLE*cur_delay_block;
					delay_ring_buf.p = 0;
					AudioADC_DMARestart(ADC0_MODULE, AudioADC1Buf, sizeof(AudioADC1Buf));
					AudioADC_DMARestart(ADC1_MODULE, AudioADC2Buf, sizeof(AudioADC2Buf));
					break;

				case 'm':
					if(cur_delay_block < MAX_DELAY_BLOCK-2)
					{
						cur_delay_block++;
						DBG("delay_blockds=%d\n", cur_delay_block);
					}
					else
					{
						cur_delay_block = 0;
						DBG("delay_blockds=%d\n", cur_delay_block);
					}
					delay_ring_buf.mem_len = BLK_LEN*LEN_PER_SAMPLE*cur_delay_block;
					delay_ring_buf.p = 0;
					AudioADC_DMARestart(ADC0_MODULE, AudioADC1Buf, sizeof(AudioADC1Buf));
					AudioADC_DMARestart(ADC1_MODULE, AudioADC2Buf, sizeof(AudioADC2Buf));
					break;

				case 'a':
					if_do_aec =!if_do_aec;
					if(if_do_aec)
						DBG("do aec\n");
					else
						DBG("not do aec\n");
					break;

				default:
					break;
			}
		}
	}
}

//demo aec演示工程，主要演示aec的用法
//ADC0 作为参考源输入, ADC1 作为mic语音输入,
//DAC0 & DAC1 用以输出和监视, 参考monitor_mode
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

	//SysTickInit();

	Remap_InitTcm(0, 12);
	SpiFlashInit(80000000, MODE_4BIT, 0, 1);

	DMA_ChannelAllocTableSet(DmaChannelMap);

	DBG("\n");
    DBG("/-----------------------------------------------------\\\n");
    DBG("|                      AEC Demo                       |\n");
    DBG("|      Mountain View Silicon Technology Co.,Ltd.      |\n");
    DBG("\\-----------------------------------------------------/\n");
    DBG("key m to increase delay\n");
	DBG("key n to reduce delay\n");
	DBG("key a to on/off aec function\n");

	AECExample();

	while(1);
}
