/**
 **************************************************************************************
 * @file    spdif_example.c
 * @brief   spdif example
 *
 * @author  Cecilia Wang
 * @version V1.0.0
 *
 * $Created: 2016-11-30 11:30:00$
 *
 * @copyright Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <stdlib.h>
#include <nds32_intrinsic.h>
#include <string.h>
#include "math.h"
#include "type.h"
#include "debug.h"
#include "delay.h"
#include "gpio.h"
#include "uarts.h"
#include "uarts_interface.h"
#include "type.h"
#include "debug.h"
#include "timeout.h"
#include "clk.h"
#include "dma.h"
#include "i2s.h"
#include "spdif.h"
#include "watchdog.h"
#include "remap.h"
#include "spi_flash.h"
#include "chip_info.h"
//#include "audio_path_cfg.h"
#include "audio_adc.h"
#include "irqn.h"

static uint8_t DmaChannelMap[28] = {
	255,//PERIPHERAL_ID_SPIS_RX = 0,		//0
	255,//PERIPHERAL_ID_SPIS_TX,			//1
	255,//PERIPHERAL_ID_TIMER3,				//2
	255,//PERIPHERAL_ID_SDIO0_RX,			//3
	255,//PERIPHERAL_ID_SDIO0_TX,			//4
	255,//PERIPHERAL_ID_UART0_RX,			//5
	255,//PERIPHERAL_ID_SDIO1_RX,			//6
	255,//PERIPHERAL_ID_SDIO1_TX,			//7
	5,//PERIPHERAL_ID_SDPIF_RX,			//8 SPDIF_RX /TX需使用同一通道
	5,//PERIPHERAL_ID_SDPIF_TX,			//9
	255,//PERIPHERAL_ID_SPIM_RX,			//10
	255,//PERIPHERAL_ID_SPIM_TX,			//11
	255,//PERIPHERAL_ID_UART0_TX,			//12
	255,//PERIPHERAL_ID_UART1_RX,			//13
	255,//PERIPHERAL_ID_UART1_TX,			//14
	255,//PERIPHERAL_ID_PSRAM_CONTROL_TX,	//15
	255,//PERIPHERAL_ID_PSRAM_CONTROL_RX,	//16
	255,//PERIPHERAL_ID_TIMER4,				//17
	6,  //PERIPHERAL_ID_AUDIO_ADC0_RX,		//18
	255,  //PERIPHERAL_ID_AUDIO_ADC1_RX,		//19
	255,  //PERIPHERAL_ID_AUDIO_DAC0_TX,		//20
	255,  //PERIPHERAL_ID_AUDIO_DAC1_TX,		//21
	1,//PERIPHERAL_ID_I2S0_TX,				//22
	2,//PERIPHERAL_ID_I2S0_RX,				//23
	3,//PERIPHERAL_ID_I2S1_TX,			//24
	4,//PERIPHERAL_ID_I2S1_RX,			//25
	255,//PERIPHERAL_ID_ADC,     			//26
	255,//PERIPHERAL_ID_SOFTWARE,			//27
};

#define I2S0_FIFO_LEN			(10 * 1024)
uint32_t I2sAddr[I2S0_FIFO_LEN/4];

#define SPDIF_FIFO_LEN				(10 * 1024)
uint32_t SpdifAddr[SPDIF_FIFO_LEN/4];

#define AUDIO_FIFO_ADDR_BASE	0x20000000
#define AUDIO_FIFO_OFFSET		0x20000
#define ASDM0_FIFO_START_ADDR 	(AUDIO_FIFO_ADDR_BASE + AUDIO_FIFO_OFFSET) //0x2000
#define ASDM0_FIFO_LEN		 	(1024 * 8)

#define ASDM1_FIFO_START_ADDR 	(ASDM0_FIFO_START_ADDR + ASDM0_FIFO_LEN)
#define ASDM1_FIFO_LEN		 	(1024 * 8)//(1024 * 8)

#define DAC_FIFO_START_ADDR 	(ASDM1_FIFO_START_ADDR + ASDM1_FIFO_LEN)
#define DAC_FIFO_LEN		 	(1024 * 8)

#define DAC0_FIFO_START_ADDR 	(ASDM1_FIFO_START_ADDR + ASDM1_FIFO_LEN)
#define DAC0_FIFO_LEN		 	(1024 * 8)

static int32_t PcmBuf[4*1024];
static int32_t SpdifBuf[4*1024];

/*static bool IntFlg = 0;
static uint16_t gCnt = 0;
static int32_t *Ptr32 = (int32_t *)SpdifBuf;
static uint16_t gm = 0;*/

