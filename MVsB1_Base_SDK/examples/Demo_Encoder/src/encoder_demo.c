/**
 **************************************************************************************
 * @file    voice_app_demo.c
 * @brief
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2017-10-30 11:30:00$
 *
 * @copyright Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <stdlib.h>
#include <stdio.h>
#include <nds32_intrinsic.h>
//#include <string.h>
#include "uarts.h"
#include "uarts_interface.h"
#include "type.h"
#include "debug.h"
#include "timeout.h"
#include "clk.h"
#include "dma.h"
#include "dac.h"
#include "timer.h"
#include "adc.h"
#include "i2s.h"
#include "watchdog.h"
#include "spi_flash.h"
#include "remap.h"
#include "audio_adc.h"
#include "gpio.h"
#include "chip_info.h"
#include "irqn.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "adc_interface.h"
#include "dac_interface.h"
#include "rtos_api.h"
#include "ff.h"
#include "mvstdio.h"
#include "sd_card.h"
#include "sbcenc_api.h"
#include "mp3enc_api.h"
#include "mp2enc_api.h"
#include "adpcm_encoder.h"

#define ENC_TYPE_NONE 0
#define ENC_TYPE_SBC 1
#define ENC_TYPE_MSBC 2
#define ENC_TYPE_MP3 3
#define ENC_TYPE_MP2 4
#define ENC_TYPE_ADPCM 5
#define ENC_TYPE ENC_TYPE_NONE


#define MP3_ENC_BITRATE 256
#define MP2_ENC_BITRATE 256
#define ADPCM_ENC_BLOCK_SIZE 256
//#define IS_MCPS_TEST

#define ADC_MIC_BLOCK 256 //samples

//定义2个全局buf，用于缓存ADC和DAC的数据，注意单位
uint32_t AudioADCBuf[2048] = {0}; //1024 * 4 = 4K
uint32_t AudioDACBuf[2048] = {0}; //1024 * 4 = 4K

uint8_t		gRecordFlg = FALSE;

static int16_t PcmBuf1[ADC_MIC_BLOCK*2] = {0};
static int16_t FileIOBuf[4096] = {0};
static uint8_t data[4096] = {0};
uint8_t pcm_buf_raw[4095*4];
MemHandle pcm_buffer;

uint8_t enc_buf_raw[4095*4];
MemHandle enc_buffer;

#define FILE_IO_LEN 512
uint8_t file_io_buf[FILE_IO_LEN*2];

FATFS 	gFatfs_sd;   				/* File system object */
FIL 	gFil;

const char enc_filename[8][16] = {"0:/record.pcm", "0:/record.sbc", "0:/recordm.sbc", "0:/record.mp3", "0:/record.mp2", "0:/record.wav"};

#if ENC_TYPE == ENC_TYPE_SBC
static SBCEncoderContext sbc_ct;
#elif ENC_TYPE == ENC_TYPE_MSBC
static SBCEncoderContext msbc_ct;
#elif ENC_TYPE == ENC_TYPE_MP3
static MP3EncoderContext mp3enc_ct;
#elif ENC_TYPE == ENC_TYPE_MP2
static MP2EncoderContext mp2enc_ct;
#elif ENC_TYPE == ENC_TYPE_ADPCM
static ADPCMEncoderContext adpcmenc_ct;
static ADPCMHeader adpcm_hdr;
uint8_t hdr_wr_cnt = 0;
#endif

uint8_t ch_num = 2;
uint16_t SampleRate = 44100;

static uint8_t DmaChannelMap[29] = {
	255,//PERIPHERAL_ID_SPIS_RX = 0,		//0
	255,//PERIPHERAL_ID_SPIS_TX,			//1
	255,//PERIPHERAL_ID_TIMER3,			//2
	2,//PERIPHERAL_ID_SDIO_RX,			//3
	3,//PERIPHERAL_ID_SDIO_TX,			//4
	255,//PERIPHERAL_ID_UART0_RX,			//5
	255,//PERIPHERAL_ID_TIMER1,				//6
	255,//PERIPHERAL_ID_TIMER2,				//7
	255,//PERIPHERAL_ID_SDPIF_RX,			//8 SPDIF_RX /TX需使用同一通道
	255,//PERIPHERAL_ID_SDPIF_TX,			//8
	255,//PERIPHERAL_ID_SPIM_RX,			//9
	255,//PERIPHERAL_ID_SPIM_TX,			//10
	255,//PERIPHERAL_ID_UART0_TX,			//11
	255,//PERIPHERAL_ID_UART1_RX,			//12
	255,//PERIPHERAL_ID_UART1_TX,			//13
	255,//PERIPHERAL_ID_TIMER4,				//14
	255,//PERIPHERAL_ID_TIMER5,				//15
	255,//PERIPHERAL_ID_TIMER6,				//16
	4,//PERIPHERAL_ID_AUDIO_ADC0_RX,		//17
	5,//PERIPHERAL_ID_AUDIO_ADC1_RX,		//18
	0,//PERIPHERAL_ID_AUDIO_DAC0_TX,		//19
	1,//PERIPHERAL_ID_AUDIO_DAC1_TX,		//20
	255,//PERIPHERAL_ID_I2S0_RX,			//21
	255,//PERIPHERAL_ID_I2S0_TX,			//22
	255,//PERIPHERAL_ID_I2S1_RX,			//23
	255,//PERIPHERAL_ID_I2S1_TX,			//24
	255,//PERIPHERAL_ID_PPWM,				//25
	255,//PERIPHERAL_ID_ADC,     			//26
	255,//PERIPHERAL_ID_SOFTWARE,			//27
};

