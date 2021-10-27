/**
 **************************************************************************************
 * @file    demo_RemindSound.c
 * @brief   Remind Sound example
 *
 * @author  pi
 * @version V1.0.0
 *
 * $Created: 2018-04-01 19:17:00$
 *
 * @Copyright (C) 2018, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include "type.h"
#include "chip_info.h"
#include <stdlib.h>
#include <string.h>
#include "clk.h"
#include "irqn.h"
#include "dma.h"
#include "spi_flash.h"
#include "gpio.h"
#include "dac.h"
#include "uarts.h"
#include "remap.h"
#include "sys.h"
#include "watchdog.h"
#include "dac_interface.h"
#include "sw_uart.h"
#include "audio_decoder_api.h"
#include "debug.h"
#include "audio_utility.h"
#include "mvstdio.h"
#include "remind_sound_item.h"

/*******************************************Action & Data Stream*****************************************************
*
*Init & Flash Crc Check
*Scan SoundTable Item, if item exist in flash : play
*                                                                    |-> dac0_dma_buffer -DMA4-> DAC0
* FlashConstData -CallbackRead-> Decode-> @decoder_buf -pcm_addr&Set-+
*                                                                    |-> dac1_dma_buffer -DMA5-> DAC1
*
*********************************************************************************************************************
*/


