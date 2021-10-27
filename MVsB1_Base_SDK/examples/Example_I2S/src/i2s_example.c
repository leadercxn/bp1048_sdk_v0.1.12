/**
 **************************************************************************************
 * @file    i2s_example.c
 * @brief   i2s example
 *
 * @author  Cecilia Wang
 * @version V1.0.0
 *
 * $Created: 2019-05-28 11:30:00$
 *
 * @copyright Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <stdlib.h>
#include <nds32_intrinsic.h>
#include "gpio.h"
#include "uarts.h"
#include "audio_adc.h"
#include "dac.h"
#include "uarts_interface.h"
#include "adc_interface.h"
#include "dac_interface.h"
#include "i2s_interface.h"
#include "type.h"
#include "debug.h"
#include "timeout.h"
#include "clk.h"
#include "dma.h"
#include "i2s.h"
#include "watchdog.h"
#include "remap.h"
#include "spi_flash.h"
#include "chip_info.h"
#include "dac_interface.h"

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


#define I2S0_RX_FIFO_LEN			(10 * 1024)
#define I2S0_TX_FIFO_LEN			(10 * 1024)
#define I2S1_TX_FIFO_LEN			(10 * 1024)
#define I2S1_RX_FIFO_LEN			(10 * 1024)
#define ADC_FIFO_LEN				(10 * 1024)
#define DAC_FIFO_LEN				(10 * 1024)

#define AUDIO_FRAME					(192)

int16_t PcmBuf[AUDIO_FRAME * 4];
int16_t AdcFifo[ADC_FIFO_LEN / 2];
int16_t I2s0RxFifo[I2S0_RX_FIFO_LEN / 2];
int16_t I2s0TxFifo[I2S0_RX_FIFO_LEN / 2];
int16_t I2s1TxFifo[I2S1_TX_FIFO_LEN / 2];
int16_t I2s1RxFifo[I2S1_RX_FIFO_LEN / 2];

int16_t DacFifo[DAC_FIFO_LEN / 2];

void I2S0_Master_RX_Example(void);
void I2S0_Slave_TX_Example(void);
void I2S0_I2S1_Binding_Example(void);
void I2S0_24Bit_Example(void);


int main(void)
{
	uint8_t key;
	uint8_t	UartPort = 1;

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
    DBG("|                     I2S Example                     |\n");
    DBG("|      Mountain View Silicon Technology Co.,Ltd.      |\n");
    DBG("\\-----------------------------------------------------/\n");
    DBG("r: I2S0_Master_RX_Example    \n");
    DBG("t: I2S1_Slave_TX_Example     \n");
    DBG("s: I2S0_I2S1_Binding_Example \n");
    DBG("h: I2S0_24bit_Example        \n");
    DBG("\n");

    while(1)
	{
		key = 0;
		if(UARTS_Recv(UartPort, &key, 1,100) > 0)
		{
			switch(key)
			{
				case 'r':
					DBG("r: I2S0_Master_RX_Example\n");
					I2S0_Master_RX_Example();
					break;
				case 't':
					DBG("t: I2S0_Slave_TX_Example\n");
					I2S0_Slave_TX_Example();
					break;
				case 's':
					DBG("s: I2S0_I2S1_Binding_Example\n");
					I2S0_I2S1_Binding_Example();
					break;
				case 'h':
					DBG("h: I2S0_24bit_Example\n");
					I2S0_24Bit_Example();
					break;
				default:
					break;
			}
		}
	}



}


//I2S0 Config: master and rx
void I2S0_Master_RX_Example(void)
{
	I2SParamCt i2s_ct;


	GPIO_PortAModeSet(GPIOA0, 9);// mclk out
	GPIO_PortAModeSet(GPIOA1, 6);// lrclk
	GPIO_PortAModeSet(GPIOA2, 5);// bclk
	GPIO_PortAModeSet(GPIOA4, 1);// din

	i2s_ct.IsMasterMode = 0;
	i2s_ct.SampleRate   = 48000;
	i2s_ct.I2sFormat    = I2S_FORMAT_I2S;
	i2s_ct.I2sBits      = I2S_LENGTH_16BITS;
	i2s_ct.RxPeripheralID = PERIPHERAL_ID_I2S0_RX;
	i2s_ct.RxBuf          = (void* )I2s0RxFifo;
	i2s_ct.RxLen		  = I2S0_RX_FIFO_LEN;
	i2s_ct.I2sTxRxEnable  = 2;

	AudioDAC_Init(DAC0, 48000, (void* )DacFifo, DAC_FIFO_LEN, NULL, 0);
	AudioI2S_Init(I2S0_MODULE, &i2s_ct);

	while(1)
	{
		if(AudioI2S_DataLenGet(I2S0_MODULE) >= AUDIO_FRAME)
		{
			AudioI2S_DataGet(I2S0_MODULE, PcmBuf, AUDIO_FRAME);
			AudioDAC_DataSet(DAC0, PcmBuf, AUDIO_FRAME);
		}
	}
}

//I2S0 Config: slave and tx
void I2S0_Slave_TX_Example(void)
{
	int   cur_sampleRate = 48000;
	int   sampleRate     = 48000;
	I2SParamCt i2s_ct;

	GPIO_PortAModeSet(GPIOA0, 9);// mclk out
	GPIO_PortAModeSet(GPIOA1, 6);// lrclk
	GPIO_PortAModeSet(GPIOA2, 5);// bclk
	GPIO_PortAModeSet(GPIOA3, 7);// dout

	i2s_ct.IsMasterMode = 1;
	i2s_ct.SampleRate   = 48000;
	i2s_ct.I2sFormat    = I2S_FORMAT_I2S;
	i2s_ct.I2sBits      = I2S_LENGTH_16BITS;
	i2s_ct.TxPeripheralID = PERIPHERAL_ID_I2S0_TX;
	i2s_ct.TxBuf          = (void* )I2s0TxFifo;
	i2s_ct.TxLen		  = I2S0_TX_FIFO_LEN;
	i2s_ct.I2sTxRxEnable  = 1;
	AudioI2S_Init(I2S0_MODULE, &i2s_ct);

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
	AudioADC_DigitalInit(ADC0_MODULE, 48000, (void*)AdcFifo, sizeof(AdcFifo));

	I2S_SampleRateCheckInterruptClr(I2S0_MODULE);
	I2S_SampleRateCheckInterruptEnable(I2S0_MODULE);

	while(1)
	{
		if(AudioADC_DataLenGet(ADC0_MODULE) >= AUDIO_FRAME)
		{
			AudioADC_DataGet(ADC0_MODULE, PcmBuf, AUDIO_FRAME);
			AudioI2S_DataSet(I2S0_MODULE, PcmBuf, AUDIO_FRAME);
		}

		if(I2S_SampleRateCheckInterruptGet(I2S0_MODULE))
		{
			cur_sampleRate = I2S_SampleRateGet(I2S0_MODULE);
			if(cur_sampleRate != sampleRate)
			{
				DBG("I2S Master sample rate change: %d\n", cur_sampleRate);
				sampleRate = cur_sampleRate;
				if((sampleRate == 11025) || (sampleRate == 22050) || (sampleRate == 44100))
				{
					Clock_AudioMclkSel(AUDIO_ADC0, PLL_CLOCK1);
				}
				else
				{
					Clock_AudioMclkSel(AUDIO_ADC0, PLL_CLOCK2);
				}
				AudioADC_SampleRateSet(ADC0_MODULE, sampleRate);
			}
			I2S_SampleRateCheckInterruptClr(I2S0_MODULE);
		}
	}
}

void I2S0_I2S1_Binding_Example(void)
{
	I2SParamCt i2s0_ct, i2s1_ct;

	//I2S0和I2S1的lrclk和bclk采样内部link，减少2个GPIO的复用
	GPIO_PortAModeSet(GPIOA0,  9); // i2s0_mclk_out_0(o)
	GPIO_PortAModeSet(GPIOA1,  4); // i2s0_lrck_0(o)/i2s1_lrck_2(i)
	GPIO_PortAModeSet(GPIOA2,  3); // i2s0_bclk_0(o)/i2s1_bclk_2(i)
	GPIO_PortAModeSet(GPIOA3,  7); // i2s0_dout_0(o)
	GPIO_PortAModeSet(GPIOA11, 2); // i2s1_din_1(i)

	//I2S0 Config: master and tx
	i2s0_ct.IsMasterMode   = 0;
	i2s0_ct.SampleRate     = 48000;
	i2s0_ct.I2sFormat      = I2S_FORMAT_I2S;
	i2s0_ct.I2sBits        = I2S_LENGTH_16BITS;
	i2s0_ct.TxPeripheralID = PERIPHERAL_ID_I2S0_TX;
	i2s0_ct.TxBuf          = (void* )I2s0TxFifo;
	i2s0_ct.TxLen		   = I2S0_TX_FIFO_LEN;
	i2s0_ct.I2sTxRxEnable  = 1;
	AudioI2S_Init(I2S0_MODULE, &i2s0_ct);

	//I2S1 Config: slave and rx
	i2s1_ct.IsMasterMode   = 1;
	i2s1_ct.SampleRate     = 48000;
	i2s1_ct.I2sFormat      = I2S_FORMAT_I2S;
	i2s1_ct.I2sBits        = I2S_LENGTH_16BITS;
	i2s1_ct.RxPeripheralID = PERIPHERAL_ID_I2S1_RX;
	i2s1_ct.RxBuf          = (void* )I2s1RxFifo;
	i2s1_ct.RxLen		   = I2S1_RX_FIFO_LEN;
	i2s1_ct.I2sTxRxEnable  = 2;
	AudioI2S_Init(I2S1_MODULE, &i2s1_ct);

	while(1)
	{
		if(AudioI2S_DataLenGet(I2S1_MODULE) > AUDIO_FRAME)
		{
			AudioI2S_DataGet(I2S1_MODULE, PcmBuf, AUDIO_FRAME);
			AudioI2S_DataSet(I2S0_MODULE, PcmBuf, AUDIO_FRAME);
		}
	}
}

void I2S0_24Bit_Example(void)
{
	I2SParamCt i2s_ct;

	GPIO_PortAModeSet(GPIOA0, 9);// mclk out
	GPIO_PortAModeSet(GPIOA1, 6);// lrclk
	GPIO_PortAModeSet(GPIOA2, 5);// bclk
	GPIO_PortAModeSet(GPIOA3, 7);// dout
	GPIO_PortAModeSet(GPIOA4, 1);// din

	i2s_ct.IsMasterMode = 0;
	i2s_ct.SampleRate   = 48000;
	i2s_ct.I2sFormat    = I2S_FORMAT_I2S;
	i2s_ct.I2sBits      = I2S_LENGTH_24BITS;

	//rx config
	i2s_ct.RxPeripheralID = PERIPHERAL_ID_I2S0_RX;
	i2s_ct.RxBuf          = (void* )I2s0RxFifo;
	i2s_ct.RxLen			= I2S0_RX_FIFO_LEN;

	//tx config
	i2s_ct.TxPeripheralID = PERIPHERAL_ID_I2S0_TX;
	i2s_ct.TxBuf          = (void* )I2s0TxFifo;
	i2s_ct.TxLen		  = I2S0_TX_FIFO_LEN;

	i2s_ct.I2sTxRxEnable = 3;
	AudioI2S_Init(I2S0_MODULE, &i2s_ct);

	while(1)
	{
		if(AudioI2S_DataLenGet(I2S0_MODULE) >= AUDIO_FRAME)
		{
			AudioI2S_DataGet(I2S0_MODULE, PcmBuf, AUDIO_FRAME);
			AudioI2S_DataSet(I2S0_MODULE, PcmBuf, AUDIO_FRAME);
		}
	}
}

