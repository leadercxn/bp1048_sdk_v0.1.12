/**
 **************************************************************************************
 * @file    Demo_BT_Hfp.c
 * @brief   此demo专用于测试BT HFP通话AEC效果,并将hfp接收到的data和mic采集到的data保存到sd卡中
 *          该Demo适用于128PIN开发板
 *
 * @author  KK
 * @version V1.0.0
 *
 * $Created: 2020-7-7 19:30:00$
 *
 * @copyright Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
 
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

#include "bt_config.h"
#include "blue_aec.h"
#include "eq.h"
#include "expander.h"
#include "drc.h"
#include "typedefine.h"
#include "audio_adc.h"

#include "ff.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "cvsd_plc.h"
#include "bt_app_interface.h"



#define BT_SBC_PACKET_SIZE					595
#define BT_SBC_DECODER_INPUT_LEN			(8*1024)
#define BT_SBC_LEVEL_HIGH					(BT_SBC_DECODER_INPUT_LEN - BT_SBC_PACKET_SIZE * 4)
#define BT_SBC_LEVEL_LOW					(BT_SBC_PACKET_SIZE *6)//(BT_SBC_LEVEL_HIGH  - BT_SBC_PACKET_SIZE * 3)
#define BT_SBC_LEVEL_START					(BT_SBC_LEVEL_HIGH  - BT_SBC_PACKET_SIZE * 3)
#define SBC_DECODER_FIFO_MIN				(119*2)

uint8_t  a2dp_sbcBuf[BT_SBC_DECODER_INPUT_LEN];

static uint8_t dac0_dma_buffer[5120 * 4] = {0};
static uint8_t dac1_dma_buffer[5120 * 2] = {0};

static uint8_t adc_dma_buffer[5120 * 4] = {0};

static uint8_t decoder_buf[1024 * 40] = {0};
//static uint8_t decoder_buf[1024 * 20] = {0};

static int16_t vol_l = 0x200;
static int16_t vol_r = 0x200;

uint8_t DecoderInitialized = 0;

MemHandle SBC_MemHandle; //a2dp和hfp公用

static void SaveDataToSbcBuffer(uint8_t * data, uint16_t dataLen);
static void AudioDAC0_ResetDMA(void);

//////////////////////////////////////////////////////////////////////
//aec
typedef struct __AecUnit
{
	BlueAECContext 		 ct;
	uint32_t 			 enable;
	int32_t 			 es_level;
	int32_t 		     ns_level;
	uint32_t   	 		 param_cnt;
	uint8_t              channel;

} AecUnit;
AecUnit		mic_aec_unit;


//msbc encoder
#include "sbcenc_api.h"
#define MSBC_CHANNE_MODE	1 		// mono
#define MSBC_SAMPLE_REATE	16000	// 16kHz
#define MSBC_BLOCK_LENGTH	15

#define BT_MSBC_LEVEL_START			(57*2)
#define BT_MSBC_PACKET_LEN			60
#define BT_MSBC_MIC_INPUT_SAMPLE	120 //stereo:480bytes -> mono:240bytes

SBCEncoderContext	sbc_encode_ct;


// sco sync header H2
uint32_t scoSyncHeaderCnt = 0;
uint8_t sco_sync_header[4][2] = 
{
	{0x01, 0x08}, 
	{0x01, 0x38}, 
	{0x01, 0xc8}, 
	{0x01, 0xf8}
};

//AEC
#define FRAME_SIZE					BLK_LEN
#define AEC_SAMPLE_RATE				16000
#define LEN_PER_SAMPLE				2 //mono
#define MAX_DELAY_BLOCK				BT_HFP_AEC_MAX_DELAY_BLK
#define DEFAULT_DELAY_BLK			BT_HFP_AEC_DELAY_BLK

#define BT_MSBC_DECODER_INPUT_LEN	2*1024
#define MSBC_DECODER_FIFO_MIN		10*57
uint8_t hfp_msbcBuf[BT_MSBC_DECODER_INPUT_LEN];

void Hfp_DecoderInit(void);

//发送memory和fifo
MemHandle	HfSend_MemHandle; 
uint8_t		HfSend_fifo[1024 * 2];

//msbc解码后缓存fifo(凑64sample处理一次)
MemHandle	MsbcDecoder_MemHandle; 
uint8_t		MsbcDecoder_fifo[1024 * 2];

//mic采集aec之后缓存数据(用于进行msbc编码)
MemHandle	MsbcEncoder_MemHandle; 
uint8_t		MsbcEncoder_fifo[1024 * 2];

extern void aec_pcm_debug_dump_init(void);

extern void aec_store_process(int16_t* ref, int16_t* mic, uint16_t n);

//////////////////////////////////////////////////////////////////////
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

void error_give_a_pulse(void)
{
	GPIO_RegOneBitSet(GPIO_A_OUT,GPIOA0 << 4);
	GPIO_RegOneBitClear(GPIO_A_OUT,GPIOA0 << 4);
}

void go_into_write_block(void)
{
	GPIO_RegOneBitSet(GPIO_A_OUT,GPIOA0 << 5);
}

void go_outof_write_block(void)
{
	GPIO_RegOneBitClear(GPIO_A_OUT,GPIOA0 << 5);
}

void dump_task(void)
{

	GPIO_PortAModeSet(GPIOA0 << 4, 0);
	GPIO_RegOneBitSet(GPIO_A_OE,GPIOA0 << 4);
	GPIO_RegOneBitClear(GPIO_A_IE,GPIOA0 << 4);
	GPIO_RegOneBitClear(GPIO_A_OUT,GPIOA0 << 4);

	GPIO_PortAModeSet(GPIOA0 << 5, 0);
	GPIO_RegOneBitSet(GPIO_A_OE,GPIOA0 << 5);
	GPIO_RegOneBitClear(GPIO_A_IE,GPIOA0 << 5);
	GPIO_RegOneBitClear(GPIO_A_OUT,GPIOA0 << 5);

	aec_pcm_debug_dump_init();
	while(1)
	{
		aec_debug_dump_process();
		vTaskDelay(1);
	}
}

static int32_t encoder_per_frame;
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

	//初始化DAC所需要的DMA通道
    DMA_ChannelAllocTableSet((uint8_t*)DmaChannelMap);

	prvInitialiseHeap();
	osSemaphoreMutexCreate();

	DBG("\n");
	DBG("********************************************************************************\n");
	DBG("|                    MVsilicon B1 HPF example                                   |\n");
	DBG("|            Mountain View Silicon Technology Co.,Ltd.                          |\n");
	DBG("|Audio Decoder Version: %s\n", (unsigned char *)audio_decoder_get_lib_version());
	DBG("********************************************************************************\n");

	NVIC_EnableIRQ(SWI_IRQn);
	GIE_ENABLE();	//开启总中断


	//AudioDAC_Init(ALL, 44100, dac0_dma_buffer, sizeof(dac0_dma_buffer), dac1_dma_buffer, sizeof(dac1_dma_buffer));
	//AudioDAC0_ResetDMA();

#if (BT_HFP_SUPPORT_WBS == ENABLE)
    //初始化DAC
    AudioDAC_Init(ALL, 16000, dac0_dma_buffer, sizeof(dac0_dma_buffer), dac1_dma_buffer, sizeof(dac1_dma_buffer));
    AudioDAC0_ResetDMA();

    //初始化ADC
	//Mic1 Mic2  analog
	AudioADC_AnaInit();

	AudioADC_MicBias1Enable(TRUE);
	//AudioADC_VcomConfig(1);//MicBias en
	//模拟通道先配置为NONE，防止上次配置通道残留，然后再配置需要的模拟通道

	AudioADC_DynamicElementMatch(ADC1_MODULE, TRUE, TRUE);

	AudioADC_PGASel(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2);
	AudioADC_PGASel(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1);

	//AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2, 15, 2);
	//AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1, 15, 2);
	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2, 6, 4);
	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1, 6, 4);
	
    AudioADC_DigitalInit(ADC1_MODULE, 16000, adc_dma_buffer, sizeof(adc_dma_buffer));

	//encoder
	sbc_encoder_initialize(&sbc_encode_ct, MSBC_CHANNE_MODE, MSBC_SAMPLE_REATE, MSBC_BLOCK_LENGTH, SBC_ENC_QUALITY_MIDDLE, &encoder_per_frame);
#else
	//初始化DAC
    AudioDAC_Init(ALL, 8000, dac0_dma_buffer, sizeof(dac0_dma_buffer), dac1_dma_buffer, sizeof(dac1_dma_buffer));
    AudioDAC0_ResetDMA();

    //初始化ADC
	//Mic1 Mic2  analog
	AudioADC_AnaInit();

	AudioADC_MicBias1Enable(TRUE);
	//AudioADC_VcomConfig(1);//MicBias en
	//模拟通道先配置为NONE，防止上次配置通道残留，然后再配置需要的模拟通道

	AudioADC_DynamicElementMatch(ADC1_MODULE, TRUE, TRUE);

	AudioADC_PGASel(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2);
	AudioADC_PGASel(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1);

	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2, 5, 2);
	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1, 5, 2);
	
	
    AudioADC_DigitalInit(ADC1_MODULE, 8000, adc_dma_buffer, sizeof(adc_dma_buffer));
#endif

    //演示用，左右声道音量设固定值
    AudioDAC_VolSet(AUDIO_DAC0, vol_l, vol_r);

    //初始化解码器
    Hfp_DecoderInit();
	BtHf_AECEffectInit();

    //启动蓝牙服务
    extern void BtStackServiceStart(void);
	xTaskCreate( (TaskFunction_t)dump_task, "dump_task", 512, NULL, 1, NULL );
    xTaskCreate( (TaskFunction_t)BtStackServiceStart, "BtStackServiceStart", 1024, NULL, 2, NULL );

    vTaskStartScheduler();
	while(1);

}

static void AudioDAC0_ResetDMA(void)
{
	DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_DAC0_TX);
	DMA_CircularConfig(PERIPHERAL_ID_AUDIO_DAC0_TX, sizeof(dac0_dma_buffer) / 2, (void*)dac0_dma_buffer, sizeof(dac0_dma_buffer));
	DMA_ChannelEnable(PERIPHERAL_ID_AUDIO_DAC0_TX);
}

////////////////////////////////////////////////////////////////////////////////////////////////
/*******************************************************************************
 * AEC 
 * 用于进行AEC的缓存远端发送数据
 * uint:sample
 ******************************************************************************/
 //aec
