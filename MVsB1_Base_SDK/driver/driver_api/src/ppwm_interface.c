#include "type.h"
#include "dac.h"
#include "dma.h"
#include "clk.h"
#include "ppwm.h"
#include "debug.h"

#define USER_DEFINE_MCLK_112896M_DIV_VALUE 11289600
#define USER_DEFINE_MCLK_12288M_DIV_VALUE	12288000

void AudioPPWM_Init(uint32_t SampleRate, void* Buf, uint16_t Len)
{
	Clock_AudioPllClockSet(APLL_CLK_MODE, PLL_CLK_1, USER_DEFINE_MCLK_112896M_DIV_VALUE);
	Clock_AudioPllClockSet(APLL_CLK_MODE, PLL_CLK_2, USER_DEFINE_MCLK_12288M_DIV_VALUE);

	if((SampleRate == 11025) || (SampleRate == 22050) || (SampleRate == 44100))
	{
		Clock_AudioMclkSel(AUDIO_PPWM, PLL_CLOCK1);
	}
	else
	{
		Clock_AudioMclkSel(AUDIO_PPWM, PLL_CLOCK2);
	}

	PPWM_ClkEnable();
	PPWM_Disable();
	PPWM_Pause();
	PPWM_VolSet(0x1000);
	PPWM_FadeTimeSet(10);
	PPWM_Run();

	DMA_CircularConfig(PERIPHERAL_ID_PPWM, Len/2, Buf, Len);
    DMA_ChannelEnable(PERIPHERAL_ID_PPWM);
	PPWM_Enable();
}

void AudioPPWM_SampleRateChange(uint32_t SampleRate)
{

		if((SampleRate == 11025) || (SampleRate == 22050) || (SampleRate == 44100))
		{
			Clock_AudioMclkSel(AUDIO_PPWM, PLL_CLOCK1);
		}
		else
		{
			Clock_AudioMclkSel(AUDIO_PPWM, PLL_CLOCK2);
		}
}


uint16_t AudioPPWM_DataSpaceLenGet(void)
{
	return DMA_CircularSpaceLenGet(PERIPHERAL_ID_PPWM) / 2;
}

uint16_t AudioPPWM_DataSet(void* Buf, uint16_t Len)
{
	uint16_t Length;
	Length = Len * 2;
	return (uint16_t)DMA_CircularDataPut(PERIPHERAL_ID_PPWM, Buf, Length & 0xFFFFFFFE);
}