static int16_t PcmBuf1[1024] = {0};
//static int16_t PcmBuf2[1024] = {0};
//static int16_t PcmBuf3[1024] = {0};
static int16_t vol_l = 0xfff;//0xfff;0xe42
static int16_t vol_r = 0xfff;//0xfff;0xe42

/*static SPDIF_INT_TYPE SpdifIntType = BEGIN_FRAME_INT;
static SPDIF_INT_TYPE SpdifIntType1 = BEGIN_FRAME_INT;
static SPDIF_INT_TYPE SpdifIntType2 = LOCK_INT;*/

//#define SAMPLERATE_ADJUST_EN

#ifdef SAMPLERATE_ADJUST_EN
static int32_t AdjustVal = 0;
static int32_t AdjustValBak = 0;
static int32_t FreeLenWarn = 0;
static int32_t PllUpDownFlag = 0xff;
#endif

uint8_t SineBuf[768] =
{
		0x01, 0x00, 0x00, 0x00, 0x5F, 0x08, 0x5F, 0x08, 0x9A, 0x10, 0x9A, 0x10, 0x8D, 0x18, 0x8D, 0x18,
		0x13, 0x20, 0x14, 0x20, 0x0D, 0x27, 0x0D, 0x27, 0x5C, 0x2D, 0x5D, 0x2D, 0xE5, 0x32, 0xE4, 0x32,
		0x8E, 0x37, 0x8E, 0x37, 0x44, 0x3B, 0x44, 0x3B, 0xF7, 0x3D, 0xF7, 0x3D, 0x99, 0x3F, 0x9A, 0x3F,
		0x26, 0x40, 0x26, 0x40, 0x9A, 0x3F, 0x9A, 0x3F, 0xF8, 0x3D, 0xF6, 0x3D, 0x44, 0x3B, 0x45, 0x3B,
		0x8F, 0x37, 0x8D, 0x37, 0xE5, 0x32, 0xE5, 0x32, 0x5C, 0x2D, 0x5C, 0x2D, 0x0D, 0x27, 0x0D, 0x27,
		0x13, 0x20, 0x14, 0x20, 0x8D, 0x18, 0x8D, 0x18, 0x9B, 0x10, 0x9A, 0x10, 0x5F, 0x08, 0x5F, 0x08,
		0x00, 0x00, 0xFF, 0xFF, 0xA0, 0xF7, 0xA0, 0xF7, 0x65, 0xEF, 0x65, 0xEF, 0x73, 0xE7, 0x74, 0xE7,
		0xEC, 0xDF, 0xED, 0xDF, 0xF3, 0xD8, 0xF2, 0xD8, 0xA3, 0xD2, 0xA4, 0xD2, 0x1C, 0xCD, 0x1C, 0xCD,
		0x72, 0xC8, 0x73, 0xC8, 0xBC, 0xC4, 0xBC, 0xC4, 0x09, 0xC2, 0x09, 0xC2, 0x66, 0xC0, 0x66, 0xC0,
		0xDA, 0xBF, 0xDA, 0xBF, 0x66, 0xC0, 0x65, 0xC0, 0x09, 0xC2, 0x0A, 0xC2, 0xBC, 0xC4, 0xBC, 0xC4,
		0x72, 0xC8, 0x72, 0xC8, 0x1B, 0xCD, 0x1B, 0xCD, 0xA3, 0xD2, 0xA4, 0xD2, 0xF2, 0xD8, 0xF2, 0xD8,
		0xED, 0xDF, 0xEC, 0xDF, 0x74, 0xE7, 0x74, 0xE7, 0x65, 0xEF, 0x66, 0xEF, 0xA0, 0xF7, 0xA0, 0xF7,
		0x00, 0x00, 0x00, 0x00, 0x60, 0x08, 0x60, 0x08, 0x9B, 0x10, 0x9B, 0x10, 0x8D, 0x18, 0x8C, 0x18,
		0x13, 0x20, 0x13, 0x20, 0x0D, 0x27, 0x0D, 0x27, 0x5C, 0x2D, 0x5C, 0x2D, 0xE4, 0x32, 0xE4, 0x32,
		0x8E, 0x37, 0x8E, 0x37, 0x44, 0x3B, 0x45, 0x3B, 0xF7, 0x3D, 0xF7, 0x3D, 0x9A, 0x3F, 0x99, 0x3F,
		0x27, 0x40, 0x27, 0x40, 0x9A, 0x3F, 0x9A, 0x3F, 0xF7, 0x3D, 0xF6, 0x3D, 0x44, 0x3B, 0x44, 0x3B,
		0x8E, 0x37, 0x8F, 0x37, 0xE5, 0x32, 0xE5, 0x32, 0x5D, 0x2D, 0x5C, 0x2D, 0x0E, 0x27, 0x0E, 0x27,
		0x14, 0x20, 0x13, 0x20, 0x8D, 0x18, 0x8D, 0x18, 0x9B, 0x10, 0x9A, 0x10, 0x60, 0x08, 0x60, 0x08,
		0xFF, 0xFF, 0xFF, 0xFF, 0xA1, 0xF7, 0xA1, 0xF7, 0x65, 0xEF, 0x65, 0xEF, 0x73, 0xE7, 0x73, 0xE7,
		0xEC, 0xDF, 0xED, 0xDF, 0xF3, 0xD8, 0xF2, 0xD8, 0xA4, 0xD2, 0xA3, 0xD2, 0x1B, 0xCD, 0x1C, 0xCD,
		0x72, 0xC8, 0x71, 0xC8, 0xBB, 0xC4, 0xBB, 0xC4, 0x09, 0xC2, 0x09, 0xC2, 0x66, 0xC0, 0x66, 0xC0,
		0xDA, 0xBF, 0xDA, 0xBF, 0x66, 0xC0, 0x65, 0xC0, 0x09, 0xC2, 0x09, 0xC2, 0xBC, 0xC4, 0xBC, 0xC4,
		0x72, 0xC8, 0x71, 0xC8, 0x1B, 0xCD, 0x1C, 0xCD, 0xA3, 0xD2, 0xA5, 0xD2, 0xF3, 0xD8, 0xF2, 0xD8,
		0xEC, 0xDF, 0xED, 0xDF, 0x74, 0xE7, 0x74, 0xE7, 0x66, 0xEF, 0x65, 0xEF, 0xA0, 0xF7, 0xA1, 0xF7,
		0x00, 0x00, 0x00, 0x00, 0x60, 0x08, 0x5F, 0x08, 0x9A, 0x10, 0x9A, 0x10, 0x8C, 0x18, 0x8D, 0x18,
		0x14, 0x20, 0x13, 0x20, 0x0E, 0x27, 0x0D, 0x27, 0x5C, 0x2D, 0x5C, 0x2D, 0xE5, 0x32, 0xE5, 0x32,
		0x8E, 0x37, 0x8F, 0x37, 0x44, 0x3B, 0x44, 0x3B, 0xF7, 0x3D, 0xF7, 0x3D, 0x9A, 0x3F, 0x9B, 0x3F,
		0x26, 0x40, 0x26, 0x40, 0x9A, 0x3F, 0x99, 0x3F, 0xF7, 0x3D, 0xF7, 0x3D, 0x45, 0x3B, 0x44, 0x3B,
		0x8E, 0x37, 0x8E, 0x37, 0xE4, 0x32, 0xE5, 0x32, 0x5D, 0x2D, 0x5D, 0x2D, 0x0D, 0x27, 0x0E, 0x27,
		0x13, 0x20, 0x14, 0x20, 0x8C, 0x18, 0x8D, 0x18, 0x9A, 0x10, 0x9B, 0x10, 0x60, 0x08, 0x5F, 0x08,
		0x00, 0x00, 0x00, 0x00, 0xA1, 0xF7, 0xA0, 0xF7, 0x66, 0xEF, 0x65, 0xEF, 0x73, 0xE7, 0x73, 0xE7,
		0xED, 0xDF, 0xEC, 0xDF, 0xF3, 0xD8, 0xF3, 0xD8, 0xA4, 0xD2, 0xA3, 0xD2, 0x1C, 0xCD, 0x1B, 0xCD,
		0x72, 0xC8, 0x72, 0xC8, 0xBC, 0xC4, 0xBC, 0xC4, 0x0A, 0xC2, 0x09, 0xC2, 0x66, 0xC0, 0x66, 0xC0,
		0xDA, 0xBF, 0xDA, 0xBF, 0x65, 0xC0, 0x66, 0xC0, 0x09, 0xC2, 0x09, 0xC2, 0xBC, 0xC4, 0xBC, 0xC4,
		0x72, 0xC8, 0x71, 0xC8, 0x1B, 0xCD, 0x1C, 0xCD, 0xA4, 0xD2, 0xA4, 0xD2, 0xF3, 0xD8, 0xF2, 0xD8,
		0xED, 0xDF, 0xED, 0xDF, 0x74, 0xE7, 0x73, 0xE7, 0x65, 0xEF, 0x66, 0xEF, 0xA1, 0xF7, 0xA1, 0xF7,
		0x00, 0x00, 0x00, 0x00, 0x60, 0x08, 0x5F, 0x08, 0x9B, 0x10, 0x9A, 0x10, 0x8D, 0x18, 0x8D, 0x18,
		0x14, 0x20, 0x13, 0x20, 0x0D, 0x27, 0x0E, 0x27, 0x5C, 0x2D, 0x5D, 0x2D, 0xE5, 0x32, 0xE5, 0x32,
		0x8E, 0x37, 0x8F, 0x37, 0x44, 0x3B, 0x45, 0x3B, 0xF7, 0x3D, 0xF7, 0x3D, 0x9A, 0x3F, 0x9A, 0x3F,
		0x26, 0x40, 0x26, 0x40, 0x99, 0x3F, 0x99, 0x3F, 0xF6, 0x3D, 0xF6, 0x3D, 0x44, 0x3B, 0x44, 0x3B,
		0x8E, 0x37, 0x8F, 0x37, 0xE5, 0x32, 0xE4, 0x32, 0x5D, 0x2D, 0x5D, 0x2D, 0x0D, 0x27, 0x0D, 0x27,
		0x14, 0x20, 0x13, 0x20, 0x8C, 0x18, 0x8C, 0x18, 0x9A, 0x10, 0x9A, 0x10, 0x5F, 0x08, 0x60, 0x08,
		0xFF, 0xFF, 0x00, 0x00, 0xA0, 0xF7, 0xA1, 0xF7, 0x65, 0xEF, 0x66, 0xEF, 0x74, 0xE7, 0x73, 0xE7,
		0xED, 0xDF, 0xEC, 0xDF, 0xF3, 0xD8, 0xF3, 0xD8, 0xA4, 0xD2, 0xA3, 0xD2, 0x1A, 0xCD, 0x1B, 0xCD,
		0x72, 0xC8, 0x71, 0xC8, 0xBC, 0xC4, 0xBB, 0xC4, 0x09, 0xC2, 0x09, 0xC2, 0x66, 0xC0, 0x67, 0xC0,
		0xD9, 0xBF, 0xD9, 0xBF, 0x66, 0xC0, 0x65, 0xC0, 0x0A, 0xC2, 0x09, 0xC2, 0xBC, 0xC4, 0xBB, 0xC4,
		0x72, 0xC8, 0x72, 0xC8, 0x1B, 0xCD, 0x1B, 0xCD, 0xA4, 0xD2, 0xA3, 0xD2, 0xF2, 0xD8, 0xF3, 0xD8,
		0xED, 0xDF, 0xEC, 0xDF, 0x73, 0xE7, 0x74, 0xE7, 0x66, 0xEF, 0x65, 0xEF, 0xA0, 0xF7, 0xA1, 0xF7
};