bool SdInitAndFsMount(void)
{
    uint16_t  Cnt = 2;//上电的时候SD卡检测2次

    bool IsFsMounted = FALSE;

	//SDIO config
    GPIO_PortAModeSet(GPIOA20, 1);
	GPIO_PortAModeSet(GPIOA21, 11);
	GPIO_PortAModeSet(GPIOA22, 1);
	GPIO_RegOneBitSet(GPIO_A_PU, GPIO_INDEX20);
	GPIO_RegOneBitClear(GPIO_A_PD, GPIO_INDEX20);
	GPIO_RegOneBitSet(GPIO_A_PU, GPIO_INDEX21);
	GPIO_RegOneBitClear(GPIO_A_PD, GPIO_INDEX21);
	GPIO_RegOneBitSet(GPIO_A_PU, GPIO_INDEX22);
	GPIO_RegOneBitClear(GPIO_A_PD, GPIO_INDEX22);

	while(Cnt--)
	{
		if(SDCard_Init() == NONE_ERR)
		{
			DBG("SDCard Init Success!\n");
			break;
		}
		else
		{
			DBG("SdInit Failed!\n");
			goto end;
		}
	}

	if(f_mount(&gFatfs_sd, "0:/", 1) == 0)
	{
		DBG("SD卡挂载到 0:/--> 成功\n");
	}
	else
	{
		DBG("SD卡挂载到 0:/--> 失败\n");
		goto end;
	}

	IsFsMounted = TRUE;

end:
    return IsFsMounted;
}

bool encoder_init(int32_t num_channels, int32_t sample_rate, int32_t *samples_per_frame)
{
	int32_t ret = -1;

#if ENC_TYPE == ENC_TYPE_NONE
	ret = 0;
	*samples_per_frame = ADC_MIC_BLOCK;
#elif ENC_TYPE == ENC_TYPE_SBC
	ret = sbc_encoder_initialize(&sbc_ct, num_channels, sample_rate, 16, SBC_ENC_QUALITY_HIGH, samples_per_frame);
#elif ENC_TYPE == ENC_TYPE_MSBC
	ret = sbc_encoder_initialize(&msbc_ct, num_channels, sample_rate, 15, SBC_ENC_QUALITY_HIGH, samples_per_frame);
#elif ENC_TYPE == ENC_TYPE_MP3
	*samples_per_frame = (sample_rate > 32000)?(1152):(576);
	ret = mp3_encoder_initialize(&mp3enc_ct, num_channels, sample_rate, MP3_ENC_BITRATE, 0);
#elif ENC_TYPE == ENC_TYPE_MP2
	*samples_per_frame = 1152;
	ret = mp2_encoder_initialize(&mp2enc_ct, num_channels, sample_rate, MP2_ENC_BITRATE);
#elif ENC_TYPE == ENC_TYPE_ADPCM
	ret = adpcm_encoder_initialize(&adpcmenc_ct, num_channels, sample_rate, ADPCM_ENC_BLOCK_SIZE);
	*samples_per_frame = adpcmenc_ct.samples_per_block;
	//DBG("ADPCM samples of per block: %d\n", adpcmenc_ct.samples_per_block);
#endif

	return (ret == 0);
}

