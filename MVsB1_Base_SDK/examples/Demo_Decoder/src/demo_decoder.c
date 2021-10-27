/**
 **************************************************************************************
 * @file    demo_decoder.c
 * @brief   decoder example
 *
 * @author  Castle
 * @version V1.0.0
 *
 * $Created: 2018-01-04 19:17:00$
 *
 * @Copyright (C) 2017, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include <stdlib.h>
#include <nds32_intrinsic.h>
#include <string.h>
#include "watchdog.h"
#include "sys.h"
#include "gpio.h"
#include "uarts.h"
#include "uarts_interface.h"
#include "dac_interface.h"
#include "type.h"
#include "debug.h"
#include "timeout.h"
#include "clk.h"
#include "dma.h"
#include "timer.h"
#include "spi_flash.h"
#include "remap.h"
#include "irqn.h"
#include "sdio.h"
#include "sd_card.h"
#include "dac.h"
#include "chip_info.h"
#include "ff.h"
#include "ffpresearch.h"

#include "audio_decoder_api.h"
#include "typedefine.h"

//#define USE_UDISK_PLAY

static uint8_t SDIO_DmaChannelMap[29] = {
	255,//PERIPHERAL_ID_SPIS_RX = 0,		//0
	255,//PERIPHERAL_ID_SPIS_TX,			//1
	255,//PERIPHERAL_ID_TIMER3,			//2
	2,//PERIPHERAL_ID_SDIO_RX,			//3
	3,//PERIPHERAL_ID_SDIO_TX,			//4
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
	255,//PERIPHERAL_ID_AUDIO_ADC0_RX,		//18
	255,//PERIPHERAL_ID_AUDIO_ADC1_RX,		//19
	0,//PERIPHERAL_ID_AUDIO_DAC0_TX,		//20
	1,//PERIPHERAL_ID_AUDIO_DAC1_TX,		//21
	255,//PERIPHERAL_ID_I2S0_RX,			//22
	255,//PERIPHERAL_ID_I2S0_TX,			//23
	255,//PERIPHERAL_ID_I2S1_RX,			//24
	255,//PERIPHERAL_ID_I2S1_TX,			//25
	255,//PERIPHERAL_ID_PPWM,				//26
	255,//PERIPHERAL_ID_ADC,     			//27
	255,//PERIPHERAL_ID_SOFTWARE,			//28
};

static uint32_t SampleRateCC = 48000;

int16_t vol_l = 0x200;
int16_t vol_r = 0x200;
int16_t vol_x = 0x200;


FATFS gFatfs_u;   /* File system object */
FATFS gFatfs_sd;   /* File system object */

FIL gFil;    /* File object */
FIL gFilx;    /* File object */
ff_dir gDirs;    /* Directory object */
static uint32_t play_samples;

static uint8_t dac0_dma_buffer[5120 * 4] = {0};
static uint8_t dac1_dma_buffer[5120 * 2] = {0};

static uint8_t decoder_buf[1024 * 40] = {0};

static char current_vol[8];//disk volume like 0:/, 1:/

//#define USE_UDISK_PLAY
#ifdef USE_UDISK_PLAY
void Timer2Interrupt(void)
{
	Timer_InterruptFlagClear(TIMER2, UPDATE_INTERRUPT_SRC);
	OTG_PortLinkCheck();
}
#endif

static void AudioDAC0_ResetDMA(void)
{
	DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_DAC0_TX);
	DMA_CircularConfig(PERIPHERAL_ID_AUDIO_DAC0_TX, sizeof(dac0_dma_buffer) / 2, (void*)dac0_dma_buffer, sizeof(dac0_dma_buffer));
	DMA_ChannelEnable(PERIPHERAL_ID_AUDIO_DAC0_TX);
}

static void AudioDAC1_ResetDMA(void)
{
	DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_DAC1_TX);
	DMA_CircularConfig(PERIPHERAL_ID_AUDIO_DAC1_TX, sizeof(dac1_dma_buffer) / 2, (void*)dac1_dma_buffer, sizeof(dac1_dma_buffer));
	DMA_ChannelEnable(PERIPHERAL_ID_AUDIO_DAC1_TX);
}


int32_t bfloor(int64_t x, int32_t intbits, int32_t fracbits)
{
    int64_t min = -(1LL << (intbits - 1 + fracbits));
    int64_t max =  (1LL << (intbits - 1 + fracbits)) - 1;

    if(x < min)
    {
        x = min;
    }

    if(x > max)
    {
        x = max;
    }

    return (int32_t)x;
}

