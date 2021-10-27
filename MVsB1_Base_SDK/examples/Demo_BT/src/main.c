#include <stdlib.h>
#include <nds32_intrinsic.h>
#include "uarts.h"
#include "uarts_interface.h"
#include "backup.h"
#include "backup_interface.h"
#include "type.h"
#include "debug.h"
#include "timeout.h"
#include "clk.h"
#include "dma.h"
#include "timer.h"
#include "adc.h"
#include "i2s.h"
#include "watchdog.h"
#include "reset.h"
#include "rtc.h"
#include "spi_flash.h"
#include "gpio.h"
#include "chip_info.h"
#include "irqn.h"
#include "remap.h"
#include "dac.h" //for volscale
#include "otg_detect.h"
#include "sw_uart.h"

#include "sadc_interface.h"
#include "powercontroller.h"
#include "audio_decoder_api.h"


#define BT_SBC_PACKET_SIZE					595
#define BT_SBC_DECODER_INPUT_LEN			(8*1024)
#define BT_SBC_LEVEL_HIGH					(BT_SBC_DECODER_INPUT_LEN - BT_SBC_PACKET_SIZE * 4)
#define BT_SBC_LEVEL_LOW					(BT_SBC_PACKET_SIZE *6)//(BT_SBC_LEVEL_HIGH  - BT_SBC_PACKET_SIZE * 3)
#define BT_SBC_LEVEL_START					(BT_SBC_LEVEL_HIGH  - BT_SBC_PACKET_SIZE * 3)
#define SBC_DECODER_FIFO_MIN				(119*2)

uint8_t  a2dp_sbcBuf[BT_SBC_DECODER_INPUT_LEN];

static uint8_t dac0_dma_buffer[5120 * 4] = {0};
static uint8_t dac1_dma_buffer[5120 * 2] = {0};

static uint8_t decoder_buf[1024 * 40] = {0};

static int16_t vol_l = 0x200;
static int16_t vol_r = 0x200;

static uint8_t DecoderInitialized = 0;

MemHandle SBC_MemHandle;
static void A2dp_DecoderInit(void);
static void SaveDataToSbcBuffer(uint8_t * data, uint16_t dataLen);
static void AudioDAC0_ResetDMA(void);


static const uint8_t DmaChannelMap[29] = {
	255,//PERIPHERAL_ID_SPIS_RX = 0,	//0
	255,//PERIPHERAL_ID_SPIS_TX,		//1
	255,//PERIPHERAL_ID_TIMER3,			//2
	4,//PERIPHERAL_ID_SDIO_RX,			//3
	5,//PERIPHERAL_ID_SDIO_TX,			//4
	255,//PERIPHERAL_ID_UART0_RX,		//5
	255,//PERIPHERAL_ID_TIMER1,			//6
	255,//PERIPHERAL_ID_TIMER2,			//7
	255,//PERIPHERAL_ID_SDPIF_RX,		//8 SPDIF_RX /TX same chanell
	255,//PERIPHERAL_ID_SDPIF_TX,		//8 SPDIF_RX /TX same chanell
	255,//PERIPHERAL_ID_SPIM_RX,		//9
	255,//PERIPHERAL_ID_SPIM_TX,		//10
	255,//PERIPHERAL_ID_UART0_TX,		//11
	255,//PERIPHERAL_ID_UART1_RX,		//12
	255,//PERIPHERAL_ID_UART1_TX,		//13
	255,//PERIPHERAL_ID_TIMER4,			//14
	255,//PERIPHERAL_ID_TIMER5,			//15
	255,//PERIPHERAL_ID_TIMER6,			//16
	0,//PERIPHERAL_ID_AUDIO_ADC0_RX,	//17
	1,//PERIPHERAL_ID_AUDIO_ADC1_RX,	//18
	2,//PERIPHERAL_ID_AUDIO_DAC0_TX,	//19
	3,//PERIPHERAL_ID_AUDIO_DAC1_TX,	//20
	255,//PERIPHERAL_ID_I2S0_RX,		//21
	255,//PERIPHERAL_ID_I2S0_TX,		//22
	255,//PERIPHERAL_ID_I2S1_RX,		//23
	255,//PERIPHERAL_ID_I2S1_TX,		//24
	255,//PERIPHERAL_ID_PPWM,			//25
	255,//PERIPHERAL_ID_ADC,     		//26
	255,//PERIPHERAL_ID_SOFTWARE,		//27
};




void SystemClockInit(void)
{
	//clock配置
	Clock_Config(1, 24000000);
	Clock_PllLock(288000);
	Clock_APllLock(240000);
	Clock_USBClkDivSet(4);
	Clock_SysClkSelect(PLL_CLK_MODE);
	Clock_USBClkSelect(APLL_CLK_MODE);
	Clock_UARTClkSelect(APLL_CLK_MODE);
	Clock_Timer3ClkSelect(RC_CLK_MODE);//for cec rc clk

	Clock_Module1Enable(ALL_MODULE1_CLK_SWITCH);
	Clock_Module2Enable(ALL_MODULE2_CLK_SWITCH);
	Clock_Module3Enable(ALL_MODULE3_CLK_SWITCH);
}

