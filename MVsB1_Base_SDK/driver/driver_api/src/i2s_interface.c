#include <string.h>
#include "type.h"
#include "dma.h"
#include "clk.h"
#include "gpio.h"
#include "i2s_interface.h"
#include "reset.h"


#define USER_DEFINE_MCLK_112896M_DIV_VALUE  11289600
#define USER_DEFINE_MCLK_12288M_DIV_VALUE	12288000

void AudioI2S_Init(I2S_MODULE Module, I2SParamCt *ct)
{
	Clock_AudioPllClockSet(APLL_CLK_MODE, PLL_CLK_1, USER_DEFINE_MCLK_112896M_DIV_VALUE);
	Clock_AudioPllClockSet(APLL_CLK_MODE, PLL_CLK_2, USER_DEFINE_MCLK_12288M_DIV_VALUE);

	//tx
	if(ct->I2sTxRxEnable & 0x1)
	{
		DMA_ChannelDisable(ct->TxPeripheralID);
		DMA_CircularConfig(ct->TxPeripheralID, ct->TxLen/2, ct->TxBuf, ct->TxLen);
	}

	if(ct->I2sTxRxEnable & 0x2)
	{
		DMA_ChannelDisable(ct->RxPeripheralID);
		DMA_CircularConfig(ct->RxPeripheralID, ct->RxLen/2, ct->RxBuf, ct->RxLen);
	}

	if(Module == I2S0_MODULE)
    {
		if((ct->SampleRate == 11025) || (ct->SampleRate == 22050) || (ct->SampleRate == 44100) || (ct->SampleRate == 88200))
			Clock_AudioMclkSel(AUDIO_I2S0, PLL_CLOCK1);
		else
			Clock_AudioMclkSel(AUDIO_I2S0, PLL_CLOCK2);
    }
    else if(Module == I2S1_MODULE)
    {
    	if((ct->SampleRate == 11025) || (ct->SampleRate == 22050) || (ct->SampleRate == 44100) || (ct->SampleRate == 88200))
			Clock_AudioMclkSel(AUDIO_I2S1, PLL_CLOCK1);
		else
			Clock_AudioMclkSel(AUDIO_I2S1, PLL_CLOCK2);
    }

	I2S_SampleRateSet(Module, ct->SampleRate);
	if(ct->IsMasterMode == 0)
	{
		I2S_MasterModeSet(Module, ct->I2sFormat, ct->I2sBits);
	}
	else
	{
		I2S_SlaveModeSet(Module, ct->I2sFormat, ct->I2sBits);
	}
	
	if(ct->I2sTxRxEnable & 0x1)
	{
		DMA_ChannelEnable(ct->TxPeripheralID);
		I2S_ModuleTxEnable(Module);
	}
	if(ct->I2sTxRxEnable & 0x2)
	{
		DMA_ChannelEnable(ct->RxPeripheralID);
		I2S_ModuleRxEnable(Module);
	}
	I2S_ModuleEnable(Module);
}

void AudioI2S_DeInit(I2S_MODULE Module)
{
	if(Module == I2S0_MODULE)
    {
		Reset_RegisterReset(I2S0_REG_SEPA);
		Reset_FunctionReset(I2S0_FUNC_SEPA);
	    I2S_ModuleDisable(I2S0_MODULE);

		DMA_InterruptFlagClear(PERIPHERAL_ID_I2S0_RX, DMA_DONE_INT);
    	DMA_InterruptFlagClear(PERIPHERAL_ID_I2S0_RX, DMA_THRESHOLD_INT);
    	DMA_InterruptFlagClear(PERIPHERAL_ID_I2S0_RX, DMA_ERROR_INT);
		
		DMA_InterruptFlagClear(PERIPHERAL_ID_I2S0_TX, DMA_DONE_INT);
    	DMA_InterruptFlagClear(PERIPHERAL_ID_I2S0_TX, DMA_THRESHOLD_INT);
    	DMA_InterruptFlagClear(PERIPHERAL_ID_I2S0_TX, DMA_ERROR_INT);
		
		DMA_ChannelDisable(PERIPHERAL_ID_I2S0_RX);
        DMA_ChannelDisable(PERIPHERAL_ID_I2S0_TX);
    }
    else if(Module == I2S1_MODULE)
    {
    	Reset_RegisterReset(I2S1_REG_SEPA);
		Reset_FunctionReset(I2S1_FUNC_SEPA);
	    I2S_ModuleDisable(I2S1_MODULE);
		
		DMA_InterruptFlagClear(PERIPHERAL_ID_I2S1_RX, DMA_DONE_INT);
    	DMA_InterruptFlagClear(PERIPHERAL_ID_I2S1_RX, DMA_THRESHOLD_INT);
    	DMA_InterruptFlagClear(PERIPHERAL_ID_I2S1_RX, DMA_ERROR_INT);
		
		DMA_InterruptFlagClear(PERIPHERAL_ID_I2S1_TX, DMA_DONE_INT);
    	DMA_InterruptFlagClear(PERIPHERAL_ID_I2S1_TX, DMA_THRESHOLD_INT);
    	DMA_InterruptFlagClear(PERIPHERAL_ID_I2S1_TX, DMA_ERROR_INT);

		
		DMA_ChannelDisable(PERIPHERAL_ID_I2S1_RX);
        DMA_ChannelDisable(PERIPHERAL_ID_I2S1_TX);
		
    }
}