bool encoder_encode(int16_t *pcm_in, uint8_t *data_out, uint32_t *plength)
{
	int32_t ret = -1;
#if ENC_TYPE == ENC_TYPE_NONE
	memcpy(data_out, pcm_in, ADC_MIC_BLOCK*2*ch_num);
	*plength = ADC_MIC_BLOCK*2*ch_num;
	ret = 0;
#elif ENC_TYPE == ENC_TYPE_SBC
	ret = sbc_encoder_encode(&sbc_ct, pcm_in, data_out, plength);
#elif ENC_TYPE == ENC_TYPE_MSBC
	ret = sbc_encoder_encode(&msbc_ct, pcm_in, data_out, plength);
#elif ENC_TYPE == ENC_TYPE_MP3
	ret = mp3_encoder_encode(&mp3enc_ct, pcm_in, data_out, plength);
#elif ENC_TYPE == ENC_TYPE_MP2
	ret = mp2_encoder_encode(&mp2enc_ct, pcm_in, data_out, plength);
#elif ENC_TYPE == ENC_TYPE_ADPCM
	ret = adpcm_encoder_encode(&adpcmenc_ct, pcm_in, data_out, plength);
	memcpy(&adpcm_hdr, adpcm_encoder_get_file_header(&adpcmenc_ct), sizeof(ADPCMHeader));
#endif

	return (ret == 0);
}


static void AudioMicTask(void)
{
  uint16_t  i;

  //DAC init
  AudioDAC_Init(DAC0, SampleRate, (void*)AudioDACBuf, sizeof(AudioDACBuf), NULL, 0);

//	//Mic1 Mic2  analog
	AudioADC_AnaInit();

//	AudioADC_VcomConfig(1);//MicBias en
	//模拟通道先配置为NONE，防止上次配置通道残留，然后再配置需要的模拟通道
	AudioADC_MicBias1Enable(TRUE);

	AudioADC_DynamicElementMatch(ADC1_MODULE, TRUE, TRUE);

	AudioADC_PGASel(ADC1_MODULE, CHANNEL_RIGHT, LINEIN_NONE);
	AudioADC_PGASel(ADC1_MODULE, CHANNEL_LEFT, LINEIN_NONE);

	AudioADC_PGASel(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2);
	AudioADC_PGASel(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1);

	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2, 0, 4);
	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1, 0, 4);

	AudioADC_DigitalInit(ADC1_MODULE, SampleRate, (void*)AudioADCBuf, sizeof(AudioADCBuf));

	while(1)
	{
		if(AudioADC_DataLenGet(ADC1_MODULE) >= ADC_MIC_BLOCK)
		{
			AudioADC_DataGet(ADC1_MODULE, PcmBuf1, ADC_MIC_BLOCK);
			AudioDAC_DataSet(DAC0, PcmBuf1, ADC_MIC_BLOCK);

			if(gRecordFlg)
			{
				if(ch_num == 1)
				{
					for(i=0; i<ADC_MIC_BLOCK; i++)
					{
						PcmBuf1[i] = __nds32__clips(((int32_t)PcmBuf1[i*2] + PcmBuf1[i*2+1]), (16)-1);
					}
				}

				if(mv_mremain(&pcm_buffer) > ADC_MIC_BLOCK*2*ch_num)
				{
					mv_mwrite(PcmBuf1, 1, ADC_MIC_BLOCK*2*ch_num, &pcm_buffer);
				}
				else
				{
					DBG("F");
				}
			}//write buffer

		}
		else
		{
			vTaskDelay(4);
		}
	}
}

void EncoderTask(void)
{
	int32_t spf;
	uint32_t msbclen;
	//uint8_t *output_ptr;

	if(1)
	{

		if(FALSE == encoder_init(ch_num, SampleRate, &spf))
		{
			DBG("init error!\n");
			goto encoder_err;
		}

		DBG("init ok!\n");

		while(1)
		{
			if(gRecordFlg)
			{
				if(mv_msize(&pcm_buffer) >= spf*2*ch_num)
				{
					mv_mread(FileIOBuf, 1, spf*2*ch_num, &pcm_buffer);
#ifdef IS_MCPS_TEST
					GIE_DISABLE();
					__nds32__mtsr(0, NDS32_SR_PFMC0);
					__nds32__mtsr(1, NDS32_SR_PFM_CTL);
#endif
					//main encoder entry
					if(FALSE == encoder_encode(FileIOBuf, data, &msbclen))
						goto encoder_err;

#ifdef IS_MCPS_TEST
					__nds32__mtsr(0, NDS32_SR_PFM_CTL);
					GIE_ENABLE();
					DBG("%d\n", (uint64_t)__nds32__mfsr(NDS32_SR_PFMC0)*SampleRate/spf/1000000);
#endif
					if(mv_mremain(&enc_buffer) > msbclen)
						mv_mwrite(data, 1, msbclen, &enc_buffer);
					else
						DBG("B");
				}
				else
				{
					vTaskDelay(2);
				}
			}//do encoder
			else
			{
				vTaskDelay(2);
			}

		}//while(1)
	}

encoder_err:
	DBG("mp3 encoder error.");
	while(1)
	{
		vTaskDelay(2);
	}

}