uint8_t				AecDelayBuf[BLK_LEN*LEN_PER_SAMPLE*MAX_DELAY_BLOCK];
MemHandle			AecDelayRingBuf;
uint16_t			SourceBuf_Aec[BLK_LEN * 2];

extern void aec_help_effect_Init(void);
void BtHf_AECEffectInit(void)
{
	mic_aec_unit.enable				 = 1;
	mic_aec_unit.es_level    		 = BT_HFP_AEC_ECHO_LEVEL;
	mic_aec_unit.ns_level     		 = BT_HFP_AEC_NOISE_LEVEL;
	mic_aec_unit.channel			 = 1;
	
	blue_aec_init(&mic_aec_unit.ct, mic_aec_unit.es_level, mic_aec_unit.ns_level);
	aec_help_effect_Init();
}

bool BtHf_AECInit(void)
{
	memset(AecDelayBuf, 0, sizeof(AecDelayBuf));
	
	AecDelayRingBuf.addr = AecDelayBuf;
	AecDelayRingBuf.mem_capacity = (BLK_LEN*LEN_PER_SAMPLE*MAX_DELAY_BLOCK);
	AecDelayRingBuf.mem_len = BLK_LEN*LEN_PER_SAMPLE*DEFAULT_DELAY_BLK;
	AecDelayRingBuf.p = 0;
	
	memset(SourceBuf_Aec, 0, sizeof(SourceBuf_Aec));
	return TRUE;
}

