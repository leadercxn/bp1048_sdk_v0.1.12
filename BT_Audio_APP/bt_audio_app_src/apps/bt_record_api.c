/**
 **************************************************************************************
 * @file    bt_record_api.c
 * @brief   蓝牙K歌录音模式(使用通话链路)
 *
 * @author  KK
 * @version V1.0.0
 *
 * $Created: 2019-8-18 18:00:00$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <string.h>
#include "type.h"
#include "app_config.h"
#include "app_message.h"
#include "mvintrinsics.h"
//driver
#include "chip_info.h"
#include "dac.h"
#include "gpio.h"
#include "dma.h"
#include "dac.h"
#include "audio_adc.h"
#include "debug.h"
//middleware
#include "main_task.h"
#include "audio_vol.h"
#include "rtos_api.h"
#include "adc_interface.h"
#include "dac_interface.h"
#include "bt_manager.h"
#include "bt_config.h"
#include "resampler.h"
#include "mcu_circular_buf.h"
#include "audio_core_api.h"
#include "audio_decoder_api.h"
#include "sbcenc_api.h"
#include "cvsd_plc.h"
#include "ctrlvars.h"
#include "bt_hfp_api.h"
//application
#include "bt_record_mode.h"
#include "bt_record_api.h"
#include "decoder_service.h"
#include "audio_core_service.h"
#include "mode_switch_api.h"
#include "audio_core_api.h"

#ifdef CFG_FUNC_REMIND_SOUND_EN
#include "remind_sound_service.h"
#endif
#include "powercontroller.h"

extern void ResumeAudioCoreMicSource(void);

#if (defined(CFG_APP_BT_MODE_EN) && defined(BT_RECORD_FUNC_ENABLE))

extern uint8_t lc_sco_data_error_flag; //0=CORRECTLY_RX_FLAG; 1=POSSIBLY_INVALID_FLAG; 2=NO_RX_DATA_FLAG; 3=PARTIALLY_LOST_FLAG;

//msbc encoder
#define MSBC_CHANNE_MODE	1 		// mono
#define MSBC_SAMPLE_REATE	16000	// 16kHz
#define MSBC_BLOCK_LENGTH	15

// sco sync header H2
uint8_t sco_sync_header1[4][2] = 
{
	{0x01, 0x08}, 
	{0x01, 0x38}, 
	{0x01, 0xc8}, 
	{0x01, 0xf8}
};

/*******************************************************************************
 * AudioCore Sink Channel2 :输出为需要发送的HF数据(转采样前端缓存FIFO)
 * 44.1K -> 16K
 * unit: sample
 ******************************************************************************/
uint16_t BtRecord_SinkScoDataSet(void* InBuf, uint16_t InLen)
{
	if(InLen == 0)
		return 0;

	osMutexLock(BtRecordCt->ResamplerPreFifoMutex);
	MCUCircular_PutData(&BtRecordCt->ResamplerPreFifoCircular, InBuf, InLen * 2);
	osMutexUnlock(BtRecordCt->ResamplerPreFifoMutex);

	return InLen;
}

void BtRecord_SinkScoDataGet(void* OutBuf, uint16_t OutLen)
{
	if(OutLen == 0)
		return;

	osMutexLock(BtRecordCt->ResamplerPreFifoMutex);
	MCUCircular_GetData(&BtRecordCt->ResamplerPreFifoCircular, OutBuf, OutLen*2);
	osMutexUnlock(BtRecordCt->ResamplerPreFifoMutex);
}


uint16_t BtRecord_SinkScoDataSpaceLenGet(void)
{
	return MCUCircular_GetSpaceLen(&BtRecordCt->ResamplerPreFifoCircular) / 2;
}

uint16_t BtRecord_SinkScoDataLenGet(void)
{
	return MCUCircular_GetDataLen(&BtRecordCt->ResamplerPreFifoCircular) / 2;
}

/*******************************************************************************
 * AudioCore Sink Channel2 :输出为需要发送的HF数据(转采样之后16k)
 * unit: sample
 ******************************************************************************/
uint16_t BtRecord_ResampleOutDataSet(void* InBuf, uint16_t InLen)
{
	if(InLen == 0)
		return 0;

	//osMutexLock(BtRecordCt->ResamplerPreFifoMutex);
	MCUCircular_PutData(&BtRecordCt->ResampleOutFifoCircular, InBuf, InLen * 2);
	//osMutexUnlock(BtRecordCt->ResamplerPreFifoMutex);

	return InLen;
}