void disk_io_task(void)
{
	uint8_t c;
	UINT br;

	if(SdInitAndFsMount())
	{
		while(1)
		{
			if(gRecordFlg && mv_msize(&enc_buffer) >= FILE_IO_LEN)
			{
				mv_mread(file_io_buf, 1, FILE_IO_LEN, &enc_buffer);
				f_write(&gFil, file_io_buf, FILE_IO_LEN, &br);
#if ENC_TYPE == ENC_TYPE_ADPCM
				if(++hdr_wr_cnt%32 == 0)
				{
					f_lseek(&gFil, 0);
					f_write(&gFil, (uint8_t*)&adpcm_hdr, sizeof(ADPCMHeader), &br);
					//f_sync(&gFil);//
					f_lseek(&gFil, f_size(&gFil));
					//DBG(".");
				}
#endif
			}
			else
			{
				vTaskDelay(2);
			}

			if(UART1_RecvByte(&c))
			{
				if(c == 'r')
				{
					if(gRecordFlg == FALSE)
					{
						if(f_open(&gFil, enc_filename[ENC_TYPE], FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
						{
							DBG("Open file error!\n");
						}
						else
						{
							DBG("Open File ok: %s\n", enc_filename[ENC_TYPE]);
#if ENC_TYPE == ENC_TYPE_ADPCM
							f_write(&gFil, (uint8_t*)&adpcm_hdr, sizeof(ADPCMHeader), &br);
#endif
						}

						mv_mopen(&pcm_buffer, pcm_buf_raw, sizeof(pcm_buf_raw)-4, NULL);
						mv_mopen(&enc_buffer, enc_buf_raw, sizeof(enc_buf_raw)-4, NULL);

						gRecordFlg = TRUE;
						DBG("start rec.\n");
					}
					else
					{
#if ENC_TYPE == ENC_TYPE_ADPCM
						f_lseek(&gFil, 0);
						f_write(&gFil, (uint8_t*)&adpcm_hdr, sizeof(ADPCMHeader), &br);
#endif
						f_close(&gFil);
						gRecordFlg = FALSE;
						DBG("\nstop record\n");
					}
				}//c == 'r'
			}//uart
		}
	}
	else
	{
		DBG("SD card error!\n");
		while(1)
		{
			vTaskDelay(1);
		}
	}
}


//语音处理演示Demo
//需要fatfs，RTOS中间件
int main(void)
{
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

	prvInitialiseHeap();
  osSemaphoreMutexCreate();

	DBG("\n");
  DBG("/-----------------------------------------------------\\\n");
  DBG("|                 B1X Encode Example                  |\n");
  DBG("|      Mountain View Silicon Technology Co.,Ltd.      |\n");
  DBG("\\-----------------------------------------------------/\n");
  DBG("\n");

	NVIC_EnableIRQ(SWI_IRQn);
	GIE_ENABLE();	//开启总中断

#if ENC_TYPE == ENC_TYPE_NONE
	ch_num = 2;
	SampleRate = 16000;
#elif ENC_TYPE == ENC_TYPE_SBC
	ch_num = 2;
	SampleRate = 44100;
#elif ENC_TYPE == ENC_TYPE_MSBC
	ch_num = 1;
	SampleRate = 16000;
#elif ENC_TYPE == ENC_TYPE_MP3
	ch_num = 2;
	SampleRate = 44100;
#elif ENC_TYPE == ENC_TYPE_MP2
	ch_num = 2;
	SampleRate = 44100;
#elif ENC_TYPE == ENC_TYPE_ADPCM
	ch_num = 2;
	SampleRate = 44100;
#else
#error "not supported encoder type"
#endif

	mv_mopen(&pcm_buffer, pcm_buf_raw, sizeof(pcm_buf_raw)-4, NULL);
	mv_mopen(&enc_buffer, enc_buf_raw, sizeof(enc_buf_raw)-4, NULL);

	xTaskCreate( (TaskFunction_t)AudioMicTask, "AudioMicTask", 512, NULL, 1, NULL );
	xTaskCreate( (TaskFunction_t)EncoderTask, "EncoderTask", 1024, NULL, 1, NULL );
	xTaskCreate( (TaskFunction_t)disk_io_task, "disk_io_task", 1024, NULL, 1, NULL );

	vTaskStartScheduler();

	while(1);

}