void BtHf_AECReset(void)
{
	memset(AecDelayBuf, 0, sizeof(AecDelayBuf));

	AecDelayRingBuf.addr = AecDelayBuf;
	AecDelayRingBuf.mem_capacity = (BLK_LEN*LEN_PER_SAMPLE*MAX_DELAY_BLOCK);
	AecDelayRingBuf.mem_len = BLK_LEN*LEN_PER_SAMPLE*DEFAULT_DELAY_BLK;
	AecDelayRingBuf.p = 0;

	memset(SourceBuf_Aec, 0, sizeof(SourceBuf_Aec));
}

uint32_t BtHf_AECRingDataSet(void *InBuf, uint16_t InLen)
{
	if(InLen == 0)
		return 0;
	
	return mv_mwrite(InBuf, 1, InLen*2, &AecDelayRingBuf);
}

uint32_t BtHf_AECRingDataGet(void* OutBuf, uint16_t OutLen)
{
	if(OutLen == 0)
		return 0;
	
	return mv_mread(OutBuf, 1, OutLen*2, &AecDelayRingBuf);
}

int32_t BtHf_AECRingDataSpaceLenGet(void)
{
	return mv_mremain(&AecDelayRingBuf)/2;
}

int32_t BtHf_AECRingDataLenGet(void)
{
	return mv_msize(&AecDelayRingBuf)/2;
}