void BtRecord_ResampleOutDataGet(void* OutBuf, uint16_t OutLen)
{
	if(OutLen == 0)
		return;

	//osMutexLock(BtRecordCt->ResamplerPreFifoMutex);
	MCUCircular_GetData(&BtRecordCt->ResampleOutFifoCircular, OutBuf, OutLen*2);
	//osMutexUnlock(BtRecordCt->ResamplerPreFifoMutex);
}


uint16_t BtRecord_ResampleOutDataSpaceLenGet(void)
{
	return MCUCircular_GetSpaceLen(&BtRecordCt->ResampleOutFifoCircular) / 2;
}

uint16_t BtRecord_ResampleOutDataLenGet(void)
{
	return MCUCircular_GetDataLen(&BtRecordCt->ResampleOutFifoCircular) / 2;
}

/*******************************************************************************
 * MIC处理后，缓存到待发送缓存FIFO
 * unit: Bytes
 ******************************************************************************/
uint16_t BtRecord_SendScoBufSet(void* InBuf, uint16_t InLen)
{
	if(InLen == 0)
		return 0;

	MCUCircular_PutData(&BtRecordCt->BtEncoderSendCircularBuf, InBuf, InLen);
	return InLen;
}

void BtRecord_SendScoBufGet(void* OutBuf, uint16_t OutLen)
{
	if(OutLen == 0)
		return;

	MCUCircular_GetData(&BtRecordCt->BtEncoderSendCircularBuf, OutBuf, OutLen);
}


uint16_t BtRecord_SendScoBufSpaceLenGet(void)
{
	return MCUCircular_GetSpaceLen(&BtRecordCt->BtEncoderSendCircularBuf);
}

uint16_t BtRecord_SendScoBufLenGet(void)
{
	return MCUCircular_GetDataLen(&BtRecordCt->BtEncoderSendCircularBuf);
}

/***********************************************************************************
 * msbc Encoder
 **********************************************************************************/
void BtRecord_MsbcEncoderInit(void)
{
	//encoder init
	int32_t samplesPerFrame;
	int32_t ret;

	BtRecordCt->sbc_encode_ct = (SBCEncoderContext*)osPortMalloc(sizeof(SBCEncoderContext));
	if(BtRecordCt->sbc_encode_ct == NULL)
	{
		APP_DBG("sbc encode init malloc error\n");
	}
	memset(BtRecordCt->sbc_encode_ct, 0, sizeof(SBCEncoderContext));

	if(BtRecordCt->codecType == HFP_AUDIO_DATA_PCM)
		return;
	
	ret = sbc_encoder_initialize(BtRecordCt->sbc_encode_ct, MSBC_CHANNE_MODE, MSBC_SAMPLE_REATE, MSBC_BLOCK_LENGTH, SBC_ENC_QUALITY_MIDDLE, &samplesPerFrame);
	APP_DBG("encoder sample:%ld\n", samplesPerFrame);
	if(ret != SBC_ENC_ERROR_OK)
	{
		APP_DBG("sbc encode init error\n");
		return;
	}
	BtRecordCt->mSbcEncoderStart = 1;
}

void BtRecord_MsbcEncoderDeinit(void)
{
	if(BtRecordCt->sbc_encode_ct)
	{
		osPortFree(BtRecordCt->sbc_encode_ct);
		BtRecordCt->sbc_encode_ct = NULL;
	}
	
	BtRecordCt->mSbcEncoderStart = 0;
}

/*******************************************************************************
 * MSBC Decoder
 ******************************************************************************/
void BtRecord_MsbcMemoryReset(void)
{
	if(BtRecordCt->msbcRingBuf == NULL)
		return;
	memset(BtRecordCt->msbcRingBuf, 0, MSBC_DECODER_INPUT_SIZE);
	
	BtRecordCt->msbcMemHandle.addr = BtRecordCt->msbcRingBuf;
	BtRecordCt->msbcMemHandle.mem_capacity = MSBC_DECODER_INPUT_SIZE;
	BtRecordCt->msbcMemHandle.mem_len = 0;
	BtRecordCt->msbcMemHandle.p = 0;
}