char file_long_name[FF_LFN_BUF+1];
uint8_t acc_ram_blk[MAX_ACC_RAM_SIZE];
int main(void)
{
	int32_t i;

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

	SysTickInit();

	Remap_InitTcm(0, 12);
	SpiFlashInit(80000000, MODE_4BIT, 0, 1);

	DBG("\n");
	DBG("/-----------------------------------------------------\\\n");
	DBG("|                   Decoder Example                   |\n");
	DBG("|      Mountain View Silicon Technology Co.,Ltd.      |\n");
	DBG("\\-----------------------------------------------------/\n");
	DBG("\n");

	DMA_ChannelAllocTableSet(SDIO_DmaChannelMap);
	SysTickInit();
	GIE_ENABLE();	//开启总中断

#ifdef  USE_UDISK_PLAY
 	Timer_Config(TIMER2,1000,0);
 	Timer_Start(TIMER2);
 	NVIC_EnableIRQ(Timer2_IRQn);

	DBG("请插入UDISK\n");
	{
		OTG_HostControlInit();
		OTG_HostEnumDevice();
		if(UDiskInit() == 1)
		{
			DBG("枚举MASSSTORAGE接口OK\n");
			strcpy(current_vol, "1:/");
			if(f_mount(&gFatfs_u, current_vol, 1) == 0)
			{
				DBG("U盘卡挂载到 1:/--> 成功\n");
			}
			else
			{
				DBG("U盘卡挂载到 1:/--> 失败\n");
				goto ERROR;
			}
		}
		else
		{
			DBG("UDisk init Failed!\n");
			goto ERROR;
		}
	}

#else
	//动态监测SDCard插入
	//SDIO1 gpio usage
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
		}
		else
		{
			DBG("SD卡挂载到 0:/--> 失败\n");
			goto ERROR;
		}
	}
	else
	{
		DBG("SdInit Failed!\n");
		goto ERROR;
	}