static void I2SMclkConfig(uint32_t SampleRate)
{
    if((SampleRate == 11025) || (SampleRate == 22050) || (SampleRate == 44100)
        || (SampleRate == 88200) || (SampleRate == 176400))
    {
        Clock_AudioPllClockSet(APLL_CLK_MODE, PLL_CLK_1, 11289600);
        Clock_AudioMclkSel(AUDIO_I2S0, PLL_CLOCK1);
    }
    else
    {
        Clock_AudioPllClockSet(APLL_CLK_MODE, PLL_CLK_2, 12288000);
        Clock_AudioMclkSel(AUDIO_I2S0, PLL_CLOCK2);
    }
}

//iis采样率微调，iss fifo总量是10k，当余量<2k时，降低audio时钟，当余量>8k时，提高audio时钟，
//保证iis fifo余量在2k~8k的缓冲区间内
#ifdef SAMPLERATE_ADJUST_EN
void I2SSampleRateAdjust(void)
{
	int32_t FreeLen = DMA_CircularSpaceLenGet(PERIPHERAL_ID_I2S0_TX);
	//连续20次检测到fifo余>8k,则提高audio pll
	if(FreeLen>1024*8)
	{
		FreeLenWarn++;
		if(FreeLenWarn>20)
		{
			AdjustVal = (FreeLen*10/1024 - 80) ;
			PllUpDownFlag = 1;
			FreeLenWarn = 0;
		}

	}
	//连续20次检测到fifo余量<2k,则降低audio pll
	else if(FreeLen<1024*2)
	{
		FreeLenWarn++;
		if(FreeLenWarn>20)
		{
			AdjustVal = (20-FreeLen*10/1024);
			PllUpDownFlag = 0;
			FreeLenWarn = 0;
		}
	}
	else
	{
		FreeLenWarn = 0;
	}
	if((AdjustValBak != AdjustVal)&&(PllUpDownFlag == 1))
	{
		Clock_AudioPllClockAdjust(PLL_CLK_1, 0, AdjustVal);
		Clock_AudioPllClockAdjust(PLL_CLK_2, 0, AdjustVal);
		//printf("FreeLen %d，AdjustVal %d\n",FreeLen,AdjustVal);
		AdjustValBak = AdjustVal;
	}
	else if((AdjustValBak != AdjustVal)&&(PllUpDownFlag == 0))
	{
		Clock_AudioPllClockAdjust(PLL_CLK_1, 1, AdjustVal);
		Clock_AudioPllClockAdjust(PLL_CLK_2, 1, AdjustVal);
		//printf("FreeLen %d,AdjustVal -%d\n",FreeLen,-AdjustVal);
		AdjustValBak = AdjustVal;
	}
}
#endif

