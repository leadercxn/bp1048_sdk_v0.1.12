#include "type.h"
#include "dac.h"
#include "dma.h"
#include "clk.h"
#include "debug.h"
#ifdef CFG_APP_CONFIG
#include "app_config.h"
#include "ctrlvars.h"
#endif

#define USER_DEFINE_MCLK_112896M_DIV_VALUE 	11289600
#define USER_DEFINE_MCLK_12288M_DIV_VALUE	12288000
//ChannelSel :0=disable 1=LR,2=X,3=ALL
void AudioDAC_Init(DAC_MODULE DACModule, uint32_t SampleRate, void* Buf1, uint16_t Len1, void* Buf2, uint16_t Len2)
{
	bool DAC0Enable = FALSE;
	bool DAC1Enable = FALSE;

	if(DACModule == DAC0)
	{
		DAC0Enable = TRUE;
	}
	else if(DACModule == DAC1)
	{
		DAC1Enable = TRUE;
	}
	else
	{
		DAC0Enable = TRUE;
		DAC1Enable = TRUE;
	}

	//CLK select
	Clock_AudioPllClockSet(APLL_CLK_MODE, PLL_CLK_1, USER_DEFINE_MCLK_112896M_DIV_VALUE);
	Clock_AudioPllClockSet(APLL_CLK_MODE, PLL_CLK_2, USER_DEFINE_MCLK_12288M_DIV_VALUE);

	//Clock_AudioMclkSel(AUDIO_DAC0, GPIO_IN0);//GPIO_IN0
	//Clock_AudioMclkSel(AUDIO_DAC0, OSC_IN);
	if(DAC0Enable)
	{
		if((SampleRate == 11025) || (SampleRate == 22050) || (SampleRate == 44100))
		{
			Clock_AudioMclkSel(AUDIO_DAC0, PLL_CLOCK1);
		}
		else
		{
			Clock_AudioMclkSel(AUDIO_DAC0, PLL_CLOCK2);
		}
	}
	if(DAC1Enable)
	{
		if((SampleRate == 11025) || (SampleRate == 22050) || (SampleRate == 44100))
		{
			Clock_AudioMclkSel(AUDIO_DAC1, PLL_CLOCK1);
		}
		else
		{
			Clock_AudioMclkSel(AUDIO_DAC1, PLL_CLOCK2);
		}
	}

	if(DAC0Enable)
	{
		//ADC0
		AudioDAC_SampleRateSet(DAC0, SampleRate);//48000, 44100
		AudioDAC_EdgeSet(DAC0, NEGEDGE);//POSEDGE,NEGEDGE
		AudioDAC_VolSet(DAC0, 0xFFF, 0xFFF);//默认使用3/4数字增益

		//更好的听感，系统默认关闭Scramble和Dither
		AudioDAC_ScrambleDisable(DAC0);
		AudioDAC_DitherDisable(DAC0);

		//AudioDAC_DoutModeSet(MODE0);//DAC0独有API
		AudioDAC_DoutModeSet(DAC0, MODE0, WIDTH_16_BIT);//WIDTH_24_BIT_1,WIDTH_24_BIT_2,WIDTH_16_BIT  bkd change maybe need 24bit too
		AudioDAC_DsmOutdisModeSet(DAC0, 1);
		AudioDAC_ZeroNumSet(DAC0, 0);

		AudioDAC_FuncReset(DAC0);
		AudioDAC_DigitalMute(DAC0, TRUE, TRUE);
		DelayMs(10);
		AudioDAC_FadeTimeSet(DAC0, 10);
		AudioDAC_FadeEnable(DAC0);
	}

	if(DAC1Enable)
	{
		//ADC1
		AudioDAC_SampleRateSet(DAC1, SampleRate);//48000, 44100
		AudioDAC_EdgeSet(DAC1, NEGEDGE);//
		AudioDAC_VolSet(DAC1, 0xFFF, 0xFFF);

		//更好的听感，系统默认关闭Scramble和Dither
		AudioDAC_ScrambleDisable(DAC1);
		AudioDAC_DitherDisable(DAC1);

		AudioDAC_DsmOutdisModeSet(DAC1, 1);
		AudioDAC_ZeroNumSet(DAC1, 0);
		AudioDAC_FuncReset(DAC1);
		AudioDAC_DigitalMute(DAC1, TRUE, TRUE);
		DelayMs(10);
		AudioDAC_FadeTimeSet(DAC1, 10);
		AudioDAC_FadeEnable(DAC1);
	}
	//analog
	AudioDAC_VIMDControl();

	if(DACModule == DAC0)
	{
		AudioDAC_ChannelEnable(DAC0);
		if(AudioDAC_PowerOn(DAC0))
		{
			DBG("DAC0 poweron OK!\n");
		}
	}
	else if(DACModule == DAC1)
	{
		AudioDAC_ChannelEnable(DAC1);
		if(AudioDAC_PowerOn(DAC1))
		{
			DBG("DAC1 poweron OK!\n");
		}
	}
	else
	{
		AudioDAC_AllChannelEnable();
		if(AudioDAC_AllPowerOn())
		{
			DBG("DAC All poweron OK!\n");
		}
	}

	if(DAC0Enable)
	{
		AudioDAC_SCFMute(DAC0, FALSE, FALSE);
	}

	if(DAC1Enable)
	{
		AudioDAC_SCFMute(DAC1, FALSE, FALSE);
	}

	if(DAC0Enable)
	{
		DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_DAC0_TX);
		DMA_CircularConfig(PERIPHERAL_ID_AUDIO_DAC0_TX, Len1/2, Buf1, Len1);
		DMA_ChannelEnable(PERIPHERAL_ID_AUDIO_DAC0_TX);
	}

	if(DAC1Enable)
	{
		DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_DAC1_TX);
		DMA_CircularConfig(PERIPHERAL_ID_AUDIO_DAC1_TX, Len2/2, Buf2, Len2);
		DMA_ChannelEnable(PERIPHERAL_ID_AUDIO_DAC1_TX);
	}

	if(DAC0Enable)
	{
		AudioDAC_ClkEnable(DAC0, TRUE);
	}
	if(DAC1Enable)
	{
		AudioDAC_ClkEnable(DAC1, TRUE);
	}

	if(DAC0Enable)
	{
		AudioDAC_Enable(DAC0);
		AudioDAC_Reset(DAC0);//??
		AudioDAC_DigitalMute(DAC0, FALSE, FALSE);
	}
	if(DAC1Enable)
	{
		AudioDAC_Enable(DAC1);
		AudioDAC_Reset(DAC1);//??
		AudioDAC_DigitalMute(DAC1, FALSE, FALSE);
	}

	if(DAC0Enable)
	{
		AudioDAC_IBSelect(DAC0, 3, 3);
	}
	if(DAC1Enable)
	{
		AudioDAC_IBSelect(DAC1, 3, 3);
	}
}


