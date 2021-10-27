#include <string.h>
#include "type.h"
#include "spdif.h"
#include "dma.h"
#include "clk.h"
#include "debug.h"
//#include "rtos_api.h"

uint16_t AudioSpdifRx_GetDataLen(void)
 {
   uint16_t NumSamples = 0;
   NumSamples = DMA_CircularDataLenGet(PERIPHERAL_ID_SPDIF_RX);
   return NumSamples / 8;
 }
void *osPortMalloc(uint16_t osWantedSize);
void osPortFree( void *ospv );
uint16_t AudioSpdifRx_GetData(void* Buf, uint16_t Len)
{
	uint16_t Length = 0;//Samples
	uint8_t *SpdifBuf;

   	Length = DMA_CircularDataLenGet(PERIPHERAL_ID_SPDIF_RX) / 8;

	if(Length > Len)
	{
		Length = Len;
	}

	SpdifBuf = (uint8_t *)osPortMalloc(Length * 8);
	if(SpdifBuf == NULL)
	{
		DBG("Malloc failure!\n");
		return 0;
	}
	Length = DMA_CircularDataGet(PERIPHERAL_ID_SPDIF_RX, SpdifBuf, Length * 8);
	Length = SPDIF_SPDIFDataToPCMData((int32_t *)SpdifBuf, Length, (int32_t *)Buf, SPDIF_WORDLTH_16BIT);
	osPortFree(SpdifBuf);
    return Length / 4;
}
void AudioSpdif_DeInit(void)
{
	SPDIF_ModuleDisable();
    DMA_ChannelDisable(PERIPHERAL_ID_SPDIF_RX);
    //DMA_ChannelClose(PERIPHERAL_ID_SPDIF_RX);
}