void SpdifRxExample(void)
{
	bool SpdifLockFlag = FALSE;
	uint32_t samplerate = 48000;
	uint32_t Len_spdif;
	uint32_t Len_i2s;

	DBG("SpdifRxExample!\n");

    //将SPDIF时钟切换到AUPLL
    Clock_APllLock(225792);//11.025,22.05,44.1,88.2,176.4 KHz
    SPDIF_ClockSourceSelect(SPIDF_CLK_SOURCE_AUPLL);
    Clock_AudioPllClockSet(APLL_CLK_MODE, PLL_CLK_2, 12288000);

	//mclk config
	I2SMclkConfig(samplerate);

	//i2s config
	I2S_ModuleDisable(0);
    GPIO_PortAModeSet(GPIOA0, 9);       //i2s0 mclk out
    GPIO_PortAModeSet(GPIOA1, 4);       //i2s0 lrclk out
    GPIO_PortAModeSet(GPIOA2, 3);       //i2s0 bclk out
    GPIO_PortAModeSet(GPIOA3, 7);       //i2s0 do

	DMA_CircularConfig(PERIPHERAL_ID_I2S0_TX, I2S0_FIFO_LEN/2, I2sAddr, I2S0_FIFO_LEN);

	I2S_MasterModeSet(I2S0_MODULE, I2S_FORMAT_I2S, I2S_LENGTH_16BITS);
	I2S_SampleRateSet(I2S0_MODULE, samplerate);
	I2S_ModuleEnable(I2S0_MODULE);


	//spdif config
	//同轴
    GPIO_PortAModeSet(GPIOA28, 10);
    SPDIF_AnalogModuleEnable(SPDIF_ANA_INPUT_A28, SPDIF_ANA_LEVEL_300mVpp);

	SPDIF_RXInit(1, 0, 0);
	DMA_CircularConfig(PERIPHERAL_ID_SPDIF_RX, SPDIF_FIFO_LEN/2, SpdifAddr, SPDIF_FIFO_LEN);
	SPDIF_ModuleEnable();

	DMA_ChannelEnable(PERIPHERAL_ID_SPDIF_RX);
	DMA_ChannelEnable(PERIPHERAL_ID_I2S0_TX);

	while(1)
	{
		if(SpdifLockFlag && !SPDIF_FlagStatusGet(LOCK_FLAG_STATUS))
		{
			DBG("SPDIF RX UNLOCK!\n");
			SpdifLockFlag = FALSE;
		}
		if(!SpdifLockFlag && SPDIF_FlagStatusGet(LOCK_FLAG_STATUS))
		{
			DBG("SPDIF RX LOCK!\n");
			SpdifLockFlag = TRUE;
		}

		//监控SPDIF RX采样率是否改变
		if(SpdifLockFlag == TRUE)
		{
			if(samplerate != SPDIF_SampleRateGet())
			{
				samplerate = SPDIF_SampleRateGet();

				if((samplerate == 11025) || (samplerate == 22050) || (samplerate == 44100) || (samplerate == 88200) || (samplerate == 176400))
				{
					Clock_APllLock(225792);
					Clock_AudioMclkSel(AUDIO_I2S0, PLL_CLOCK1);
					Clock_AudioPllClockSet(APLL_CLK_MODE, PLL_CLK_1, 11289600);
				}
				else
				{
					Clock_APllLock(221184);
					Clock_AudioMclkSel(AUDIO_I2S0, PLL_CLOCK2);
					Clock_AudioPllClockSet(APLL_CLK_MODE, PLL_CLK_2, 12288000);
				}
				I2S_SampleRateSet(I2S0_MODULE, samplerate);
				DBG("Current sample rate: %d\n", (int)samplerate);
			}
#ifdef SAMPLERATE_ADJUST_EN
			I2SSampleRateAdjust();
#endif
		}

		if(DMA_CircularDataLenGet(PERIPHERAL_ID_SPDIF_RX) >= 256*4)
		{

			Len_spdif = DMA_CircularDataGet(PERIPHERAL_ID_SPDIF_RX, SpdifBuf, 1024);
			Len_i2s   = SPDIF_SPDIFDataToPCMData((int32_t *)SpdifBuf, Len_spdif, (int32_t *)PcmBuf, SPDIF_WORDLTH_16BIT);

			DMA_CircularDataPut(PERIPHERAL_ID_I2S0_TX, PcmBuf, Len_i2s);
		}
	}
}