#endif

	//search all audio files
	ffpresearch_init(
#ifdef  USE_UDISK_PLAY
			&gFatfs_u,
#else
			&gFatfs_sd,
#endif
			NULL, NULL,
			acc_ram_blk);
	f_scan_vol(current_vol);


	AudioDAC_Init(ALL, 48000, dac0_dma_buffer, sizeof(dac0_dma_buffer), dac1_dma_buffer, sizeof(dac1_dma_buffer));

	for(i = 1; i <= f_file_real_quantity(); i++)
	//for(i = 199; i <= f_file_real_quantity(); i++)
	{
		FileType fileType;
		DBG("i;%d\n", (int)i);
		memset(&gFil, 0x00, sizeof(gFil));
		memset(file_long_name, 0x00, sizeof(file_long_name));
		if(FR_OK != f_open_by_num(current_vol, &gDirs, &gFil, i, file_long_name))
		{
			DBG("Open file error!\n");
			continue;
		}
		fileType = get_audio_type((TCHAR *)file_long_name);

		DBG("Song Name: %s\n", gFil.fn);
		DBG("Song Name long: %s\n", file_long_name);
		DBG("Song type: %d\n", fileType);

		if(RT_SUCCESS == audio_decoder_initialize(decoder_buf, &gFil, IO_TYPE_FILE, fileType))
		{
			DBG("decoder init OK! size=%d\n", (int)audio_decoder_get_context_size());
			//AudioDAC_Switch_SampleRate(AUDIO_DAC0, audio_decoder->song_info->sampling_rate);
			//AudioDAC_Switch_SampleRate(AUDIO_DAC1, audio_decoder->song_info->sampling_rate);
			AudioDAC_SampleRateChange(ALL, audio_decoder->song_info->sampling_rate);
			SampleRateCC = audio_decoder->song_info->sampling_rate;
			play_samples = 0;

			DBG("[SONG_INFO]: ChannelCnt : %6d\n",		  (unsigned int)audio_decoder->song_info->num_channels);
			DBG("[SONG_INFO]: SampleRate : %6d Hz\n",	  (unsigned int)audio_decoder->song_info->sampling_rate);
			DBG("[SONG_INFO]: BitRate	 : %6d Kbps\n",   (unsigned int)audio_decoder->song_info->bitrate / 1000);
			DBG("[SONG_INFO]: DecoderSize: %6d Bytes \n", (unsigned int)audio_decoder->decoder_size);
			DBG("[SONG_INFO]: Duration	 : %02u:%02u:%02u\n",	  (unsigned int)(audio_decoder->song_info->duration / 1000) / 3600, (unsigned int)((audio_decoder->song_info->duration / 1000) % 3600) / 60, (unsigned int)((audio_decoder->song_info->duration / 1000) % 3600) % 60);
			DBG("[SONG_INFO]: IsVBR 	 : %s\n",		  audio_decoder->song_info->vbr_flag ? "   YES" : "    NO");

			DBG("[SONG_INFO]: CHARSET	 : %6d\n", (int)audio_decoder->song_info->char_set);
			DBG("[SONG_INFO]: TITLE 	 : %s\n", audio_decoder->song_info->title);
			DBG("[SONG_INFO]: ARTIST	 : %s\n", audio_decoder->song_info->artist);
			DBG("[SONG_INFO]: ALBUM 	 : %s\n", audio_decoder->song_info->album);
			DBG("[SONG_INFO]: COMMT 	 : %s\n", audio_decoder->song_info->comment);
			DBG("[SONG_INFO]: GENRE 	 : %s\n", audio_decoder->song_info->genre_str);
			DBG("[SONG_INFO]: YEAR		 : %s\n", audio_decoder->song_info->year);
			DBG("[SONG_INFO]: TRACK 	 : %6d\n", audio_decoder->song_info->track);
			DBG("\n");
			AudioDAC0_ResetDMA();
			AudioDAC1_ResetDMA();
			AudioDAC_VolSet(AUDIO_DAC0, vol_l, vol_r);
			while(RT_YES == audio_decoder_can_continue())
			{
				uint8_t c;
				if(RT_SUCCESS == audio_decoder_decode())
				{
					static uint32_t play_sec, last_play_sec = 0;
					if(SampleRateCC != audio_decoder->song_info->sampling_rate)
					{
						DBG("[INFO]: Change sample rate from %u to %u Hz\n", (unsigned int)SampleRateCC, (unsigned int)audio_decoder->song_info->sampling_rate);
						SampleRateCC = audio_decoder->song_info->sampling_rate;
						//AudioDAC_Switch_SampleRate(AUDIO_DAC0, audio_decoder->song_info->sampling_rate);
						//AudioDAC_Switch_SampleRate(AUDIO_DAC1, audio_decoder->song_info->sampling_rate);
						AudioDAC_SampleRateChange(ALL, audio_decoder->song_info->sampling_rate);
					}

					while(AudioDAC0DataSpaceLenGet() < audio_decoder->song_info->pcm_data_length);
					if(audio_decoder->song_info->num_channels == 1)
					{
						uint16_t i, *one_sample = (uint16_t*)audio_decoder->song_info->pcm_addr;
						for(i = 0; i < audio_decoder->song_info->pcm_data_length; i++)
						{
							uint16_t osm[2];
							osm[0] = *one_sample; osm[1] = *one_sample;
							AudioDAC0DataSet(osm, 1);
							one_sample ++;
						}
						AudioDAC1DataSet(audio_decoder->song_info->pcm_addr, audio_decoder->song_info->pcm_data_length);
					}
					else
					{
						uint16_t i, *one_sample = (uint16_t*)audio_decoder->song_info->pcm_addr;
						AudioDAC0DataSet(audio_decoder->song_info->pcm_addr, audio_decoder->song_info->pcm_data_length);
						for(i = 0; i < audio_decoder->song_info->pcm_data_length; i++)
						{
							//pcmdata = bfloor(*one_sample + *(one_sample+1), 1, 15);
							AudioDAC1DataSet(one_sample, 1);
							one_sample +=2;
						}

					}
					play_samples += audio_decoder->song_info->pcm_data_length;
					play_sec = play_samples / audio_decoder->song_info->sampling_rate;
					//DBG(".");
					if((play_sec % 3600) % 60 != last_play_sec)
					{
						DBG("\rplay time: %02u:%02u:%02u", (unsigned int)play_sec / 3600, (unsigned int)(play_sec % 3600) / 60, (unsigned int)(play_sec % 3600) % 60);
						last_play_sec = (play_sec % 3600) % 60;
					}
				}
				else
				{
					DBG("[INFO]: ERROR%d\n", (int)audio_decoder->error_code);
				}

				if(UART1_RecvByte(&c))
				{
					if(c == 'p')
					{
						DBG("\nNext...\n");
						break;
					}

					if(c == 'k')
					{
						if(vol_l > 10)
						{
							vol_l -= 10;vol_r -= 10;
							AudioDAC_VolSet(AUDIO_DAC0, vol_l, vol_r);
							DBG("vol-: %d\n", vol_l);
						}
						else
						{
							vol_l = 0;vol_r = 0;
							AudioDAC_VolSet(AUDIO_DAC0, vol_l, vol_r);
							DBG("vol-: %d\n", vol_l);
						}
					}

					if(c == 'l')
					{
						if(vol_l < 0xaff)
						{
							vol_l += 10;vol_r += 10;
							AudioDAC_VolSet(AUDIO_DAC0, vol_l, vol_r);
							DBG("vol+: %d\n", vol_l);
						}
					}
				}
			}
		}
		else
		{
			DBG("decoder init failed!!! %d\n", (int)audio_decoder_get_error_code());
		}

		f_close(&gFil);
	}
	DBG("Play finished!!!\n");
ERROR:
	while(1);
}