int16_t *BtHf_AecInBuf(void)
{
	if(BtHf_AECRingDataLenGet() > BLK_LEN)
	{
		BtHf_AECRingDataGet(SourceBuf_Aec , BLK_LEN);
	}
	else
	{
		memset(SourceBuf_Aec, 0, sizeof(SourceBuf_Aec));
	}
	return (int16_t *)SourceBuf_Aec;
}


/*******************************************************************************
 * hfp sco
 ******************************************************************************/
extern uint8_t lc_sco_data_error_flag; //0=CORRECTLY_RX_FLAG; 1=POSSIBLY_INVALID_FLAG; 2=NO_RX_DATA_FLAG; 3=PARTIALLY_LOST_FLAG;
static int16_t BtHf_SaveScoData(uint8_t* data, uint16_t len);
uint32_t msbc_cnt = 0;

CVSD_PLC_State		cvsdPlcState;
void Hfp_DecoderInit(void)
{
	//接收fifo
	memset(hfp_msbcBuf, 0, BT_MSBC_DECODER_INPUT_LEN);
	SBC_MemHandle.addr = hfp_msbcBuf;
	SBC_MemHandle.mem_capacity = BT_MSBC_DECODER_INPUT_LEN;
	SBC_MemHandle.mem_len = 0;
	SBC_MemHandle.p = 0;
	SaveHfpScoDataToBuffer = BtHf_SaveScoData;

	//发送fifo(cvsd:pcm / msbc编码)
	memset(HfSend_fifo, 0, 2*1024);
	HfSend_MemHandle.addr = HfSend_fifo;
	HfSend_MemHandle.mem_capacity = 2*1024;
	HfSend_MemHandle.mem_len = 0;
	HfSend_MemHandle.p = 0;

	//msbc解码后缓存fifo
	memset(MsbcDecoder_fifo, 0, 2*1024);
	MsbcDecoder_MemHandle.addr = MsbcDecoder_fifo;
	MsbcDecoder_MemHandle.mem_capacity = 2*1024;
	MsbcDecoder_MemHandle.mem_len = 0;
	MsbcDecoder_MemHandle.p = 0;

	//mic采集aec之后缓存数据(用于进行msbc编码)
	memset(MsbcEncoder_fifo, 0, 2*1024);
	MsbcEncoder_MemHandle.addr = MsbcEncoder_fifo;
	MsbcEncoder_MemHandle.mem_capacity = 2*1024;
	MsbcEncoder_MemHandle.mem_len = 0;
	MsbcEncoder_MemHandle.p = 0;

	BtHf_AECInit();
	msbc_cnt=0;
	scoSyncHeaderCnt = 0;

	//plc init
	//cvsd plc config
	memset(&cvsdPlcState, 0, sizeof(CVSD_PLC_State));
	cvsd_plc_init(&cvsdPlcState);
}