int main(void)
{


	Chip_Init(1);

	WDG_Disable();

	SystemClockInit();

	GPIO_PortAModeSet(GPIOA24, 1);//Rx, A24:uart1_rxd_0
	GPIO_PortAModeSet(GPIOA25, 6);//Tx, A25:uart1_txd_0
	DbgUartInit(1, 256000, 8, 0, 1);



	Remap_InitTcm(FLASH_ADDR, TCM_SIZE);


	SpiFlashInit(80000000, MODE_4BIT, 0, 1);
	Clock_RcFreqGet(TRUE);//不可屏蔽


	NVIC_EnableIRQ(SWI_IRQn);
	GIE_ENABLE();	//开启总中断



	DBG("\n");
	DBG("********************************************************************************\n");
	DBG("|                    MVsilicon B1 Example                                      |\n");
	DBG("|            Mountain View Silicon Technology Co.,Ltd.                          |\n");
	DBG("|Audio Decoder Version: %s\n", (unsigned char *)audio_decoder_get_lib_version());

//初始化DAC所需要的DMA通道
    DMA_ChannelAllocTableSet((uint8_t*)DmaChannelMap);

    //初始化DAC
    AudioDAC_Init(ALL, 44100, dac0_dma_buffer, sizeof(dac0_dma_buffer), dac1_dma_buffer, sizeof(dac1_dma_buffer));
    AudioDAC0_ResetDMA();

    //演示用，左右声道音量设固定值
    AudioDAC_VolSet(AUDIO_DAC0, vol_l, vol_r);

    //初始化解码器
    A2dp_DecoderInit();

    //启动蓝牙服务
    BtStackServiceStart();


	while(1);

}

#include "../src/bluetooth/inc/bt_app_interface.h"
static void A2dp_DecoderInit(void)
{
	memset(a2dp_sbcBuf, 0, BT_SBC_DECODER_INPUT_LEN);

	SBC_MemHandle.addr = a2dp_sbcBuf;
	SBC_MemHandle.mem_capacity = BT_SBC_DECODER_INPUT_LEN;
	SBC_MemHandle.mem_len = 0;
	SBC_MemHandle.p = 0;

	SaveA2dpStreamDataToBuffer = SaveDataToSbcBuffer;
}


static void SaveDataToSbcBuffer(uint8_t * data, uint16_t dataLen)
{
	uint32_t	insertLen = 0;
	int32_t		remainLen = 0;

	if(1/*sbcDecoderInitFlag*/)
	{
		remainLen = mv_mremain(&SBC_MemHandle);
		if(BT_SBC_DECODER_INPUT_LEN - remainLen > BT_SBC_LEVEL_LOW ) //水位过门槛后
		{

		}
		if(remainLen <= (dataLen+8))
		{
			BT_DBG("F");//sbc fifo full
			//增加读指针
			//
			return ;
		}
		insertLen = mv_mwrite(data, dataLen, 1,&SBC_MemHandle);
		if(BT_SBC_DECODER_INPUT_LEN - remainLen < BT_SBC_DECODER_INPUT_LEN >> 3)
		{//fifo数据不足时，填入数据及时通知decoder

		}
		if(insertLen != dataLen)
		{
			DBG("insert data len err! i:%ld,d:%d\n", insertLen, dataLen);
		}

		if( !DecoderInitialized )
		{
			int32_t ret = audio_decoder_initialize(decoder_buf, &SBC_MemHandle, (int32_t)IO_TYPE_MEMORY, MSBC_DECODER);
			if( ret != RT_SUCCESS )
				printf(" error audio_decoder_initialize %d\n", ret);
			else
				DecoderInitialized = 1;
		}
	}

}



 void A2dp_Decode(void)
{
	static uint32_t SampleRateCC = 44100;
	uint32_t len;


	if( RT_SUCCESS == audio_decoder_can_continue() )
	{
		if(mv_msize(&SBC_MemHandle) <= SBC_DECODER_FIFO_MIN)
		{
			return;
		}
		if(audio_decoder_decode() == RT_SUCCESS)
		{
			if( SampleRateCC != audio_decoder->song_info->sampling_rate){
				SampleRateCC = audio_decoder->song_info->sampling_rate;
				AudioDAC_SampleRateChange(DAC0, audio_decoder->song_info->sampling_rate);
			}

			while(AudioDAC0DataSpaceLenGet() < audio_decoder->song_info->pcm_data_length);
			AudioDAC0DataSet(audio_decoder->song_info->pcm_addr, audio_decoder->song_info->pcm_data_length  * audio_decoder->song_info->num_channels);

		}else
			DBG("[INFO]: ERROR%d\n", (int)audio_decoder->error_code);
	}
}
static void AudioDAC0_ResetDMA(void)
{
	DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_DAC0_TX);
	DMA_CircularConfig(PERIPHERAL_ID_AUDIO_DAC0_TX, sizeof(dac0_dma_buffer) / 2, (void*)dac0_dma_buffer, sizeof(dac0_dma_buffer));
	DMA_ChannelEnable(PERIPHERAL_ID_AUDIO_DAC0_TX);
}