uint8_t SDIO_DmaChannelMap[29] =
{
	255,//PERIPHERAL_ID_SPIS_RX = 0,		//0
	255,//PERIPHERAL_ID_SPIS_TX,			//1
	255,//PERIPHERAL_ID_TIMER3,			//2
	255,//PERIPHERAL_ID_SDIO_RX,			//3
	255,//PERIPHERAL_ID_SDIO_TX,			//4
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

//#define FILE_TYPE_SBC //sbc类型 解码器buf占用小，但编码大三倍。

#pragma pack(1)
typedef struct _SongClipsHdr
{
	char sync[4];
	uint32_t crc;
	uint8_t cnt;
} SongClipsHdr;
#pragma pack()

#pragma pack(1)
typedef struct _SongClipsEntry
{
	uint8_t id[8];
	uint32_t offset;
	uint32_t size;
} SongClipsEntry;
#pragma pack()

#define FLASH_DATA_BASE_ADDR 		(0x00100000)
#define REMIND_FLASH_STORE_OVERFLOW	(0x001D0000)
#define REMIND_FLASH_STORE_BASE 	(FLASH_DATA_BASE_ADDR)
#define REMIND_FLASH_HDR_SIZE 		(0x00001000)
#define REMIND_CLIPS_DATA_ADDR 		(REMIND_FLASH_STORE_BASE + REMIND_FLASH_HDR_SIZE)
#define REMIND_CLIPS_ENTRY_MAX 		(REMIND_FLASH_HDR_SIZE/sizeof(SongClipsEntry))
#define REMIND_FLASH_ADDR(n) 		(REMIND_FLASH_STORE_BASE + sizeof(SongClipsEntry) * n + sizeof(SongClipsHdr))//flash提示音区配置决定
#define	REMIND_SOUND_ID_LEN			sizeof(((SongClipsEntry *)0)->id)
#define REMIND_FLASH_READ_TIMEOUT 	10 		//ms
static uint8_t File_buf[REMIND_FLASH_HDR_SIZE]; //使用提示音数据头区域大小，方便crc，实际可以缩小，
uint32_t ConstDataAddr,ConstDataSize;
uint32_t ConstDataOffset;

static uint32_t play_samples;
static uint32_t SampleRateCC = 44100;

static uint8_t dac0_dma_buffer[2304 * 4];//目前MP3解码帧sample：576，此buf可缩至 576*2*4。
static uint8_t dac1_dma_buffer[2304 * 2];
#ifdef FILE_TYPE_SBC
#define DECODER_BUF_SIZE		(6 * 1024)
#define FILE_TYPE				SBC_DECODER
#else
#define DECODER_BUF_SIZE		(19 * 1024)
#define FILE_TYPE				MP3_DECODER
#endif

static uint8_t decoder_buf[DECODER_BUF_SIZE];


static const unsigned short CrcCCITTTable[256] =
{
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
	0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
	0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
	0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
	0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
	0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
	0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
	0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
	0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
	0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
	0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
	0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
	0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
	0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
	0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
	0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
	0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
	0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
	0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
	0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
	0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
	0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
	0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};




MemHandle sound_clip_mh;//解码器接口类型参数，实际启用Callback方式后参数未起作用
int32_t error_cnt = 0; 	//连续解码出错次数,
#define ERROR_MAX			3 //容错率设定


//缓存数据填充（decoder 解帧数据不足时调用），解码器终止判断依据
uint32_t RemindSoundFillStreamCallback(void *buffer, uint32_t length)
{
	int32_t RemainBytes = ConstDataSize - ConstDataOffset;
	int32_t ReadBytes   = length > RemainBytes ? RemainBytes : length;

	if(ReadBytes == 0)
	{
		error_cnt = ERROR_MAX;
		return 0;	//
	}

	//APP_DBG("SoundRemind Fill Stream Callback...\n");
	if(SpiFlashRead(FLASH_DATA_BASE_ADDR + ConstDataAddr + ConstDataOffset,
					buffer,
					ReadBytes,
					REMIND_FLASH_READ_TIMEOUT) != FLASH_NONE_ERR)
	{
		ReadBytes = 0;
		DBG("read const data error!\r\n");
	}
	DBG("%d@%d\n", (int)ReadBytes, (int)RemainBytes);
	ConstDataOffset += ReadBytes;

	return ReadBytes;
}


unsigned short CRC16(unsigned char *Buf, unsigned int BufLen, unsigned short CRC)
{
	unsigned int i;
	for(i = 0 ; i < BufLen; i++)
	{
		CRC = (CRC << 8) ^ CrcCCITTTable[((CRC >> 8) ^ *Buf++) & 0x00FF];
	}
	return CRC;
}

bool sound_clips_all_crc(void)
{
	SongClipsHdr *hdr;
	SongClipsEntry *ptr;
	uint16_t crc=0, i, j, CrcRead;
	uint32_t FlashAddr, all_len = 0;
	uint8_t *data_ptr = NULL;
	bool ret = TRUE;
	FlashAddr = REMIND_FLASH_STORE_BASE;

	data_ptr = File_buf;//借用文件缓冲buf
	if(SpiFlashRead(FlashAddr,
					data_ptr,
					REMIND_FLASH_HDR_SIZE,
					REMIND_FLASH_READ_TIMEOUT) != FLASH_NONE_ERR)
	{
		DBG("read const data error!\r\n");
		ret = FALSE;
	}
	else
	{
		ptr = (SongClipsEntry*)(data_ptr + sizeof(SongClipsHdr));
		hdr = (SongClipsHdr *)(data_ptr);
		if(strncmp(hdr->sync, "MVUB", 4))
		{
			DBG("sync not found\n");
			ret = FALSE;
		}
		else
		{
			for(i = 0; i < hdr->cnt; i++)
			{
				all_len += ptr[i].size;
				for(j = 0; j < REMIND_SOUND_ID_LEN; j++)
				{
					DBG("%c", ((uint8_t *)&ptr[i].id)[j]);
				}
				DBG("/");
			}
			DBG("\nALL clips size = %d\n", (int)all_len);
			if(REMIND_FLASH_STORE_BASE + REMIND_FLASH_HDR_SIZE + all_len > REMIND_FLASH_STORE_OVERFLOW)
			{
				DBG("Remind flash const data overflow.\n");
				ret = FALSE;
			}
			CrcRead = hdr->crc;
			crc = CRC16(data_ptr, 4, crc);
			crc = CRC16(data_ptr + 8, REMIND_FLASH_HDR_SIZE - 8, crc);
			FlashAddr += REMIND_FLASH_HDR_SIZE;
			while(all_len && ret)
			{
				if(all_len > REMIND_FLASH_HDR_SIZE)
				{
					i = REMIND_FLASH_HDR_SIZE;
				}
				else
				{
					i = all_len;
				}
				if(SpiFlashRead(FlashAddr,
								data_ptr,
								i,
								REMIND_FLASH_READ_TIMEOUT) != FLASH_NONE_ERR)
				{
					DBG("read const data error!\r\n");
					ret = FALSE;
				}
				else
				{
					crc = CRC16(data_ptr, i, crc);
					FlashAddr += i;
					all_len -= i;
				}
			}
			DBG("crc = %04X|%04X\n", crc, CrcRead);
		}
	}
	return ret;
}


/**
 * @func        drag_sound_clips_info
 * @brief
 * @param       uint8_t *         	预定义的文件ID/8字符

 * @Output      None
 * @return      bool
 * @Others
 */
bool drag_sound_clips_info(uint8_t *NameStr)
{
	SongClipsEntry SongClips;
	int i;

	for(i = 0; i < SOUND_REMIND_TOTAL; i++)
	{
		if(FLASH_NONE_ERR != SpiFlashRead(REMIND_FLASH_ADDR(i), (uint8_t *)&SongClips, sizeof(SongClipsEntry), REMIND_FLASH_READ_TIMEOUT))
		{
			return FALSE;
		}
		if(memcmp(SongClips.id, NameStr, REMIND_SOUND_ID_LEN) == 0)
		{
			ConstDataOffset = 0;//注意重入问题
			ConstDataAddr = SongClips.offset;//for temp
			ConstDataSize = SongClips.size;
			DBG("GET clip In flash: id = %c%c%c%c%c%c%c%c, i = %d, addr = 0x%X, Size = %d\n", ((char*)(&SongClips.id))[0], ((char*)(&SongClips.id))[1],
				((char*)(&SongClips.id))[2], ((char*)(&SongClips.id))[3], ((char*)(&SongClips.id))[4], ((char*)(&SongClips.id))[5],
				((char*)(&SongClips.id))[6], ((char*)(&SongClips.id))[7], i, (unsigned int)SongClips.offset, (int)SongClips.size);
			mv_mread_callback_set(RemindSoundFillStreamCallback);//解码器读数据API重载
			return TRUE;
		}
	}
	return FALSE;
}


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

bool PlayRemindItem(uint8_t * NameStr)
{
	error_cnt = 0;
	if(drag_sound_clips_info(NameStr))
	{

		if(RT_SUCCESS == audio_decoder_initialize((uint8_t*)decoder_buf, (void*)&sound_clip_mh, IO_TYPE_MEMORY, FILE_TYPE))
		{

			DBG("decoder init OK! size=%d\n", (int)audio_decoder_get_context_size());
			AudioDAC_SampleRateChange(ALL, audio_decoder->song_info->sampling_rate);
			SampleRateCC = audio_decoder->song_info->sampling_rate;
			play_samples = 0;

			DBG("[SONG_INFO]: ChannelCnt : %6d\n",		  (int)audio_decoder->song_info->num_channels);
			DBG("[SONG_INFO]: SampleRate : %6d Hz\n",	  (int)audio_decoder->song_info->sampling_rate);
			DBG("[SONG_INFO]: BitRate	 : %6d Kbps\n",   (int)audio_decoder->song_info->bitrate / 1000);
			if(audio_decoder->song_info->duration) //有些格式能提供时长。
			{
				DBG("[SONG_INFO]: Duration	 : %d mS\n",	  (int)audio_decoder->song_info->duration);
			}
			if(audio_decoder->song_info->vbr_flag)
			{
				DBG("[SONG_INFO]: IsVBR 	 :    YES\n");
			}
			DBG("\n");
			AudioDAC0_ResetDMA();
			AudioDAC1_ResetDMA();

			while(RT_YES == audio_decoder_can_continue())
			{
				if(RT_SUCCESS == audio_decoder_decode())
				{
					if(SampleRateCC != audio_decoder->song_info->sampling_rate)//for vbr
					{
						DBG("[INFO]: Change sample rate from %d to %d Hz\n", (int)SampleRateCC, (int)audio_decoder->song_info->sampling_rate);
						SampleRateCC = audio_decoder->song_info->sampling_rate;
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
							AudioDAC1DataSet(one_sample, 1);
							one_sample +=2;
						}

					}
					play_samples += audio_decoder->song_info->pcm_data_length;
				}
				else
				{

					if(audio_decoder->error_code == -127 || audio_decoder->error_code == -123)
					{
						if(error_cnt ++ >= 3)
						{
							mv_mread_callback_unset(); //撤销解码器读数据API重载
							break;
						}
						else
						{
							DBG("[INFO]: ERROR%d\n", (int)audio_decoder->error_code);
						}
					}
					else
					{
						error_cnt = 0;
						DBG("[INFO]: ERROR%d\n", (int)audio_decoder->error_code);

					}
				}
			}
			DBG("Finished playing, next...\n");
			return TRUE;
		}
		else
		{
			DBG("decoder init failed!!! %d\n", (int)audio_decoder_get_error_code());

		}
	}
	return FALSE;
}