uint8_t msbcInputFifo[60];
uint8_t hfpSendBuf[120];
int16_t ScoInputFifo[120];
extern uint8_t lc_sco_data_error_flag; //0=CORRECTLY_RX_FLAG; 1=POSSIBLY_INVALID_FLAG; 2=NO_RX_DATA_FLAG; 3=PARTIALLY_LOST_FLAG;
static int16_t BtHf_SaveScoData(uint8_t* data, uint16_t len)
{
	uint32_t	insertLen = 0;
	int32_t		remainLen = 0;
	int32_t		memLen = 0;
	uint8_t 	validLen = 57;

#if (BT_HFP_SUPPORT_WBS == ENABLE)
	//发
	if(mv_msize(&HfSend_MemHandle) >= 60)
	{
		mv_mread(hfpSendBuf, 60, 1, &HfSend_MemHandle);
		HfpSendScoData(hfpSendBuf, 60);
	}

	//接收到的数据需要做如下判断
	//1.全0 丢弃
	//2.长度非60的，需要放入缓存
	//3.长度为60的直接处理
	if((!lc_sco_data_error_flag))
	{
		//为0数据丢弃
		if((data[0]==0)&&(data[1]==0)&&(data[2]==0)&&(data[3]==0)&&(data[len-2]==0)&&(data[len-1]==0))
			return -1;
	}
	
	//长度
	if(len != BT_MSBC_PACKET_LEN)
	{
		if((data[0] != 0x01)||(data[2] != 0xad)||(data[3] != 0x00))
		{
			BT_DBG("msbc discard...\n");
			return -1;
		}
	}

	if(1/*sbcDecoderInitFlag*/)
	{
		remainLen = mv_mremain(&SBC_MemHandle);
		
		if(remainLen <= (len+8))
		{
			printf("F");//msbc fifo full
			//增加读指针
			//
			return 0;
		}

		if(len != 60)
			return 0;

		if((data[0] != 0x01)||(data[2] != 0xad)||(data[3] != 0x00))
		{
			BT_DBG("msbc data error...\n");
			memset(data, 0x00, len);//CC_TODO: push PLC
			if(!DecoderInitialized)
				return -1;
		}

		if(lc_sco_data_error_flag != 0)
		{
			memset(data, 0x00, len);
		}

		memcpy(msbcInputFifo, &data[2], validLen);

		insertLen = mv_mwrite(msbcInputFifo, validLen, 1,&SBC_MemHandle);
		
		if(insertLen != validLen)
		{
			printf("insert data len err! i:%ld,d:%d\n", insertLen, validLen);
		}

		memLen = mv_msize(&SBC_MemHandle);
		if( !DecoderInitialized )
		{
			int32_t ret = audio_decoder_initialize(decoder_buf, &SBC_MemHandle, (int32_t)IO_TYPE_MEMORY, MSBC_DECODER);
			if( ret != RT_SUCCESS )
				printf(" error audio_decoder_initialize %d\n", ret);
			else
			{
				printf(" msbc decoder start \n");
				DecoderInitialized = 1;
			}

			//AudioDAC_SampleRateChange(ALL, 16000);
		}
	}
#else
	//发
	memLen = mv_msize(&HfSend_MemHandle);
	if(memLen >= 120)
	{
		mv_mread(hfpSendBuf, 120, 1, &HfSend_MemHandle);
		HfpSendScoData(hfpSendBuf, 120);
	}
	
	//cvsd - plc
	/*if(lc_sco_data_error_flag)
	{
		//DBG("sco_error:%d\n", lc_sco_data_error_flag);
		lc_sco_data_error_flag=0;
		cvsd_plc_bad_frame(&cvsdPlcState, ScoInputFifo);
	}
	else
	{
		cvsd_plc_good_frame(&cvsdPlcState, (int16_t *)data, ScoInputFifo);
	}*/
	

	//收
	remainLen = mv_mremain(&SBC_MemHandle);
	if(remainLen <= (len+8))
	{
		printf("F");//msbc fifo full
		//增加读指针
		//
		return 0;
	}
	insertLen = mv_mwrite(data, len, 1,&SBC_MemHandle);

	if(BtHf_AECRingDataSpaceLenGet()>(len/2))
	{
		BtHf_AECRingDataSet(data, (len/2));
	}
	
	if(insertLen != len)
	{
		printf("insert data len err! i:%ld,d:%d\n", insertLen, len);
	}
	if( !DecoderInitialized )
	{
		DecoderInitialized=1;
		//AudioDAC_SampleRateChange(ALL, 8000);
	}
#endif
	return 0;
}

void Hfp_Mic_Process(void)
{
}

#define ONE_BLOCK_WRITE 2048
MemHandle aec_debug_fifo;
static uint8_t aec_debug_raw_buf[4096*8];
static FATFS gFatfs_sd;   /* File system object */
static FIL gFil;    /* File object */
static char current_vol[8];//disk volume like 0:/, 1:/
uint8_t aec_temp_buf[ONE_BLOCK_WRITE];
int16_t aec_temp_buf1[256*2];
uint8_t hfp_status_for_aec_debug = 0;
bool has_sdcard = FALSE;
char file_string[64];

//CC_TODO: pcm data dump funcs
void aec_pcm_debug_dump_init(void)
{
	FRESULT ret;
	uint16_t file_num = 0;
	
	mv_mopen(&aec_debug_fifo, aec_debug_raw_buf, sizeof(aec_debug_raw_buf)-4, NULL);

	GPIO_PortAModeSet(GPIOA20, 1);
	GPIO_PortAModeSet(GPIOA21, 11);
	GPIO_PortAModeSet(GPIOA22, 1);
	GPIO_RegOneBitSet(GPIO_A_PU, GPIO_INDEX20);
	GPIO_RegOneBitClear(GPIO_A_PD, GPIO_INDEX20);
	GPIO_RegOneBitSet(GPIO_A_PU, GPIO_INDEX21);
	GPIO_RegOneBitClear(GPIO_A_PD, GPIO_INDEX21);
	GPIO_RegOneBitSet(GPIO_A_PU, GPIO_INDEX22);
	GPIO_RegOneBitClear(GPIO_A_PD, GPIO_INDEX22);
	DBG("请插入SD卡\n");
	if(SDCard_Init() == 0)
	{
		DBG("SDCard Init Success!\n");
		strcpy(current_vol, "0:/");
		if(f_mount(&gFatfs_sd, current_vol, 1) == 0)
		{
			DBG("SD卡挂载到 0:/--> 成功\n");
			has_sdcard = TRUE;
		}
		else
		{
			DBG("SD卡挂载到 0:/--> 失败\n");
			has_sdcard = FALSE;
			return;
		}
	}
	else
	{
		DBG("SdInit Failed!\n");
		has_sdcard = FALSE;
		return;
	}
}