//ChannelSel :0=LR,1=X,2=ALL
void AudioDAC_SampleRateChange(DAC_MODULE DACModule, uint32_t SampleRate)
{
	bool DAC0Enable = FALSE;
	bool DAC1Enable = FALSE;

	if(DACModule == DAC0)
	{
		DAC0Enable = TRUE;
	}
	else if(DACModule == DAC1)
	{
		DAC1Enable = TRUE;
	}
	else
	{
		DAC0Enable = TRUE;
		DAC1Enable = TRUE;
	}

	if(DAC0Enable == TRUE)
	{
		if((SampleRate == 11025) || (SampleRate == 22050) || (SampleRate == 44100))
		{
			Clock_AudioMclkSel(DAC0, PLL_CLOCK1);
		}
		else
		{
			Clock_AudioMclkSel(DAC0, PLL_CLOCK2);
		}
		AudioDAC_SampleRateSet(DAC0, SampleRate);
	}

	if(DAC1Enable == TRUE)
	{
		if((SampleRate == 11025) || (SampleRate == 22050) || (SampleRate == 44100))
		{
			Clock_AudioMclkSel(DAC1, PLL_CLOCK1);
		}
		else
		{
			Clock_AudioMclkSel(DAC1, PLL_CLOCK2);
		}
		AudioDAC_SampleRateSet(DAC1, SampleRate);
	}
}


uint16_t AudioDAC_DataSpaceLenGet(DAC_MODULE DACModule)
{
	if(DACModule == DAC0)
	{
		return DMA_CircularSpaceLenGet(PERIPHERAL_ID_AUDIO_DAC0_TX) / 4;
	}
	else if(DACModule == DAC1)
	{
		return DMA_CircularSpaceLenGet(PERIPHERAL_ID_AUDIO_DAC1_TX) / 2;
	}
	return 0;
}

uint16_t AudioDAC_DataSet(DAC_MODULE DACModule, void* Buf, uint16_t Len)
{
	uint16_t Length;

	if(Buf ==NULL) return 0;

	if(DACModule == DAC0)
	{
		Length = Len * 4;
		DMA_CircularDataPut(PERIPHERAL_ID_AUDIO_DAC0_TX, Buf, Length & 0xFFFFFFFC);
	}
	else if(DACModule == DAC1)
	{
		Length = Len * 2;
		DMA_CircularDataPut(PERIPHERAL_ID_AUDIO_DAC1_TX, Buf, Length & 0xFFFFFFFE);
	}
	return 0;
}

uint16_t AudioDAC0DataSpaceLenGet(void)
{
	return DMA_CircularSpaceLenGet(PERIPHERAL_ID_AUDIO_DAC0_TX) / 4;
}

uint16_t AudioDAC1DataSpaceLenGet(void)
{
	return DMA_CircularSpaceLenGet(PERIPHERAL_ID_AUDIO_DAC1_TX) / 2;
}

uint16_t AudioDAC0DataSet(void* Buf, uint16_t Len)
{
	uint16_t Length;

	if(Buf == NULL) return 0;

	Length = Len * 4;

	DMA_CircularDataPut(PERIPHERAL_ID_AUDIO_DAC0_TX, Buf, Length & 0xFFFFFFFC);
	return 0;
}

uint16_t AudioDAC1DataSet(void* Buf, uint16_t Len)
{
	uint16_t Length;

	if(Buf == NULL) return 0;

	Length = Len * 2;
	DMA_CircularDataPut(PERIPHERAL_ID_AUDIO_DAC1_TX, Buf, Length & 0xFFFFFFFE);
	return 0;
}
