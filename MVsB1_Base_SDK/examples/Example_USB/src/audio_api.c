#include <stdlib.h>
#include <nds32_intrinsic.h>
#include "type.h"
#include "debug.h"
#include "dma.h"
#include "dac.h"
#include "clk.h"
#include "audio_adc.h"
#include "adc_interface.h"
#include "dac_interface.h"
#include "usb_audio_api.h"

static const uint8_t DmaChannelMap[29] = {
	255,//PERIPHERAL_ID_SPIS_RX = 0,		//0
	255,//PERIPHERAL_ID_SPIS_TX,			//1
	255,//PERIPHERAL_ID_TIMER3,			//2
	4,//PERIPHERAL_ID_SDIO_RX,			//3
	5,//PERIPHERAL_ID_SDIO0_TX,			//4
	255,//PERIPHERAL_ID_UART0_RX,		//5
	255,//PERIPHERAL_ID_TIMER1,			//6
	255,//PERIPHERAL_ID_TIMER2,			//7
	255,//PERIPHERAL_ID_SDPIF_RX,			//8 SPDIF_RX /TX需要使用同一通道
	255,//PERIPHERAL_ID_SDPIF_TX,			//8
	255,//PERIPHERAL_ID_SPIM_RX,			//10
	255,//PERIPHERAL_ID_SPIM_TX,			//11
	255,//PERIPHERAL_ID_UART0_TX,		//12
	255,//PERIPHERAL_ID_UART1_RX,		//13
	255,//PERIPHERAL_ID_UART1_TX,		//14
	255,//PERIPHERAL_ID_TIMER4,			//15
	255,//PERIPHERAL_ID_TIMER5,			//16
	255,//PERIPHERAL_ID_TIMER6,			//17
	0,//PERIPHERAL_ID_AUDIO_ADC0_RX,	//18
	1,//PERIPHERAL_ID_AUDIO_ADC1_RX,		//19
	2,//PERIPHERAL_ID_AUDIO_DAC0_TX,		//20
	3,//PERIPHERAL_ID_AUDIO_DAC1_TX,		//21
	255,//PERIPHERAL_ID_I2S0_RX,			//22
	255,//PERIPHERAL_ID_I2S0_TX,			//23
	255,//PERIPHERAL_ID_I2S1_RX,			//24
	255,//PERIPHERAL_ID_I2S1_TX,			//25
	255,//PERIPHERAL_ID_PPWM,			//26
	255,//PERIPHERAL_ID_ADC,     			//27
	255,//PERIPHERAL_ID_SOFTWARE,		//28
};

#define	DAC0_FIFO_LEN					(256 * 4)
uint32_t DAC0FIFO[DAC0_FIFO_LEN/4];

#define	MIC_FIFO_LEN					(256 * 4)
uint32_t MICFIFO[MIC_FIFO_LEN/4];


#define ONE_MS_SAMPLE	48
uint32_t dac_play_buf[ONE_MS_SAMPLE];
uint32_t adc_play_buf[ONE_MS_SAMPLE];

extern uint32_t  usb_speaker_enable;
extern uint32_t  usb_mic_enable;

void audio_init(uint32_t SampleRate)
{

	DMA_ChannelAllocTableSet((uint8_t*)DmaChannelMap);

	AudioADC_AnaInit(/*ADC1_MODULE*/);
	AudioADC_VcomConfig(1);//MicBias en
	AudioADC_MicBias1Enable(1);

	AudioADC_DynamicElementMatch(ADC1_MODULE, TRUE, TRUE);
	AudioADC_PGASel(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1);
	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1, 0, 4);

	AudioADC_PGASel(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2);
	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2, 0, 4);

	AudioADC_DigitalInit(ADC1_MODULE, SampleRate, (void*)MICFIFO, MIC_FIFO_LEN);

	AudioADC_VcomConfig(1);//MicBias en
	AudioADC_MicBias1Enable(1);


	AudioDAC_DoutModeSet(AUDIO_DAC0, MODE0, WIDTH_16_BIT);
	AudioDAC_DoutModeSet(AUDIO_DAC1, MODE0, WIDTH_16_BIT);
	AudioDAC_Init(DAC0, SampleRate, (void*)DAC0FIFO, DAC0_FIFO_LEN, NULL, 0);
}


void audio_process(void)
{
	if(UsbAudioSpeakerDataLenGet() >= ONE_MS_SAMPLE)
	{
		UsbAudioSpeakerDataGet(dac_play_buf,ONE_MS_SAMPLE);
		if(usb_speaker_enable)
		{
			AudioDAC_DataSet(DAC0,dac_play_buf,ONE_MS_SAMPLE);
		}
	}
	if(AudioADC_DataLenGet(ADC1_MODULE) > ONE_MS_SAMPLE)
	{
		AudioADC_DataGet(ADC1_MODULE,adc_play_buf,ONE_MS_SAMPLE);
		if(usb_mic_enable)
		{
			UsbAudioMicDataSet(adc_play_buf,ONE_MS_SAMPLE);
		}
	}
}