void SpdifTxExample(void)
{
	DBG("SpdifTxExample!\n");

	//spdif config
	GPIO_PortAModeSet(GPIOA28, 7);

	SPDIF_TXInit(1, 1, 0, 10);
	DMA_CircularConfig(PERIPHERAL_ID_SPDIF_TX, SPDIF_FIFO_LEN/2, SpdifAddr, SPDIF_FIFO_LEN);
	SPDIF_SampleRateSet(48000);
	SPDIF_ModuleEnable();
	DMA_ChannelEnable(PERIPHERAL_ID_SPDIF_TX);

	while(1)
	{
		if(((DMA_CircularSpaceLenGet(PERIPHERAL_ID_SPDIF_TX) / 8) * 8) >= 768 * 8)
		{
			int m;

			m = SPDIF_PCMDataToSPDIFData((int32_t *)SineBuf, 768, (int32_t *)SpdifBuf, SPDIF_WORDLTH_16BIT);
			DMA_CircularDataPut(PERIPHERAL_ID_SPDIF_TX, SpdifBuf, m & 0xFFFFFFFC);
		}
	}
}

void AudioLineInInit(uint32_t SampleRate)
{
    if((SampleRate == 11025) || (SampleRate == 22050) || (SampleRate == 44100))
    {
        Clock_AudioPllClockSet(APLL_CLK_MODE, PLL_CLK_1, 11289600);
        Clock_AudioMclkSel(AUDIO_ADC0, PLL_CLOCK1);
    }
    else
    {
        Clock_AudioPllClockSet(APLL_CLK_MODE, PLL_CLK_2, 12288000);
        Clock_AudioMclkSel(AUDIO_ADC0, PLL_CLOCK2);
    }

    AudioADC_AnaInit();

    AudioADC_DynamicElementMatch(ADC0_MODULE, TRUE, TRUE);

    AudioADC_PGASel(ADC0_MODULE, CHANNEL_RIGHT, LINEIN1_RIGHT);
    AudioADC_PGASel(ADC0_MODULE, CHANNEL_LEFT, LINEIN1_LEFT);

    AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, LINEIN1_RIGHT, 12, 0);
    AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT, LINEIN1_LEFT, 12, 0);

    DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC0_RX);
    DMA_CircularConfig(PERIPHERAL_ID_AUDIO_ADC0_RX, ASDM0_FIFO_LEN/2, (void*)ASDM0_FIFO_START_ADDR, ASDM0_FIFO_LEN);

    AudioADC_SampleRateSet(ADC0_MODULE, SampleRate);

    AudioADC_LREnable(ADC0_MODULE, 1, 1);
    AudioADC_FadeTimeSet(ADC0_MODULE, 10);
    AudioADC_FadeDisable(ADC0_MODULE);
    AudioADC_VolSet(ADC0_MODULE, vol_l, vol_r);
    AudioADC_SoftMute(ADC0_MODULE, FALSE, FALSE);
    AudioADC_Clear(ADC0_MODULE);

    DMA_ChannelEnable(PERIPHERAL_ID_AUDIO_ADC0_RX);
    DBG("ASDM0 DMA addr: %X\n", ASDM0_FIFO_START_ADDR);

    AudioADC_Enable(ADC0_MODULE);
}