void aec_store_process(int16_t* ref, int16_t* mic, uint16_t n)
{
	int i;
	static uint8_t cnn = 0;
	
	// DBG("@");
	if(hfp_status_for_aec_debug != 2)
		return;
	
	for(i=0; i<n; i++)
	{
		aec_temp_buf1[i*2+0] = ref[i];
		aec_temp_buf1[i*2+1] = mic[i];
	}
	if(mv_mremain(&aec_debug_fifo) >= n*4)
	{	
		// DBG(".");
		mv_mwrite(aec_temp_buf1, 1, n*4, &aec_debug_fifo);
	}
	else
	{
		if(cnn == 0)
		{
			DBG("\n-F-\n");
			cnn = 100;
		}
		else
		{
			cnn--;
		}
		
	}
}

void aec_debug_dump_process(void)
{
	UINT bw;
	FRESULT ret;
	uint16_t file_num = 0;

	if(!has_sdcard)
		return;
	
	if(hfp_status_for_aec_debug == 1)
	{
		file_num = 0;
		do
		{
			/* code */
			file_num++;
			memset(file_string, 0x00, 64);
			snprintf(file_string, 64, "0:/aec_bt_rec_%02d.pcm", file_num);
			ret = f_open(&gFil, file_string, FA_CREATE_NEW | FA_WRITE);
		} while (ret != FR_OK && file_num < 16);
		
		if(ret == FR_OK)
		{
			DBG("create file OK! %s\n", file_string);
			hfp_status_for_aec_debug = 2;
		}
		else
		{
			DBG("create file error! %d\n", ret);
			hfp_status_for_aec_debug = 4;
		}
		
	}
	else if(hfp_status_for_aec_debug == 2)
	{
		// DBG("&");
		if(mv_msize(&aec_debug_fifo) >= ONE_BLOCK_WRITE)
		{
			// DBG("*");
			mv_mread(aec_temp_buf, 1, ONE_BLOCK_WRITE, &aec_debug_fifo);
			ret = f_write(&gFil, aec_temp_buf, ONE_BLOCK_WRITE, &bw);
			// f_sync(&gFil);
			if(ret != FR_OK)
			{
				DBG("werr:%d ", ret);
			}
			// DBG("%d ", bw);
			// f_sync(&gFil);
		}
	}
	else if(hfp_status_for_aec_debug == 3)
	{
		f_close(&gFil);
		DBG("file close ok!\n");
		hfp_status_for_aec_debug = 4;
	}
}

EQContext eq_for_aec_cnx;
ExpanderContext expander_for_aec_cnx;
DRCContext drc_for_aec_cnx;
EQFilterParams eq_for_aec_filter_buf[10];
void aec_help_effect_Init(void)
{
	int i;
#if	BT_HFP_SUPPORT_WBS == ENABLE
	uint16_t sample_rate = 16000;
#else
	uint16_t sample_rate = 8000;
#endif
	uint32_t 		 q[2];
	int32_t  		 threshold[3];
	int32_t  		 ratio[3];
	int32_t  		 attack_tc[3];
	int32_t  		 release_tc[3];

	for(i = 0; i < 4; i++)
	{
		eq_for_aec_filter_buf[i].type = EQ_FILTER_TYPE_HIGH_PASS;
		eq_for_aec_filter_buf[i].f0 = 60;
		eq_for_aec_filter_buf[i].Q = 724;
		eq_for_aec_filter_buf[i].gain = 0;
	}
	eq_init(&eq_for_aec_cnx, sample_rate, eq_for_aec_filter_buf, 4, 0, 1);
	expander_init(&expander_for_aec_cnx, 1, sample_rate, -4500, 3, 5, 500);

	//CC_TODO: drc init
	q[0] = 0;
	q[1] = 0;
	threshold[0] = -600;
	threshold[1] = -600;
	threshold[2] = -600;
	ratio[0] = 10;
	ratio[1] = 10;
	ratio[2] = 10;
	attack_tc[0] = 1;
	attack_tc[1] = 1;
	attack_tc[2] = 1;
	release_tc[0] = 500;
	release_tc[1] = 500;
	release_tc[2] = 500;
	drc_init(&drc_for_aec_cnx, 1, sample_rate, 0/*fc*/, 0/*mode*/, q, threshold, ratio, attack_tc, release_tc);
}