int32_t BtRecord_MsbcDecoderInit(void)
{
	BtRecordCt->msbcRingBuf = osPortMalloc(MSBC_DECODER_INPUT_SIZE);
	if(BtRecordCt->msbcRingBuf == NULL)
		return -1;
	memset(BtRecordCt->msbcRingBuf, 0, MSBC_DECODER_INPUT_SIZE);
	
	BtRecordCt->msbcMemHandle.addr = BtRecordCt->msbcRingBuf;
	BtRecordCt->msbcMemHandle.mem_capacity = MSBC_DECODER_INPUT_SIZE;
	BtRecordCt->msbcMemHandle.mem_len = 0;
	BtRecordCt->msbcMemHandle.p = 0;
	
	BtRecordCt->msbcDecoderInitFlag = TRUE;
	BtRecord_MsbcDecoderStartedSet(FALSE);

	return 0;
}

int32_t BtRecord_MsbcDecoderDeinit(void)
{
	BtRecordCt->msbcMemHandle.addr = NULL;
	BtRecordCt->msbcMemHandle.mem_capacity = 0;
	BtRecordCt->msbcMemHandle.mem_len = 0;
	BtRecordCt->msbcMemHandle.p = 0;
	
	BtRecordCt->msbcDecoderInitFlag = FALSE;
	BtRecord_MsbcDecoderStartedSet(FALSE);

	if(BtRecordCt->msbcRingBuf)
	{
		osPortFree(BtRecordCt->msbcRingBuf);
		BtRecordCt->msbcRingBuf = NULL;
	}
	return 0;
}

void BtRecord_MsbcDecoderStartedSet(bool flag)
{
	BtRecordCt->msbcDecoderStarted = flag;
}

bool BtRecord_MsbcDecoderStartedGet(void)
{
	return BtRecordCt->msbcDecoderStarted;
}

bool BtRecord_MsbcDecoderIsInitialized(void)
{
	return BtRecordCt->msbcDecoderInitFlag;
}

static MemHandle * BtRecord_MsbcDecoderMemHandleGet(void)
{
	if(BtRecord_MsbcDecoderIsInitialized())
	{
		return &BtRecordCt->msbcMemHandle;
	}
	return NULL;
}

uint32_t BtRecord_MsbcDataLenGet(void)
{
	uint32_t	dataSize = 0;
	if(BtRecordCt->msbcDecoderInitFlag)
	{
		dataSize = mv_msize(&BtRecordCt->msbcMemHandle);
	}
	return dataSize;
}

int32_t BtRecord_MsbcDecoderStart(void)
{
	int32_t 		ret = 0;
	
	ret = DecoderInit(BtRecord_MsbcDecoderMemHandleGet(), (int32_t)IO_TYPE_MEMORY, MSBC_DECODER);
	if(ret != RT_SUCCESS)
	{
		APP_DBG("audio_decoder_initialize error code:%ld!\n", audio_decoder_get_error_code());
		BtRecord_MsbcDecoderStartedSet(FALSE);
		return -1;
	}
	DecoderPlay();
	audio_decoder->song_info = audio_decoder_get_song_info();
//	APP_DBG("[INFO]: sample rate from %u Hz, Channel:%d, type:%d\n", (unsigned int)audio_decoder->song_info->sampling_rate, audio_decoder->song_info->num_channels, audio_decoder->song_info->stream_type);

	APP_DBG("Decoder Service Start...\n");

	BtRecord_MsbcDecoderStartedSet(TRUE);
	
#ifdef CFG_FUNC_REMIND_SOUND_EN
	SoftFlagDeregister(SoftFlagDecoderMask & ~SoftFlagDecoderApp);//安全性清理
#endif	
	return 0;
}

/*******************************************************************************
 * hf volume sync
 ******************************************************************************/
void SetBtRecordSyncVolume(uint8_t gain)
{
	BT_MANAGER_ST *	tempBtManager = NULL;
	uint32_t volume = gain;
	
	tempBtManager = GetBtManager();

	if(tempBtManager == NULL)
		return;

	tempBtManager->volGain = (uint8_t)((volume*15)/CFG_PARA_MAX_VOLUME_NUM);
	APP_DBG("hf volume:%d\n", tempBtManager->volGain);
	HfpSpeakerVolume(tempBtManager->volGain);
}

uint8_t GetBtRecordSyncVolume(void)
{
	return BtRecordCt->BtSyncVolume;
}


/***********************************************************************************
 * get sco data(CVSD)
 **********************************************************************************/
//Len/Length都是Samples(单声道)
uint16_t BtRecord_ScoDataGet(void *OutBuf,uint16_t OutLen)
{
	uint16_t NumSamples = 0;
	NumSamples = MCUCircular_GetData(&BtRecordCt->BtScoCircularBuf,OutBuf,OutLen*2);
	return NumSamples/2;
}

