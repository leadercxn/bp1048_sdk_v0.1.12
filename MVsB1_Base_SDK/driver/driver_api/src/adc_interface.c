#include <string.h>
#include "type.h"
#include "audio_adc.h"
#include "dma.h"
#include "clk.h"
#ifdef CFG_APP_CONFIG
#include "app_config.h"
#include "ctrlvars.h"
#endif
#include "reset.h"

#define USER_DEFINE_MCLK_112896M_DIV_VALUE	11289600
#define USER_DEFINE_MCLK_12288M_DIV_VALUE	12288000

void AudioADC_DMARestart(ADC_MODULE Module, void* Buf, uint16_t Len)
{
	if(Module == ADC0_MODULE)
	{
		DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC0_RX);
		memset(Buf, 0, Len);
		AudioADC_Clear(ADC0_MODULE);
		DMA_CircularFIFOClear(PERIPHERAL_ID_AUDIO_ADC0_RX);
		DMA_ChannelEnable(PERIPHERAL_ID_AUDIO_ADC0_RX);
	}
	else if(Module == ADC1_MODULE)
	{
		DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC1_RX);
		memset(Buf, 0, Len);
		AudioADC_Clear(ADC1_MODULE);
		DMA_CircularFIFOClear(PERIPHERAL_ID_AUDIO_ADC1_RX);
		DMA_ChannelEnable(PERIPHERAL_ID_AUDIO_ADC1_RX);
	}
}

void AudioADC_DigitalInit(ADC_MODULE Module, uint32_t SampleRate, void* Buf, uint16_t Len)
{
	//音频时钟使能，其他模块可能也会开启

	Clock_AudioPllClockSet(APLL_CLK_MODE, PLL_CLK_1, USER_DEFINE_MCLK_112896M_DIV_VALUE);
	Clock_AudioPllClockSet(APLL_CLK_MODE, PLL_CLK_2, USER_DEFINE_MCLK_12288M_DIV_VALUE);

	if(Module == ADC0_MODULE)
    {
    	AudioADC_Disable(ADC0_MODULE);
        //DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC0_RX);
		//Reset_FunctionReset(DMAC_FUNC_SEPA);
    	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC0_RX, DMA_DONE_INT);
    	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC0_RX, DMA_THRESHOLD_INT);
    	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC0_RX, DMA_ERROR_INT);
		DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC0_RX);
        DMA_CircularConfig(PERIPHERAL_ID_AUDIO_ADC0_RX, Len/2, Buf, Len);
        if(AudioADC_IsOverflow(ADC0_MODULE))
		{
        	AudioADC_OverflowClear(ADC0_MODULE);
		}
		AudioADC_Clear(ADC0_MODULE);
        
    	if((SampleRate == 11025) || (SampleRate == 22050) || (SampleRate == 44100))
    	{
    		Clock_AudioMclkSel(AUDIO_ADC0, PLL_CLOCK1);
    	}
    	else
    	{
    		Clock_AudioMclkSel(AUDIO_ADC0, PLL_CLOCK2);
    	}
    	AudioADC_SampleRateSet(ADC0_MODULE, SampleRate);
        
//    	AudioADC_HighPassFilterConfig(ADC0_MODULE, 0xFFE);
    	AudioADC_HighPassFilterSet(ADC0_MODULE, TRUE);
//    	AudioADC_HighPassFilterSet(ADC0_MODULE, 0);
        AudioADC_LREnable(ADC0_MODULE, 1, 1);
        AudioADC_FadeTimeSet(ADC0_MODULE, 10);
        AudioADC_FadeEnable(ADC0_MODULE);
		//AudioADC_FadeDisable(ADC0_MODULE);
        AudioADC_VolSet(ADC0_MODULE, 0xFFF, 0xFFF);
        AudioADC_SoftMute(ADC0_MODULE, FALSE, FALSE);
        AudioADC_Clear(ADC0_MODULE);
        AudioADC_Enable(ADC0_MODULE);
        
        DMA_ChannelEnable(PERIPHERAL_ID_AUDIO_ADC0_RX);
    }
    else if(Module == ADC1_MODULE)
    {
    	AudioADC_Disable(ADC1_MODULE);
    	//DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC1_RX);
		//Reset_FunctionReset(DMAC_FUNC_SEPA);
    	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC1_RX, DMA_DONE_INT);
    	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC1_RX, DMA_THRESHOLD_INT);
    	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC1_RX, DMA_ERROR_INT);
		DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC1_RX);
        DMA_CircularConfig(PERIPHERAL_ID_AUDIO_ADC1_RX, Len/2, Buf, Len);
    	if(AudioADC_IsOverflow(ADC1_MODULE))
    	{
    		AudioADC_OverflowClear(ADC1_MODULE);
    	}
    	AudioADC_Clear(ADC1_MODULE);
        
    	if((SampleRate == 11025) || (SampleRate == 22050) || (SampleRate == 44100))
    	{
    		Clock_AudioMclkSel(AUDIO_ADC1, PLL_CLOCK1);
    	}
    	else
    	{
    		Clock_AudioMclkSel(AUDIO_ADC1, PLL_CLOCK2);
    	}
    	AudioADC_SampleRateSet(ADC1_MODULE, SampleRate);

    	AudioADC_HighPassFilterConfig(ADC1_MODULE, 0xFFE);
        AudioADC_HighPassFilterSet(ADC1_MODULE, TRUE);
        AudioADC_LREnable(ADC1_MODULE, 1, 1);
        AudioADC_FadeTimeSet(ADC1_MODULE, 10);
        AudioADC_FadeEnable(ADC1_MODULE);
        //AudioADC_FadeDisable(ADC1_MODULE);
        AudioADC_VolSet(ADC1_MODULE, 0xFFF, 0xFFF);
        AudioADC_SoftMute(ADC1_MODULE, FALSE, FALSE);
        AudioADC_Clear(ADC1_MODULE);
        AudioADC_Enable(ADC1_MODULE);
        
        DMA_ChannelEnable(PERIPHERAL_ID_AUDIO_ADC1_RX);
    }
}