int16_t pcm_fifo[1024]={0};
int16_t cvsd_fifo[240]={0};
uint32_t micAdcBuf[512];
//int16_t micAdcBuf[512];
int16_t pcmDelayBuf[256];
int16_t micBuf[256];
int16_t micBackBuf[256];
int16_t micAecBuf[256];
int16_t micEncoderBuf[240];
uint8_t msbcEncodedBuf[60];
void Hfp_Decode(void)
{
	static uint32_t SampleRateCC = 16000;
	uint32_t len;
	uint8_t i;
	uint16_t *saveToPcm;
	int32_t		remainLen = 0;
#if(BT_HFP_SUPPORT_WBS == ENABLE)
	//if(!DecoderInitialized)
	//	return;

	//if(AudioDAC0DataSpaceLenGet()<120)
	//	return;

	//decoder
	if(mv_msize(&SBC_MemHandle) >= 57)
	{
		if(!DecoderInitialized)
			return;
		
		if( RT_SUCCESS == audio_decoder_can_continue() )
		{
			if(audio_decoder_decode() == RT_SUCCESS)
			{
				
				if( SampleRateCC != audio_decoder->song_info->sampling_rate)
				{
					printf("[Warming]:msbc sample rate =%d \n", audio_decoder->song_info->sampling_rate);
				}
				saveToPcm = (uint16_t*)audio_decoder->song_info->pcm_addr;
				memcpy(cvsd_fifo, saveToPcm, audio_decoder->song_info->pcm_data_length*2);
				
				if(mv_mremain(&MsbcDecoder_MemHandle)>(audio_decoder->song_info->pcm_data_length*2))
				{
					mv_mwrite(cvsd_fifo, (audio_decoder->song_info->pcm_data_length*2), 1, &MsbcDecoder_MemHandle);
				}
				else
				{
					DBG("msbc decoder fifo full\n");
				}
			}else
				printf("[INFO]: ERROR%d\n", (int)audio_decoder->error_code);
		}
	}

	//adc in //CC_TODO: do aec here
	if(AudioADC_DataLenGet(ADC1_MODULE) >= BLK_LEN)
	{
		AudioADC_DataGet(ADC1_MODULE, micAdcBuf, BLK_LEN);

		for(i=0;i<BLK_LEN;i++)
		{
			micBuf[i] = (uint16_t)micAdcBuf[i]; 
		}
		
		if(!DecoderInitialized)
		{
			return;
		}
		
		if(BtHf_AECRingDataLenGet()>=BLK_LEN)
		{
			BtHf_AECRingDataGet(pcmDelayBuf,BLK_LEN);
			aec_store_process(pcmDelayBuf, micBuf, BLK_LEN);
#if 1			
			//eq_apply(&eq_for_aec_cnx, micBuf, micBuf, BLK_LEN);
			blue_aec_run(&mic_aec_unit.ct,  (int16_t *)(pcmDelayBuf), (int16_t *)(micBuf), (int16_t *)(micAecBuf));
			// expander_apply(&expander_for_aec_cnx, micAecBuf, micAecBuf, BLK_LEN);
			//drc_apply(&drc_for_aec_cnx, micAecBuf, micAecBuf, BLK_LEN, 32767, 32767);
#else
			memcpy(micAecBuf, micBuf, BLK_LEN*2);
#endif
		}
		else
		{
			for(i=0;i<BLK_LEN;i++)
			{
				micAecBuf[i] = (uint16_t)micBuf[i]; 
			}
		}
		
		if(mv_mremain(&MsbcEncoder_MemHandle) > BLK_LEN*2)
			mv_mwrite(micAecBuf, BLK_LEN*2, 1, &MsbcEncoder_MemHandle);
		else
			printf("mic aec fifo full\n");
	}

	//msbc encoder
	if(mv_msize(&MsbcEncoder_MemHandle)>=(120*2))//120sample
	{
		uint32_t encodedLen=0;
		
		if(!DecoderInitialized)
		{
			return;
		}
		
		mv_mread(micEncoderBuf, 120*2, 1, &MsbcEncoder_MemHandle);

		memset(msbcEncodedBuf, 0, 60);
		if(scoSyncHeaderCnt>3) scoSyncHeaderCnt = 0;
		memcpy(msbcEncodedBuf, sco_sync_header[scoSyncHeaderCnt], 2);
		scoSyncHeaderCnt++;
		scoSyncHeaderCnt %= 4;
		
		sbc_encoder_encode(&sbc_encode_ct, (int16_t*)micEncoderBuf, &msbcEncodedBuf[2], &encodedLen);
		if(encodedLen!=57)
		{
			printf("msbc encoded len:%d\n", encodedLen);
		}

		if(mv_mremain(&HfSend_MemHandle)>60)
		{
			mv_mwrite(msbcEncodedBuf, 60, 1, &HfSend_MemHandle);
		}
		else
		{
			printf("hf send fifo full\n");
		}
	}

	//dac out
	if(AudioDAC0DataSpaceLenGet() >= BLK_LEN)
	{
		if(!DecoderInitialized)
		{
			return;
		}
		
		if(mv_msize(&MsbcDecoder_MemHandle)>=(BLK_LEN*2))
		{
			memset(cvsd_fifo, 0, BLK_LEN*2);
			memset(pcm_fifo, 0, BLK_LEN*4);
			
			mv_mread(cvsd_fifo, BLK_LEN*2, 1, &MsbcDecoder_MemHandle);
			memcpy(pcmDelayBuf, cvsd_fifo, BLK_LEN*2);
			
			for(i=0;i<(BLK_LEN*2);i++)
			{
				pcm_fifo[i*2]=cvsd_fifo[i];
				pcm_fifo[i*2+1]=cvsd_fifo[i];
			}
			AudioDAC0DataSet(pcm_fifo, BLK_LEN);

			//aec: referrence data
			if(BtHf_AECRingDataSpaceLenGet()>(BLK_LEN))
			{
				BtHf_AECRingDataSet(pcmDelayBuf, (BLK_LEN));
			}
			else
			{
				printf("aec referrence data fifo full\n");
			}
		}
	}
#else
	int32_t		memLen = 0;
	//adc in: mic
	if(AudioADC_DataLenGet(ADC1_MODULE) >= BLK_LEN)
	{
		AudioADC_DataGet(ADC1_MODULE, micAdcBuf, BLK_LEN);

		for(i=0;i<BLK_LEN;i++)
		{
			micBuf[i] = (uint16_t)micAdcBuf[i];

			//micBuf[i] = ((int32_t)micAdcBuf[2 * i + 0] + (int32_t)micAdcBuf[2 * i + 1]);
		}
		
		if(!DecoderInitialized)
		{
			return;
		}

		if(BtHf_AECRingDataLenGet()>=BLK_LEN)
		{
			BtHf_AECRingDataGet(pcmDelayBuf,BLK_LEN);
			aec_store_process(pcmDelayBuf, micBuf, BLK_LEN);
//			eq_apply(&eq_for_aec_cnx, micBuf, micBuf, BLK_LEN);
			blue_aec_run(&mic_aec_unit.ct,  (int16_t *)(pcmDelayBuf), (int16_t *)(micBuf), (int16_t *)(micAecBuf));
//			drc_apply(&drc_for_aec_cnx, micAecBuf, micAecBuf, BLK_LEN, 32767, 32767);
			for(i=0;i<BLK_LEN;i++)
			{
				micAecBuf[i] = (micAecBuf[i]*2);
			}
			// memcpy(micAecBuf, micBuf, BLK_LEN*2);
			mv_mwrite(micAecBuf, BLK_LEN*2, 1, &HfSend_MemHandle);//KK 20200702
		}
		else
		{
			for(i=0;i<BLK_LEN;i++)
			{
				micAecBuf[i] = (uint16_t)micBuf[i]; 
			}
		}
//		mv_mwrite(micAecBuf, BLK_LEN*2, 1, &HfSend_MemHandle);//KK 20200702
	}
	
	//dac out
	if(AudioDAC0DataSpaceLenGet() >= BLK_LEN)
	{
		memLen = mv_msize(&SBC_MemHandle);
		if(memLen > BLK_LEN*2)
		{
			mv_mread(cvsd_fifo, BLK_LEN*2, 1, &SBC_MemHandle);
			for(i=0;i<BLK_LEN;i++)
			{
				pcm_fifo[i*2]=cvsd_fifo[i];
				pcm_fifo[i*2+1]=cvsd_fifo[i];
			}
			AudioDAC0DataSet(pcm_fifo, BLK_LEN);
		}
	}
#endif
}