void BtRecord_ScoDataSet(void *InBuf,uint16_t InLen)
{
	MCUCircular_PutData(&BtRecordCt->BtScoCircularBuf,InBuf,InLen*2);
}

uint16_t BtRecord_ScoDataLenGet(void)
{
	uint16_t NumSamples = 0;
	NumSamples = MCUCircular_GetDataLen(&BtRecordCt->BtScoCircularBuf);
	return NumSamples/2;
}

uint16_t BtRecord_ScoDataSpaceLenGet(void)
{
	uint16_t NumSamples = 0;
	NumSamples = MCUCircular_GetSpaceLen(&BtRecordCt->BtScoCircularBuf);
	return NumSamples/2;
}

/***********************************************************************************
 * 接收到HFP SCO数据,插入缓存fifo
 **********************************************************************************/
int16_t BtRecord_SaveScoData(uint8_t* data, uint16_t len)
{
	//uint16_t i;
	
	uint32_t	insertLen = 0;
	int32_t 	remainLen = 0;
	uint16_t	msbcLen = 0;
	int32_t SRCDoneLen;	//sample number

	if((BtRecordCt->ModeKillFlag)||(BtRecordCt->state != TaskStateRunning)||(!BtRecordCt->hfModeAudioResInitOk))
		return -1;

	if((DecoderSourceNumGet()!=BT_RECORD_SOURCE_NUM)
#ifdef CFG_FUNC_REMIND_SOUND_EN
		|| SoftFlagGet(SoftFlagDecoderRemind)
#endif
		)
		return -1;

	//开启输出
	if(!BtRecordCt->AudioCoreBtHf->AudioSource[BT_RECORD_SOURCE_NUM].Enable)
		BtRecordCt->AudioCoreBtHf->AudioSource[BT_RECORD_SOURCE_NUM].Enable = 1;

	BtRecordCt->ScoInputLen = len;

	if(BtRecordCt->codecType == HFP_AUDIO_DATA_PCM)
	{
		//cvsd - plc
		if(lc_sco_data_error_flag)
		{
			//APP_DBG("sco_error:%d\n", lc_sco_data_error_flag);
			lc_sco_data_error_flag=0;
			cvsd_plc_bad_frame(BtRecordCt->cvsdPlcState, (int16_t *)BtRecordCt->ScoInputFifo);
		}
		else
		{
			cvsd_plc_good_frame(BtRecordCt->cvsdPlcState, (int16_t *)data, (int16_t *)BtRecordCt->ScoInputFifo);
		}
		
		BtRecordCt->ScoInputLen = len;
		
		//CVSD : 转采样 8k->16k mono
		//在此进行转采样,会带来300-400us的任务占用时间
		SRCDoneLen = resampler_polyphase_apply(BtRecordCt->ScoInResamplerCt, (int16_t*)BtRecordCt->ScoInputFifo, (int16_t*)BtRecordCt->ScoInSrcOutBuf, BtRecordCt->ScoInputLen/2);
		if(SRCDoneLen>0)
		{
			if(BtRecord_ScoDataSpaceLenGet() > SRCDoneLen)
				BtRecord_ScoDataSet(BtRecordCt->ScoInSrcOutBuf, SRCDoneLen);
			else
			{
				APP_DBG("f");
				//当fifo满了,AudioCore后端阻塞(Source disable),则丢弃fifo前面缓存的数据
				if(AudioCore.AudioSink[AUDIO_HF_SCO_SINK_NUM].Enable == DISABLE)
				{
					int16_t d[BT_CVSD_PACKET_LEN*2];
					BtRecord_ScoDataGet(d, SRCDoneLen);//discard
					BtRecord_ScoDataSet(BtRecordCt->ScoInSrcOutBuf, SRCDoneLen);
					APP_DBG("-");
				}
			}
		}

		//send data(120Bytes)
		if(BtRecord_SendScoBufLenGet() > BT_CVSD_PACKET_LEN)
		{
			memset(BtRecordCt->scoSendBuf,0,BT_CVSD_PACKET_LEN);
			BtRecord_SendScoBufGet(BtRecordCt->scoSendBuf, BT_CVSD_PACKET_LEN);
			HfpSendScoData(BtRecordCt->scoSendBuf, BT_CVSD_PACKET_LEN);
		}
	}
	else
	{
		//接收到的数据需要做如下判断
		//1.全0 丢弃
		//2.长度非60的，需要放入缓存
		//3.长度为60的直接处理
		if((!lc_sco_data_error_flag)&&(!BtRecord_MsbcDecoderStartedGet()))
		{
			//为0数据丢弃
			if((data[0]==0)&&(data[1]==0)&&(data[2]==0)&&(data[3]==0)&&(data[len-2]==0)&&(data[len-1]==0))
				return -1;
		}

		//长度
		if(BtRecordCt->ScoInputLen != BT_MSBC_PACKET_LEN)
		{
			//if((data[0] != 0x01)||(data[2] != 0xad)||(data[3] != 0x00))
			{
				APP_DBG("msbc discard...\n");
				return -1;
			}
		}
		
		//数据格式: 0-1:sync header;  2-58:msbc data(57bytes - 有效数据); 59:tail
		memcpy(BtRecordCt->ScoInputFifo, &data[2], BtRecordCt->ScoInputLen-3);
		
		//接收到错误的msbc数据包,则将该次收到的数据全部清空为0
		if(lc_sco_data_error_flag)
		{
			//APP_DBG("msbc e:%d\n",lc_sco_data_error_flag);
			lc_sco_data_error_flag=0;
			memset(BtRecordCt->ScoInputFifo, 0, BtRecordCt->ScoInputLen-3);

			if(!BtRecord_MsbcDecoderStartedGet())
				return -1;
		}
		else if((BtRecordCt->ScoInputFifo[0] != 0xad)||(BtRecordCt->ScoInputFifo[1] != 0x00))
		{
			APP_DBG("msbc error\n");
			memset(BtRecordCt->ScoInputFifo, 0, BtRecordCt->ScoInputLen-3);
		}
		
		msbcLen = (BtRecordCt->ScoInputLen - 3);
		
		remainLen = mv_mremain(&BtRecordCt->msbcMemHandle);
		if(remainLen <= msbcLen)
		{
			APP_DBG("msbc receive fifo is full\n");
			//AudioCore source阻塞,丢弃缓存前面的数据
			/*if(AudioCore.AudioSink[AUDIO_HF_SCO_SINK_NUM].Enable == DISABLE)
			{
				uint8_t d[57];
				mv_mread(d, 57, 1, &BtRecordCt->msbcMemHandle);//discard
				APP_DBG("--");
				remainLen = mv_mremain(&BtRecordCt->msbcMemHandle);
				if(remainLen < msbcLen)
				{
					return -1;
				}
			}
			else*/
			{
				return -1;
			}
		}
		insertLen = mv_mwrite(BtRecordCt->ScoInputFifo, msbcLen, 1, &BtRecordCt->msbcMemHandle);
		if(insertLen != msbcLen)
		{
			APP_DBG("insert data len err! i:%ld,d:%d\n", insertLen, msbcLen);
		}

		/*if(AudioCore.AudioSink[AUDIO_HF_SCO_SINK_NUM].Enable == DISABLE)
		{
			uint8_t d[57];
			mv_mread(d, 57, 1, &BtRecordCt->msbcMemHandle);//discard 2 packet
			APP_DBG("--");
		}*/

		//decoder start
		if(!BtRecord_MsbcDecoderStartedGet())
		{
			if(BtRecord_MsbcDataLenGet()>BT_RECORD_MSBC_LEVEL_START)
			{
				int32_t ret=0;
				ret = BtRecord_MsbcDecoderStart();
				if(ret == 0)
				{
					APP_DBG("msbc decoder start success\n");
					BtRecordCt->DecoderSync = TaskStateStarting;
					BtRecord_MsbcDecoderStartedSet(TRUE);
/*#ifdef CFG_FUNC_FREQ_ADJUST
					AudioCoreSourceFreqAdjustEnable(1, BT_RECORD_LEVEL_LOW, BT_RECORD_LEVEL_HIGH);
#endif*/
				}
				else
				{
					APP_DBG("msbc decoder start fail\n");
				}
				
			}
		}
		
		//send data(60Bytes)
		if(BtRecord_SendScoBufLenGet() >= BT_MSBC_PACKET_LEN)
		{
			memset(BtRecordCt->scoSendBuf,0,BT_MSBC_PACKET_LEN);
			BtRecord_SendScoBufGet(BtRecordCt->scoSendBuf,BT_MSBC_PACKET_LEN);
			HfpSendScoData(BtRecordCt->scoSendBuf, BT_MSBC_PACKET_LEN);
		}
	}

	return 0;
}