//·µ»ØLength uint£ºsample
//16bits:  NumSamples / 4;
//>16bits: NumSamples / 8;
uint16_t AudioI2S_DataLenGet(I2S_MODULE Module)
{
	uint16_t NumSamples = 0;
	uint16_t databits   = I2S_WordlengthGet(Module);

	if(Module == I2S0_MODULE)
    {
		NumSamples = DMA_CircularDataLenGet(PERIPHERAL_ID_I2S0_RX);
    }
    else if(Module == I2S1_MODULE)
    {
    	NumSamples = DMA_CircularDataLenGet(PERIPHERAL_ID_I2S1_RX);
    }

	return databits == I2S_LENGTH_16BITS? NumSamples / 4 : NumSamples / 8;
}

//·µ»ØLength uint£ºsample
uint16_t AudioI2S_DataGet(I2S_MODULE Module, void* Buf, uint16_t Len)
{
	uint16_t Length = 0;
	uint16_t databits   = I2S_WordlengthGet(Module);

    if(Module == I2S0_MODULE)
    {
    	Length = DMA_CircularDataLenGet(PERIPHERAL_ID_I2S0_RX);
    }
    else if(Module == I2S1_MODULE)
    {
    	Length = DMA_CircularDataLenGet(PERIPHERAL_ID_I2S1_RX);
    }

    if(databits > I2S_LENGTH_16BITS)
    	Len <<= 1;

	if(Length > Len * 4)
	{
		Length = Len * 4;
	}

    if(Module == I2S0_MODULE)
	{
		DMA_CircularDataGet(PERIPHERAL_ID_I2S0_RX, Buf, Length & (databits > I2S_LENGTH_16BITS ? 0xFFFFFFF8 : 0xFFFFFFFC));
	}
	else if(Module == I2S1_MODULE)
	{
		DMA_CircularDataGet(PERIPHERAL_ID_I2S1_RX, Buf, Length & (databits > I2S_LENGTH_16BITS ? 0xFFFFFFF8 : 0xFFFFFFFC));
	}

    return databits == I2S_LENGTH_16BITS? Length / 4 : Length / 8;
}


uint16_t AudioI2S_DataSpaceLenGet(I2S_MODULE Module)
{
	uint16_t databits   = I2S_WordlengthGet(Module);

	if(Module == I2S0_MODULE)
	{
		return databits == I2S_LENGTH_16BITS? DMA_CircularSpaceLenGet(PERIPHERAL_ID_I2S0_TX) / 4 : DMA_CircularSpaceLenGet(PERIPHERAL_ID_I2S0_TX) / 8;
	}
	else
	{
		return databits == I2S_LENGTH_16BITS? DMA_CircularSpaceLenGet(PERIPHERAL_ID_I2S1_TX) / 4 : DMA_CircularSpaceLenGet(PERIPHERAL_ID_I2S1_TX) / 8;
	}
}

void AudioI2S_DataSet(I2S_MODULE Module, void *Buf, uint32_t Len)
{
	uint16_t Length = 0;
	uint16_t databits   = I2S_WordlengthGet(Module);

	Length = databits == I2S_LENGTH_16BITS? Len * 4 : Len * 8;//Byte

	if(Module == I2S0_MODULE)
	{
		DMA_CircularDataPut(PERIPHERAL_ID_I2S0_TX, Buf, Length & 0xFFFFFFFC);
	}
	else
	{
		DMA_CircularDataPut(PERIPHERAL_ID_I2S1_TX, Buf, Length & 0xFFFFFFFC);
	}
}





uint16_t AudioI2S0_DataLenGet(void)
{
	return AudioI2S_DataLenGet(I2S0_MODULE);
}

uint16_t AudioI2S1_DataLenGet(void)
{
	return AudioI2S_DataLenGet(I2S1_MODULE);
}

uint16_t AudioI2S0_DataGet(void* Buf, uint16_t Len)
{
	return AudioI2S_DataGet(I2S0_MODULE, Buf, Len);
}

uint16_t AudioI2S1_DataGet(void* Buf, uint16_t Len)
{
	return AudioI2S_DataGet(I2S1_MODULE, Buf, Len);
}

uint16_t AudioI2S0_DataSpaceLenGet(void)
{
	return AudioI2S_DataSpaceLenGet(I2S0_MODULE);
}

uint16_t AudioI2S1_DataSpaceLenGet(void)
{
	return AudioI2S_DataSpaceLenGet(I2S1_MODULE);
}

uint16_t AudioI2S0_DataSet(void *Buf, uint16_t Len)
{
	AudioI2S_DataSet(I2S0_MODULE, Buf, Len); 
	return 0;
}

uint16_t AudioI2S1_DataSet(void *Buf, uint16_t Len)
{
	AudioI2S_DataSet(I2S1_MODULE, Buf, Len);
	return 0;
}