void EnableSwUartAsFuart(unsigned char EnableFlag);//retarget.c for BP10开发板模拟串口GPIO A1
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

	DBG("\n");
	DBG("/-----------------------------------------------------\\\n");
	DBG("|                   RemindSound Example                   |\n");
	DBG("|      Mountain View Silicon Technology Co.,Ltd.      |\n");
	DBG("\\-----------------------------------------------------/\n");
	DBG("\n");

	DMA_ChannelAllocTableSet(SDIO_DmaChannelMap);
	SysTickInit();
	GIE_ENABLE();	//开启总中断

	AudioDAC_Init(ALL, SampleRateCC, dac0_dma_buffer, sizeof(dac0_dma_buffer), dac1_dma_buffer, sizeof(dac1_dma_buffer));

	sound_clips_all_crc();
	//启用Callback后 实际 此参数未起作用，只做必要的初始化，为解码器Api提供参数
	sound_clip_mh.addr = NULL;
	sound_clip_mh.mem_capacity = 0;
	sound_clip_mh.mem_len = 0;
	sound_clip_mh.p = 0;

	PlayRemindItem((uint8_t *)SOUND_REMIND_ZHANGSHE);
	PlayRemindItem((uint8_t *)SOUND_REMIND_DLGUODI);
	DBG("Finished all sound clips...\n");
	while(1);
}