//SPDIF发送测试，发送ADC采集到的数据，同时DAC也播放ADC采集到的数据
//ADC模块信号输入，SPDIF信号输出，ADC和SPDIF共用AUPLL，测试是否有杂音或断音
void ADCInSpdifTxExample(uint32_t SampleRate)
{
	//int32_t RealLen = 0;

    DBG("ADCInSpdifTxExample in!\n");
    //SPDIF_ModuleRst();
    GPIO_PortAModeSet(GPIOA28, 7);

    DMA_CircularConfig(PERIPHERAL_ID_SPDIF_TX, SPDIF_FIFO_LEN/2, SpdifAddr, SPDIF_FIFO_LEN);
    //Sync err
    /*NVIC_EnableIRQ(Spdif_IRQn);
    SpdifIntType = EMPTY_ERROR_INT;
    SPDIF_SetInt(SpdifIntType, 1);

    SpdifIntType1 = FULL_ERROR_INT;
    SPDIF_SetInt(SpdifIntType1, 1);

    SpdifIntType2 = UNDER_ERROR_INT;
    SPDIF_SetInt(SpdifIntType2, 1);*/
    //DMA_ChannelEnable(PERIPHERAL_ID_SPDIF_TX);
    //将SPDIF时钟切换到AUPLL
    if((SampleRate == 11025) || (SampleRate == 22050) || (SampleRate == 44100)
            || (SampleRate == 88200) || (SampleRate == 176400))
    {
        Clock_APllLock(225792);
        SPDIF_ClockSourceSelect(SPIDF_CLK_SOURCE_AUPLL);
    }
    else
    {
        Clock_APllLock(221184);
        SPDIF_ClockSourceSelect(SPIDF_CLK_SOURCE_AUPLL);
    }


    SPDIF_TXInit(1, 1, 0, 10);
    SPDIF_SampleRateSet(SampleRate);
    DMA_ChannelEnable(PERIPHERAL_ID_SPDIF_TX);
    SPDIF_ModuleEnable();

    Clock_AudioPllClockSet(APLL_CLK_MODE, PLL_CLK_1, 11289600);
    Clock_AudioPllClockSet(APLL_CLK_MODE, PLL_CLK_2, 12288000);

    AudioLineInInit(SampleRate);

    while(1)
    {
        if(DMA_CircularDataLenGet(PERIPHERAL_ID_AUDIO_ADC0_RX) >= 256*4)
        {
            DMA_CircularDataGet(PERIPHERAL_ID_AUDIO_ADC0_RX, PcmBuf1, 256*4);

            DMA_CircularDataPut(PERIPHERAL_ID_AUDIO_DAC0_TX, PcmBuf1, 256*4);

            if(((DMA_CircularSpaceLenGet(PERIPHERAL_ID_SPDIF_TX) / 8) * 8) >= 256 * 4)
            {
                int m;
                m = SPDIF_PCMDataToSPDIFData((int32_t *)PcmBuf1, 256 * 4, (int32_t *)SpdifBuf, SPDIF_WORDLTH_16BIT);
                DMA_CircularDataPut(PERIPHERAL_ID_SPDIF_TX, (void *)SpdifBuf, m & 0xFFFFFFFC);
            }
        }
    }
}


int main(void)
{
	uint8_t key;
	uint8_t	UartPort = 1;

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

	DBG("\nBuilt time: %s %s\n",__TIME__,__DATE__);

	GIE_ENABLE();

	DBG("\n");
	DBG("/-----------------------------------------------------\\\n");
	DBG("|                     SPDIF Example                     |\n");
	DBG("|      Mountain View Silicon Technology Co.,Ltd.      |\n");
	DBG("\\-----------------------------------------------------/\n");
	DBG("\n");
	DBG("please choose the example:\n");
	DBG("R: SpdifRxExample\n");
	DBG("T: SpdifTxExample\n");
	DBG("A: ADCInSpdifTxExample\n");
	DMA_ChannelAllocTableSet(DmaChannelMap);
	while(1)
	{
		key = 0;
		if(UARTS_Recv(UartPort, &key, 1,100) > 0)
		{
			switch(key)
			{
				case 'R':
					SpdifRxExample();
					break;
				case 'T':
					SpdifTxExample();
					break;
				case 'A':
					ADCInSpdifTxExample(44100);
				default:
					break;
			}
		}
	}
}