/***********************************************************************************
 * 处理HFP需要发送的数据
 **********************************************************************************/
//2-EV3 = 60bytes/packet
//处理发送数据函数入口
void BtRecord_MicProcess(void)
{
	int32_t SRCDoneLen;	//sample number
	
	if(BtRecordCt->ModeKillFlag)
		return;
	
	if(GetHfpState() != BT_HFP_STATE_ACTIVE)
	{
		return;
	}

	if(BtRecordCt->btHfScoSendReady)
	{
		BtRecordCt->btHfScoSendReady = 0;
		BtRecordCt->btHfScoSendStart = 1;
		return;
	}

	if(!BtRecordCt->btHfScoSendStart)
		return;
	
	//sink2 channel
	if(BtRecordCt->AudioCoreSinkRecordSco->Enable == 0)
		BtRecordCt->AudioCoreSinkRecordSco->Enable = 1;
	
	if(BtRecordCt->codecType == HFP_AUDIO_DATA_PCM)
	{
		//CVSD
		if(BtRecord_SinkScoDataLenGet()>BT_CVSD_PACKET_LEN) //16K:120sample -> 8K:60sample
		{
			//1.转采样 16K -> 8K
			BtRecord_SinkScoDataGet(BtRecordCt->ScoOutputFifo, BT_CVSD_PACKET_LEN);
			SRCDoneLen = resampler_polyphase_apply(BtRecordCt->ScoOutResamplerCt, (int16_t*)BtRecordCt->ScoOutputFifo, (int16_t*)BtRecordCt->ScoOutSrcOutBuf, BT_CVSD_PACKET_LEN);

			//2.put data to send fifo
			if(SRCDoneLen>0)
			{
				if(BtRecord_SendScoBufSpaceLenGet() > SRCDoneLen*2)
					BtRecord_SendScoBufSet(BtRecordCt->ScoOutSrcOutBuf, SRCDoneLen*2);
			}
		}
	}
	else
	{
		if(BtRecordCt->mSbcEncoderStart)
		{
			/*if(BtRecord_SinkScoDataLenGet()>=128)
			{
				//1.resample 44.1k -> 16k
				BtRecord_SinkScoDataGet(BtRecordCt->AudioSink2OutBuf, 128);
			}
			return;
			*/
			if(BtRecord_SinkScoDataLenGet()>=128)
			{
				//1.resample 44.1k -> 16k
				BtRecord_SinkScoDataGet(BtRecordCt->AudioSink2OutBuf, 128);
				SRCDoneLen = resampler_polyphase_apply(BtRecordCt->AudioSink2OutResamplerCt, (int16_t*)BtRecordCt->AudioSink2OutBuf, (int16_t*)BtRecordCt->ScoOutSrcOutBuf, 128);

				//2.put data to fifo
				if(SRCDoneLen>0)
				{
					if(BtRecord_ResampleOutDataSpaceLenGet() > SRCDoneLen)
						BtRecord_ResampleOutDataSet(BtRecordCt->ScoOutSrcOutBuf, SRCDoneLen);
				}
			}
			
			//msbc encoder: 120 Sample
			if(BtRecord_ResampleOutDataLenGet()>BT_MSBC_MIC_INPUT_SAMPLE)
			{
				//1.encoder
				BtRecord_ResampleOutDataGet(BtRecordCt->ScoOutputFifo, BT_MSBC_MIC_INPUT_SAMPLE);
				memset(BtRecordCt->msbcEncoderFifo, 0, BT_MSBC_PACKET_LEN);
				
				//2. add sync header
				if(BtRecordCt->msbcSyncCount>3) BtRecordCt->msbcSyncCount = 0;
				memcpy(&BtRecordCt->msbcEncoderFifo[0], sco_sync_header1[BtRecordCt->msbcSyncCount], 2);
				BtRecordCt->msbcSyncCount++;
				BtRecordCt->msbcSyncCount %= 4;

				//3. encode
				sbc_encoder_encode(BtRecordCt->sbc_encode_ct, BtRecordCt->ScoOutputFifo, &BtRecordCt->msbcEncoderFifo[2], &BtRecordCt->msbc_encoded_data_length);

				//4.put data to send fifo
				if(BtRecord_SendScoBufSpaceLenGet() > BT_MSBC_PACKET_LEN)
					BtRecord_SendScoBufSet(BtRecordCt->msbcEncoderFifo, BT_MSBC_PACKET_LEN);
			}
		}
	}
}
#endif