void AudioADC_DeInit(ADC_MODULE Module)
{
	if(Module == ADC0_MODULE)
    {
	    AudioADC_Disable(ADC0_MODULE);
        DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC0_RX);
        //DMA_ChannelClose(PERIPHERAL_ID_AUDIO_ADC0_RX);    
        AudioADC_AnaDeInit(ADC0_MODULE);
    }
    else if(Module == ADC1_MODULE)
    {
	    AudioADC_Disable(ADC1_MODULE);
    	DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC1_RX);
    	//DMA_ChannelClose(PERIPHERAL_ID_AUDIO_ADC1_RX);	
    	AudioADC_AnaDeInit(ADC1_MODULE);
    }
}


//返回Length uint：sample
uint16_t AudioADC_DataLenGet(ADC_MODULE Module)
{
	uint16_t NumSamples = 0;
	if(Module == ADC0_MODULE)
    {
		NumSamples = DMA_CircularDataLenGet(PERIPHERAL_ID_AUDIO_ADC0_RX);
    }
    else if(Module == ADC1_MODULE)
    {
    	NumSamples = DMA_CircularDataLenGet(PERIPHERAL_ID_AUDIO_ADC1_RX);
    }

	return NumSamples / 4;
}

//此处后续可能要做处理，区分单声道和立体声,数据格式转换在外边处理
//和DMA的配置也会有关系
//返回Length uint：sample
uint16_t AudioADC_DataGet(ADC_MODULE Module, void* Buf, uint16_t Len)
{
	uint16_t Length = 0;//Samples

    if(Module == ADC0_MODULE)
    {
    	Length = DMA_CircularDataLenGet(PERIPHERAL_ID_AUDIO_ADC0_RX);
    }
    else if(Module == ADC1_MODULE)
    {
    	Length = DMA_CircularDataLenGet(PERIPHERAL_ID_AUDIO_ADC1_RX);
    }
	
	if(Length > Len * 4)
	{
		Length = Len * 4;
	}

    if(Module == ADC0_MODULE)
	{
		DMA_CircularDataGet(PERIPHERAL_ID_AUDIO_ADC0_RX, Buf, Length & 0xFFFFFFFC);
	}
	else if(Module == ADC1_MODULE)
	{
		DMA_CircularDataGet(PERIPHERAL_ID_AUDIO_ADC1_RX, Buf, Length & 0xFFFFFFFC);
	}

    return Length / 4;
}

uint16_t AudioADC0DataLenGet(void)
{
	uint16_t NumSamples = 0;

	NumSamples = DMA_CircularDataLenGet(PERIPHERAL_ID_AUDIO_ADC0_RX);

	return NumSamples / 4;
}

uint16_t AudioADC1DataLenGet(void)
{
	uint16_t NumSamples = 0;

    NumSamples = DMA_CircularDataLenGet(PERIPHERAL_ID_AUDIO_ADC1_RX);

	return NumSamples / 4;
}

//返回Length uint：sample
uint16_t AudioADC0DataGet(void* Buf, uint16_t Len)
{
	uint16_t Length = 0;//Samples

    Length = DMA_CircularDataLenGet(PERIPHERAL_ID_AUDIO_ADC0_RX);


	if(Length > Len * 4)
	{
		Length = Len * 4;
	}

	DMA_CircularDataGet(PERIPHERAL_ID_AUDIO_ADC0_RX, Buf, Length & 0xFFFFFFFC);

    return Length / 4;
}

//此处后续可能要做处理，区分单声道和立体声,数据格式转换在外边处理
//和DMA的配置也会有关系
//返回Length uint：sample
uint16_t AudioADC1DataGet(void* Buf, uint16_t Len)
{
	uint16_t Length = 0;//Samples

    Length = DMA_CircularDataLenGet(PERIPHERAL_ID_AUDIO_ADC1_RX);

	if(Length > Len * 4)
	{
		Length = Len * 4;
	}
	DMA_CircularDataGet(PERIPHERAL_ID_AUDIO_ADC1_RX, Buf, Length & 0xFFFFFFFC);

    return Length / 4;
}
